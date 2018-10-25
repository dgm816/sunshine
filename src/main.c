/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main (int argc, char* argv[]) {
    int sockfd;

    struct addrinfo *result, *rp;

    int s = getaddrinfo("google.com", "80", NULL, &result);
    if(s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // start trying to connect to addresses
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sockfd == -1)
            continue;

        if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(sockfd);
    }

    // check if we succeeded
    if(rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    // clean up
    freeaddrinfo(result);

    char buff[64];
    sprintf(buff, "GET / HTTP/1.1\r\nHOST: www.google.com\r\n\r\n");

    ssize_t ret = send(sockfd, buff, strlen(buff), 0);
    printf("send ret: %u\n", ret);

    memset(buff, 0, sizeof(buff));

    printf("recv:\n");
    int lb = 0;
    while(1) {
        recv(sockfd, buff, 1, 0);
        if(*buff == '\r') {
            continue;
        }

        printf("%c", *buff);

        if(lb && *buff == '\n') {
            // found double line break
            break;
        } else if(*buff == '\n') {
            // found a link break
            lb = 1;
        } else {
            lb = 0;
        }
    }

    // close the socket
    close(sockfd);

    return 0;
}