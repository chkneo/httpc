/*
Copyright (c) 2012, Chandrasekar Kanagaraj <chkneo@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "httpc.h"

/* Prints a help message to the user.  This function never returns. */

static void
usage (void)
{
    fprintf (stderr, "usage: httpc -h <host> [-c iteration] [-d delay]\n");
    exit (1);
}

/* Reading http data file */

static char*
parseData(FILE *fp)
{
    char *data = NULL;
    char line[1024];
    fgets(line, sizeof(line), fp);
    data = malloc(sizeof(line));
    strcpy(data,line);
    while(fgets(line, sizeof(line), fp)) {
    data = strcat(data, line);
    }
    return(data);
}  

int
main (int argc, char **argv)
{
    int i=0, j, opt, port, sockfd, cpl, bytes_recieved, flag=1;
    unsigned int delay = DEFAULT_DELAY;
    unsigned int count = DEFAULT_COUNT;
    char *hostname=NULL;
    char *cp, *data, *request[MAXPACKETS], recv_data[MAXPACKETSIZE];
    struct hostent *host;
    struct sockaddr_in server;
    FILE *fp;

    while((opt = getopt(argc, argv, "h:c:d:")) != -1) {
        switch (opt) {
            case 'c':
                count = atoi(optarg);
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'h':
                hostname = malloc(sizeof(optarg));
                hostname = optarg;
                if (!hostname)
                    usage();
                break;
            default:
                usage();
                break;
        }
    }

    port = DEFAULT_PORT;
    if(hostname == NULL) {
        usage();
    }

    if (strrchr(hostname, ':')) {
        cp = strrchr(hostname, ':');
        *cp = '\0';
        strncpy(hostname, hostname, strlen(hostname) - strlen(cp));
        port = atoi(++cp);
    }  

    if (hostname == NULL) {
        printf("Invalid hostname\n");
        exit(1);
    }

    host = gethostbyname(hostname);
    if(!host) {
        printf("Unable to reach host\n");
        exit(1);
    }


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket\n");
        exit(1);
    }
    if((setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) < 0)) {
        printf("Unable to set delay parameters\n");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server.sin_zero), 0, sizeof(server.sin_zero));

    if ((fp = fopen("data","r")) == NULL) {
        printf("HTTP Data file is not available\n");
        exit(1);
    }

    data = parseData(fp);
    *request = strtok(data,SPLITER);

    while((cp = strtok(NULL, SPLITER))) {
        cpl = strlen(cp)+1;
        request[i+1] = malloc(cpl);
        strcpy(request[i+1],cp);
        i++;
    }

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Unable to connect\n");
        exit(1);
    }
    while(count) {
        for(j=0;j<=i;j++) {
            printf("--------------------------------------\n");
            printf("%s\n",request[j]); 
            printf("--------------------------------------\n");
            sleep(delay);
        }

        bytes_recieved = 0;
        while(1) {
            bytes_recieved=recv(sockfd,recv_data,1024, 0);
            recv_data[bytes_recieved] = '\0';
            printf("--------------------------------------\n");
            printf("%s\n", recv_data);
            printf("--------------------------------------\n");
            if(bytes_recieved < 1024)
                break;
        }
        count--;
    }

    close(sockfd);
    return(0);
}
