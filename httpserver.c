/*********************************************************************************
* Mia T
* httpserver.c
* creates an httpserver
*********************************************************************************/

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "httphelpers.h"
#include "List.h"
#include "queue.h"

#define OPTIONS              "t:l:"
#define BUF_SIZE             2048
#define ARRAY_SIZE(arr)      (sizeof((arr)) / sizeof((arr)[0]))
#define DEFAULT_THREAD_COUNT 4

static FILE *logfile;
#define LOG(...) fprintf(logfile, __VA_ARGS__);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full, empty = PTHREAD_COND_INITIALIZER;

int thread_num;
pthread_t *tid;
Queue tasks;

/**
   This function reads from the source file and writes to the destination file.
 */
void read_write_message(int dest_fd, int source_fd, char buffer[], int content_length) {

    int total = 0;
    int bytes = 0;
    do {
        if ((content_length - total) < BUF_SIZE) {
            bytes = read(source_fd, buffer, content_length - total);
        } else {
            bytes = read(source_fd, buffer, BUF_SIZE);
        }
        if (bytes <= 0) {
            break;
        }
        total += bytes;
        write_all(dest_fd, buffer, bytes);
    } while (bytes > 0 && total < content_length);
    return;
}

/**
   This function reads from the Message-Body and writes to a temporary file.
 */
void write_temp(
    int connfd, int temp_fd, char buffer[], char overlap[], int overlap_len, int content_length) {

    // The "4"s below are to account for rnrn at the beginning of overlap
    if (overlap_len < content_length) {
        write_all(temp_fd, overlap + 4, overlap_len - 4);
    } else {
        write_all(temp_fd, overlap + 4, content_length);
        lseek(temp_fd, 0, SEEK_SET);
        return; // No need to write anything else
    }

    content_length = content_length - (overlap_len - 4);

    read_write_message(temp_fd, connfd, buffer, content_length);

    lseek(temp_fd, 0, SEEK_SET);
    return;
}

/**
   Parses the Header-Fields.
 */
List handle_header(char buffer[], int *cursor) {
    regex_t regex;
    regmatch_t matches[2 + 1];

    List pairs = newList();
    bool has_content_length = false;

    while (buffer[*cursor] != '\r' && buffer[*cursor + 1] != '\n') {

        regcomp(&regex, "([^ \n\t]+): ([^ \n\t]+)\r\n", REG_EXTENDED);
        regexec(&regex, buffer + *cursor, ARRAY_SIZE(matches), matches, 0);

        int key_len = matches[1].rm_eo - matches[1].rm_so;
        char *key = strndup(&buffer[matches[1].rm_so + *cursor], key_len);

        int val_len = matches[2].rm_eo - matches[2].rm_so;
        char *val = strndup(&buffer[matches[2].rm_so + *cursor], val_len);

        int val_int = atoi(val);
        append(pairs, val_int, key);

        if (strcmp(key, "Content-Length") == 0) {
            has_content_length = true;
        }

        *cursor += matches[0].rm_eo;

        free(key);
        free(val);
        regfree(&regex);
    }

    *cursor += 2;

    return pairs;
}

/**
   Handles appending to the file specified by the uri.
 */
void append_request(int connfd, char buffer[], char overlap[], int overlap_len, char uri[],
    int content_length, int *status_code) {

    int uri_fd = open(uri, O_WRONLY | O_APPEND, 0644);

    // Internal Server Error
    if (errno == ENOTDIR) {
        *status_code = 500;
        send_response(connfd, *status_code);
        close(uri_fd);
        return;
    }

    char template[11] = "tempXXXXXX";
    int temp_fd = mkstemp(template);

    // Write message body into temporary file
    write_temp(connfd, temp_fd, buffer, overlap, overlap_len, content_length);

    // Put the message body into the uri
    flock(uri_fd, LOCK_EX);
    read_write_message(uri_fd, temp_fd, buffer, content_length);
    flock(uri_fd, LOCK_UN);

    send_response(connfd, *status_code);
    close(uri_fd);
    unlink(template);
    return;
}

