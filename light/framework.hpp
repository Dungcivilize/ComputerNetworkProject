#pragma once

// POSIX Macros
#define _POSIX_C_SOURCE 200809L

// Size limit for allocation
#define BUFFER_SIZE 32787
#define CMD_SIZE 32
#define ID_SIZE 16
#define NAME_SIZE 256
#define TYPENAME_SIZE 16
#define PASSWORD_SIZE 256
#define MESSAGE_SIZE 512
#define TOKEN_SIZE 32

// Network constants
#define PORT 5000
#define CONN_TIME_LIMIT 20
#define RESP_TIME_LIMIT 100

// Success codes
#define SUCCESS_SCAN 100
#define SUCCESS_CONNECTION 200
#define SUCCESS_COMMAND 300
#define SUCCESS_PW_CHANGE 400
#define SUCCESS_QUERY 500
#define SUCCESS_CONFIG 700

// Error codes
#define ERROR_UNSUPPORT_PROTOCOL 801
#define ERROR_BAD_REQUEST 802

#define ERROR_SCAN_BLOCKED 101
#define ERROR_PW_INCORRECT 201
#define ERROR_INVALID_TOKEN 401
#define ERROR_INVALID_SCOPE 501
#define ERROR_INVALID_PARAM 701
#define ERROR_INVALID_VALUE 702

// POSIX C libs for socket programming
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <time.h>

// C++ libs
#include <bits/stdc++.h>
using namespace std;