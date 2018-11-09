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
#include <argp.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

const char *argp_program_version = "sunshine 1.0";
const char *argp_program_bug_address = "https://github.com/dgm816/sunshine";

// program documentation
static char doc[] = "sunshine -- a usenet search tool";

// description of the arguments we accept
static char args_doc[] = "<nzb> [<nzb>...]";

// the options we understand
static struct argp_option options[] = {
        {"verbose",  'v', 0,            0,              "Produce verbose output" },
        {"quiet",    'q', 0,            0,              "Don't produce any output" },
        {"silent",   's', 0,            OPTION_ALIAS} ,
        {"output",   'o', "FILE",       0,              "Output to FILE instead of standard output" },
        {"username", 'u', "USERNAME",   0,              "Username to authenticate"},
        {"ssl",      'e', 0,            0,              "Use SSL" },
        { 0 }
};

// used by main to communicate with parse_opt
struct arguments {
    char *args[2];
    int silent;
    int verbose;
    int ssl;
    char *output_file;
    char *username;
};

// parse a single option
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    // get the input argument from argp_parse, which we
    // know is a pointer to our arguments structure
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'q':
        case 's':
            // quiet/silent
            arguments->silent = 1;
            arguments->verbose = 0;
            break;

        case 'v':
            // verbose
            arguments->verbose = 1;
            arguments->silent = 0;
            break;

        case 'o':
            // output file
            arguments->output_file = arg;
            break;

        case 'u':
            // username
            arguments->username = arg;
            break;

        case 'e':
            // enable ssl
            arguments->ssl = 1;
            break;

        case ARGP_KEY_ARG:
            // process an argument
            if (state->arg_num >= 1) {
                // just ignore extra arguments for now
            } else {
                arguments->args[state->arg_num] = arg;
            }
            break;

        case ARGP_KEY_END:
            // arguments are done processing
            if (state->arg_num < 1) {
                // not enough arguments
                argp_usage(state);
            }
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

// our argp parser
static struct argp argp = { options, parse_opt, args_doc, doc };

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

    struct addrinfo *result;
    struct addrinfo *rp;

    struct arguments arguments;

    // set default values
    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.output_file = "-";
    arguments.username = "";
    arguments.ssl = 0;

    // parse our arguments
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    FILE *output;

    // set our output handle
    if (strncmp("-", arguments.output_file, sizeof("-")) != 0) {
        output = fopen(arguments.output_file, "w");
        if (!output) {
            fprintf(stderr, "Unable to open output file. Exiting.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        output = stdout;
    }

    // do the output
    fprintf(output,
            "ARG1 = %s\n"
            "ARG2 = %s\n"
            "OUTPUT_FILE = %s\n"
            "VERBOSE = %s\n"
            "SILENT = %s\n"
            "Username = %s\n"
            "SSL = %s\n",
            arguments.args[0], arguments.args[1],
            arguments.output_file,
            arguments.verbose ? "yes" : "no",
            arguments.silent ? "yes" : "no",
            arguments.username,
            arguments.ssl ? "yes" : "no");

    if (strncmp("-", arguments.output_file, sizeof("-")) != 0) {
        fclose(output);
    }

    // SSL setup
    if (arguments.ssl) {
        SSL_library_init();
        method = TLS_client_method();
        ctx = SSL_CTX_new(method);
        if (ctx == NULL) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // Open connection
    char * ssl_port = "443";
    char * port = "80";

    // resolve addresses
    int s;
    if(arguments.ssl) {
        s = getaddrinfo("google.com", ssl_port, NULL, &result);
    } else {
        s = getaddrinfo("google.com", port, NULL, &result);
    }
    if(s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // start trying to connect to addresses
    for(rp = result; rp != NULL; rp = rp->ai_next) {

        // create a new socket
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sockfd == -1) {
            // try next address
            continue;
        }

        // try to connect socket
        if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            // connected
            break;
        }
    }

    // check if we succeeded
    if(rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    // clean up
    freeaddrinfo(result);

    // do we need to do ssl?
    if(arguments.ssl) {
        // Create new SSL connection state
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockfd);

        // Preform the ssl negotiation
        if (SSL_connect(ssl) != 1) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // Form the request we're going to send.
    char buff[64];
    sprintf(buff, "GET / HTTP/1.1\r\nHOST: www.google.com\r\n\r\n");

    // do we need to do ssl?
    int ret;
    if(arguments.ssl) {
        ret = SSL_write(ssl, buff, strlen(buff));
    } else {
        ret = write(sockfd, buff, strlen(buff));
    }
    if(ret != strlen(buff)) {
        fprintf(stderr, "Failed to write all data to socket?");
    }

    // Clear the whole buffer out..
    memset(buff, 0, sizeof(buff));

    // Receive a byte at a time and print it out...
    printf("Received:\n");
    int lb = 0;
    while(1) {
        // do we need to do ssl?
        if(arguments.ssl) {
            SSL_read(ssl, buff, 1);
        } else {
            read(sockfd, buff, 1);
        }

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

    // do we need to do ssl?
    if(arguments.ssl) {
        SSL_shutdown(ssl);
    }

    // close the socket
    close(sockfd);

    // clean up ssl
    if(arguments.ssl) {
        // release our SSL resources
        SSL_CTX_free(ctx);
    }

    return 0;
}