/**
   Handles putting to the file specified by the uri.
   If the uri doesn't exist, it is created.
 */
void put_request(int connfd, char buffer[], char overlap[], int overlap_len, char uri[],
    int content_length, int *status_code) {

    if (access(uri, F_OK) == 0) {
        // uri already exists
        *status_code = 200;
    } else {
        // uri doesn't already exist
        *status_code = 201;
    }

    char template[11] = "tempXXXXXX";
    int temp_fd = mkstemp(template);

    // Write message body into temporary file
    write_temp(connfd, temp_fd, buffer, overlap, overlap_len, content_length);

    int uri_fd = open(uri, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // Internal Server Error
    if (errno == ENOTDIR) {
        *status_code = 500;
        send_response(connfd, *status_code);
        close(uri_fd);
        unlink(template);
        return;
    }

    // Put message body into the uri
    flock(uri_fd, LOCK_EX);
    read_write_message(uri_fd, temp_fd, buffer, content_length);
    flock(uri_fd, LOCK_UN);

    send_response(connfd, *status_code);
    close(uri_fd);
    unlink(template);
    return;
}

/**
   Handles getting the contents from the uri.
 */
void get_request(int connfd, char uri[], int *status_code) {
    int uri_fd = open(uri, O_RDONLY, 0644);

    // Not Found
    if (errno == ENOENT) {
        *status_code = 404;
        send_response(connfd, *status_code);
        close(uri_fd);
        return;
    }

    struct stat uri_stat;
    stat(uri, &uri_stat);

    // Internal Server Error
    if (errno == ENOTDIR) {
        *status_code = 500;
        send_response(connfd, *status_code);
        close(uri_fd);
        return;
    }

    int content_length = uri_stat.st_size;

    // Send response
    get_response(connfd, uri_fd, content_length);
    close(uri_fd);
    return;
}

/**
   Parses request line. Checks for errors along the way.
   Determines which method to execute.
 */
void requests(int connfd, char buffer[], char overlap[], int overlap_len) {
    regex_t regex;
    regmatch_t matches[3 + 1];

    regcomp(
        &regex, "([a-zA-Z]{1,8}) /(/?[a-zA-Z0-9._]{1,19}) (HTTP/[0-9].[0-9])\r\n", REG_EXTENDED);
    regexec(&regex, buffer, ARRAY_SIZE(matches), matches, 0);

    int method_len = matches[1].rm_eo - matches[1].rm_so;
    char *method = strndup(&buffer[matches[1].rm_so], method_len);

    int uri_len = matches[2].rm_eo - matches[2].rm_so;
    char *uri = strndup(&buffer[matches[2].rm_so], uri_len);

    int http_len = matches[3].rm_eo - matches[3].rm_so;
    char *http_ver = strndup(&buffer[matches[3].rm_so], http_len);

    int cursor = matches[0].rm_eo - matches[0].rm_so;

    int status_code = 200;

    // Handle header
    List pairs = handle_header(buffer, &cursor);

    int content_length = get_keys_value(pairs, "Content-Length");
    int request_id = get_keys_value(pairs, "Request-Id");
    freeList(&pairs);

    // Execute methods
    errno = 0;
    if (strcmp(method, "GET") == 0 || strcmp(method, "get") == 0) {
        get_request(connfd, uri, &status_code);
    } else if (strcmp(method, "PUT") == 0 || strcmp(method, "put") == 0) {
        put_request(connfd, buffer, overlap, overlap_len, uri, content_length, &status_code);
    } else if (strcmp(method, "APPEND") == 0 || strcmp(method, "append") == 0) {
        append_request(connfd, buffer, overlap, overlap_len, uri, content_length, &status_code);
    }

    // Print audit log to logfile
    pthread_mutex_lock(&lock);
    LOG("%s,/%s,%d,%d\n", method, uri, status_code, request_id);
    fflush(logfile);
    pthread_mutex_unlock(&lock);

    free_requests(method, uri, http_ver, regex);
    return;
}

// Converts a string to an 16 bits unsigned integer.
// Returns 0 if the string is malformed or out of the range.
static size_t strtouint16(char number[]) {
    char *last;
    long num = strtol(number, &last, 10);
    if (num <= 0 || num > UINT16_MAX || *last != '\0') {
        return 0;
    }
    return num;
}

// Creates a socket for listening for connections.
// Closes the program and prints an error message on error.
static int create_listen_socket(uint16_t port) {
    struct sockaddr_in addr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err(EXIT_FAILURE, "socket error");
    }
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *) &addr, sizeof addr) < 0) {
        err(EXIT_FAILURE, "bind error");
    }
    if (listen(listenfd, 128) < 0) {
        err(EXIT_FAILURE, "listen error");
    }
    return listenfd;
}

