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

#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main (int argc, char* argv[]) {
    int sockfd = 0;

    SSL_CTX *ctx;
    const SSL_METHOD *method;
    SSL *ssl;

    struct addrinfo *result, *rp;

    // SSL setup
    SSL_library_init();

    method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if(ctx == NULL) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Open connection
    int s = getaddrinfo("google.com", "443", NULL, &result);
    if(s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // start trying to connect to addresses
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sockfd == -1)
            continue;

        if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
    }

    // check if we succeeded
    if(rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    // clean up
    freeaddrinfo(result);

    // Create new SSL connection state
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    // Preform the ssl negotiation
    if(SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Form the request we're going to send.
    char buff[64];
    sprintf(buff, "GET / HTTP/1.1\r\nHOST: www.google.com\r\n\r\n");

    //ssize_t ret = write(sockfd, buff, strlen(buff));
    int ret = SSL_write(ssl, buff, strlen(buff));
    if(ret != strlen(buff)) {
        fprintf(stderr, "Failed to write all data to socket?");
    }

    // Clear the whole buffer out..
    memset(buff, 0, sizeof(buff));

    // Receive a byte at a time and print it out...
    printf("Receved:\n");
    int lb = 0;
    while(1) {
        //read(sockfd, buff, 1);
        SSL_read(ssl, buff, 1);
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

    // release our SSL resources
    SSL_CTX_free(ctx);

    return 0;
}
