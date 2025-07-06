# httpserver
by Mia T, 06/1/22

New additions are underneath small bold text.

## httpserver.c


### main()
main() is where we create our worker threads which act on our consumer() function. Along with producer(), this acts as our dispatcher thread.

### producer()
producer() is where we create the jobs for our worker threads. In this program, the "jobs" are the multiple connections sent by the client that are then enqueued onto a queue. In this function, I use an infinite while loop in order to account for any amount of requests sent by the client. I then use a mutex lock started after the connection is accepted to prevent any other threads affecting the coherency of the connections queued in this region. I also use a condition variable that waits when the queue is full, as we do not want to produce any more jobs for our queue if it is full.

### consumer()
consumer() is where we process the jobs created by our dispatcher thread. In this function, I use an infinite while loop in order to account for any amount of requests sent by the client. I then use a mutex lock that starts at the begining of this loop, but then unlocks after the connection has been dequeued in order to prevent other threads from affecting the coherency of the queue. I also use a condition variable that waits when the queue is empty, as we don't want to take anything from the queue if it's empty. After the mutex unlock, I use the dequeued connection to be passed into handle_connection()

### sigterm_handler()
This is where I join my threads, destroy my mutex lock, and free any allocated memory.

### handle_connection()
This function reads the given request in increments of 2048 bytes into a buffer. This is because we are guaranteed for the Request-Line and Header-Fields to be no greater than 2048 bytes.

The requests are then handled when \r\n\r\n is detected by strstr() to be in the buffer. The return value of strstr() is saved to a buffer called overlap, as it may contain a portion of the Message-Body.

I use pointer arithmetic to calculate the length of overlap and pass it through all my functions for it to be used in read_write_message().

### requests()
This function parses the Request-Line using regex. I chose to use regex at the advise of the instructors after realizing that sscanf did not parse as cleanly as regex.

I use strndup() to save the regex groups into strings after failed attempts at using memcpy to do the same thing, but it caused gnarly memory issues when sending consecutive requests.

I reset errno to 0 before I handle the requests for each method to avoid any false errors.

I use the helper function get_keys_value() twice. First to get the specified Content-Length, and next to get the specified Request-Id. The Content-Length is then passed to the put/append functions and is then used to write the Message-Body. The Request-Id is used for the audit log, which is written to the logfile by using LOG() after processing the requests.

### get_request()
This function opens the uri to GET from, grabs its length, and handles errors unique to GET.

If the errno becomes ENOENT (not found) after opening the URI, then a 404 Not Found error is sent.

If the errno becomes ENOTDIR, then a 500 Internal Server Error is sent.

### put_request()
This function opens/creates the uri to PUT to and handles errors unique to PUT.

If the uri to be PUT to already exists then I send a 200 response, but if it needs to be created then I send a 201 response.

If the errno becomes ENOTDIR after opening the URI, then a 500 Internal Server Error is sent.

**New changes**

I use an exclusive flock() around the process where I write to the uri in order to maintain atomicity and coherency. This is achieved as flock() will block exclusive locks from other PUT/APPEND processes.

### append_request()
This function opens the uri to APPEND to and handles errors unique to APPEND.

If the errno becomes ENOTDIR after opening the URI, then a 500 Internal Server Error is sent.

**New changes**

I use an exclusive flock() around the process where I write to the uri in order to maintain atomicity and coherency. This is achieved as flock() will block exclusive locks from other PUT/APPEND processes.

### handle_header()
This function parses the Header-Fields in the request using regex and a cursor. I decided to use regex after many frustrating attempts and failures at parsing the Header-Fields using sscanf() and strtok() + sscanf() (I could not figure out how to stop at \r\n\r\n).

My while loop terminates when the cursor is at \r and the cursor + 1 is at \n. This is because my regex matches line-by-line, "Key: Value\r\n", and the cursor moves line-by-line accordingly. Therefore, when the cursor detects that there is another \r\n after the matched line, we know it's time to stop parsing the Header-Fields.

The Key and Value pairs are saved to a linked list ADT.

This function also detects whether any of the Header-Fields contains a Content-Length.
The linked list of key value pairs are returned.

### write_temp()
**New function**

A function that reads the Message-Body from connfd and writes it to a temporary file.

This function first writes from the string that I saved the return value of strstr() to from handle_connection(). This is because it contains either the entirety or a portion of the Message-Body. If it contains the entirety of the Message-Body, then it just writes it to the temporary file and returns. If it contains only a portion of the Message-Body, then it reads from connfd and writes to the temporary file in increments of 2048 up until the remaining Content-Length is less than 2048.

In order to maintain the position of the file cursor in between my reads/writes for the temporary file and then the uri, I use lseek() to reset the file cursor position back to 0 after the message body is written to the temporary file.

### read_write_message()
**New changes**

A function that reads the Message-Body from a source file and writes it to a destination file.

## httphelpers.c

### free_requests()
This function frees the memory allocated for regex parsing purposes in requests().

### get_keys_value()
This function loops through the linked list of key value pairs and identifies the first occurence of the passed in key string, and returns it's corresponding value. This replaces my get_content_length() function from asgn1.

### get_response()
This function handles the unique response that is associated with a GET request. This function reads from the uri and writes to connfd in increments of 2048 in order to accomodate large files.

**New changes**

I use a shared flock() around my process where I read from the uri in order to block exclusive locks from PUT/APPEND while GET is reading from the uri

### send_response()
This function sends different responses to connfd depending on the status code that is passed into it.

## List.c
This is from a Linked List ADT that I made in CSE 101 at UC Santa Cruz.
I made small additions to this ADT in order to handle having a string key and integer value pair.

## queue.c
This is a bounded queue ADT that I made by editing my Linked List ADT. I use it to store the connections created by producer() for the worker threads to act on. It stores "threads"(number of threads created by the server) number of connections.
