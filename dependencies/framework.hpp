#define BUFFER_SIZE 32787
#define _POSIX_C_SOURCE 200809L

#define BROADCAST_MESSAGE "SCAN"
#define ID_SIZE 128
#define NAME_SIZE 128
#define TYPE_SIZE 16

#define SUCCESS_CONNECTION 110
#define SUCCESS_PASSWORD_CHANGE 120
#define SUCCESS_PARAM_CHANGE 130
#define ERROR_UNKNOWN_TOKEN 210
#define ERROR_PASSWORD_INCORRECT 220
#define ERROR_INVALID_PARAM 310
#define ERROR_UNKNOWN_COMMAND 810

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
#include <unistd.h>
#include <netdb.h>

// C++ libs
#include <bits/stdc++.h>

using namespace std;