static void handle_connection(int connfd) {

    char buffer[BUF_SIZE] = { 0 };
    char *overlap;
    size_t total = 0;
    ssize_t bytes_read;

    do {
        // Read from connfd until EOF or error.
        bytes_read = read(connfd, buffer + total, BUF_SIZE - total);
        if (bytes_read < 0) {
            return;
        }

        if ((overlap = strstr(buffer, "\r\n\r\n")) != NULL) {
            // \r\n\r\n is in the buffer
            int overlap_len = bytes_read - (overlap - buffer);
            requests(connfd, buffer, overlap, overlap_len);
            break;
        }
        total += bytes_read;

    } while (bytes_read > 0 && total < BUF_SIZE);
}

static void sigterm_handler(int sig) {
    if (sig == SIGTERM) {
        warnx("received SIGTERM");
        pthread_mutex_destroy(&lock);
        freeQueue(&tasks);
        free(tid);
        fclose(logfile);
        exit(EXIT_SUCCESS);
    }
}

static void usage(char *exec) {
    fprintf(stderr, "usage: %s [-t threads] [-l logfile] <port>\n", exec);
}

void *consumer(void *tasks) {

    while (1) {
        pthread_mutex_lock(&lock);

        while (isEmpty(tasks)) {
            pthread_cond_wait(&full, &lock);
        }

        int connfd = qfront(tasks);
        dequeue(tasks);

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&lock);

        handle_connection(connfd);
        close(connfd);
    }
    return NULL;
}

void producer(int listenfd, Queue tasks) {

    while (1) {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            warn("accept error");
            continue;
        }
        pthread_mutex_lock(&lock);

        while (isFull(tasks)) {
            pthread_cond_wait(&empty, &lock);
        }

        enqueue(tasks, connfd);

        pthread_cond_signal(&full);
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *argv[]) {
    int opt = 0;
    int threads = DEFAULT_THREAD_COUNT;
    logfile = stderr;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 't':
            threads = strtol(optarg, NULL, 10);
            if (threads <= 0) {
                errx(EXIT_FAILURE, "bad number of threads");
            }
            break;
        case 'l':
            logfile = fopen(optarg, "w");
            if (!logfile) {
                errx(EXIT_FAILURE, "bad logfile");
            }
            break;
        default: usage(argv[0]); return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        warnx("wrong number of arguments");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    uint16_t port = strtouint16(argv[optind]);
    if (port == 0) {
        errx(EXIT_FAILURE, "bad port number: %s", argv[1]);
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, sigterm_handler);

    int listenfd = create_listen_socket(port);

    thread_num = threads;

    tasks = newQueue(threads);

    pthread_mutex_init(&lock, NULL);

    tid = (pthread_t *) malloc(sizeof(pthread_t) * threads);
    for (int i = 0; i < threads; i++) {
        pthread_create(&tid[i], NULL, &consumer, tasks);
    }

    producer(listenfd, tasks);

    return EXIT_SUCCESS;
}
