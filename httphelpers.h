/*********************************************************************************
* Mia T
* httphelpers.h
* helper functions for httpserver.c
*********************************************************************************/

#ifndef __HTTPHELPERS_H__
#define __HTTPHELPERS_H__

#include <regex.h>
#include <stdio.h>

#include "List.h"

ssize_t read_all(int connfd, char buffer[], size_t nbytes);

ssize_t write_all(int connfd, char buffer[], size_t nbytes);

void free_requests(char *method, char *uri, char *http_ver, regex_t regex);

int get_keys_value(List pairs, char key[]);

void get_response(int connfd, int uri_fd, int content_length);

void send_response(int connfd, int status_code);

#endif
