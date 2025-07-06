/*********************************************************************************
* Mia T
* httphelpers.c
* helper functions for httpserver.c
*********************************************************************************/

#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h>

#include "httphelpers.h"

#define BUF_SIZE 2048

/**
   Guarantees reading nbytes.
 */
ssize_t read_all(int connfd, char buffer[], size_t nbytes) {
    size_t total = 0;
    ssize_t bytes = 0;

    do {
        bytes = read(connfd, buffer + total, nbytes - total);
        if (bytes <= 0) {
            break;
        }
        total += bytes;
    } while (bytes > 0 && total < nbytes);

    return total;
}

/**
   Guarantees writing nbytes.
 */
ssize_t write_all(int connfd, char buffer[], size_t nbytes) {
    size_t total = 0;
    size_t bytes = 0;

    do {
        bytes = write(connfd, buffer + total, nbytes - total);
        if (bytes < 0) {
            return -1; // ded
        }
        total += bytes;
    } while (total < bytes);
    return total;
}

/**
   Destructor for requests().
 */
void free_requests(char *method, char *uri, char *http_ver, regex_t regex) {
    free(method);
    free(uri);
    free(http_ver);
    regfree(&regex);
}

/**
   Acquires the first value of key string that appears in pairs.
 */
int get_keys_value(List pairs, char key[]) {
    int value = 0;

    moveFront(pairs);
    for (int i = 0; i < length(pairs); i++) {
        if (strcmp(get_key(pairs), key) == 0) {
            value = get_val(pairs);
            break;
        }
        moveNext(pairs);
    }

    return value;
}

/**
   Handles reading the contents from the uri and writing the response.
 */
void get_response(int connfd, int uri_fd, int content_length) {

    char get_buf[BUF_SIZE] = { 0 };
    char cl_str[BUF_SIZE];
    sprintf(cl_str, "%d", content_length);

    write(connfd, "HTTP/1.1 200 OK\r\n", 17);
    write(connfd, "Content-Length: ", 16);
    write(connfd, cl_str, strlen(cl_str));
    write(connfd, "\r\n\r\n", 4);

    bool reading = true;
    flock(uri_fd, LOCK_SH);
    while (reading) {
        int bytes = read_all(uri_fd, get_buf, BUF_SIZE);
        if (bytes <= 0) {
            reading = false;
        }
        write_all(connfd, get_buf, bytes);
    }
    flock(uri_fd, LOCK_UN);
    return;
}

/**
   Status code responses.
 */
void send_response(int connfd, int status_code) {
    switch (status_code) {
    case 200:
        write(connfd, "HTTP/1.1 200 OK\r\n", 17);
        write(connfd, "Content-Length: 3\r\n\r\n", 21);
        write(connfd, "OK\n", 3);
        break;
    case 201:
        write(connfd, "HTTP/1.1 201 Created\r\n", 22);
        write(connfd, "Content-Length: 8\r\n\r\n", 21);
        write(connfd, "Created\n", 8);
        break;
    case 404:
        write(connfd, "HTTP/1.1 404 Not Found\r\n", 24);
        write(connfd, "Content-Length: 10\r\n\r\n", 22);
        write(connfd, "Not Found\n", 10);
        break;
    case 500:
        write(connfd, "HTTP/1.1 500 Internal Server Error\r\n", 36);
        write(connfd, "Content-Length: 22\r\n\r\n", 22);
        write(connfd, "Internal Server Error\n", 22);
        break;
    default: break;
    }
    return;
}
