#include "cptc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>


#define error(text, status) \
    do                      \
    {                       \
        perror(text);       \
        exit(status);       \
    } while (0)             \


struct RequestHandlerArgs
{
    int fd;
    int n;
    bool accept;
};

static void *requestHandlerThread(struct RequestHandlerArgs *arg)
{
    int fd = arg->fd;
    int n = arg->n;
    arg->accept = true;
    CPTC_requestHandler(fd, n);
    close(fd);
    pthread_exit(NULL);
}


void CPTC(const char *ip, in_port_t port)
{
    struct sockaddr_in addr, peer;
    socklen_t peer_size;
    int fd, peerfd;
    int n = 0;

    curl_easy_init();

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("socket()", 1);
    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) < 0)
        error("setsockopt()", 1);

    addr = (struct sockaddr_in)
    {
        .sin_addr.s_addr = inet_addr(ip),
        .sin_port = htons(port),
        .sin_family = AF_INET,
    };

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("bind()", 1);
    if (listen(fd, 128) == -1)
        error("listen()", 3);

    printf("Started serving at %s:%d\n", ip, port);
    for (;;)
    {
        peer_size = sizeof(peer);
        if ((peerfd = accept(fd, (struct sockaddr *)&peer, &peer_size)) < 0)
        {
            perror("accept()");
            continue;
        }
        printf("Connection from %s:%u!\n", inet_ntoa(peer.sin_addr), peer.sin_port);

        pthread_t thread;
        struct RequestHandlerArgs arg = {peerfd, n++, false};
        pthread_create(&thread, NULL, (void *(*)(void *))requestHandlerThread, &arg);
        pthread_detach(thread);
        while (!arg.accept);
    }
}
