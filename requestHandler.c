#define _POSIX_C_SOURCE 200112L

#include "cptc.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <cpetpet.h>


static bool isnumber(const char *s)
{
    for (const char *cp = s; *cp != '\0' && *cp != '.'; ++cp)
        if (!isdigit(*cp))
            return false;
    return true;
}

static char *generate_filename(char *buf, size_t size, const char *ext)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    snprintf(buf, size, "/tmp/%lld.%s", tp.tv_sec * 1000000000LL + tp.tv_nsec, ext);
    return buf;
}

void CPTC_requestHandler(int fd)
{
    enum CPTC_Method method;
    int ch;
    char *path, chbuf;
    char request[512];
    char filenamebuf[64];
    char response[512], *responseadd = response;
    ssize_t received = recv(fd, request, sizeof(request), 0);
    FILE *fp;

    memset(response, 0, sizeof(response));

    if (received == -1)
    {
        perror("recv()");
        return;
    }

    method = request[0];
    if (method != CPTC_GET && method != CPTC_HEAD)
    {
        strcpy(response, "HTTP/1.0 501 Not implemented\r\n");
        strcat(response, "\r\n");
        send(fd, response, strlen(response), 0);
        return;
    }

    path = strchr(request, ' ') + 1;
    *strchr(path, ' ') = '\0';
    puts(path);

    if (strlen(path) == 1)
    {
        char *length = malloc(sizeof(*length) * 32);
        snprintf(length, 32, "Content-Length: %ld\r\n", strlen(CPTC_root));
        strcpy(response, "HTTP/1.0 200 OK\r\n");
        strcat(response, "Content-Type: text/plain\r\n");
        strcat(response, length);
        strcat(response, "\r\n");
        send(fd, response, strlen(response), 0);
        if (method == CPTC_GET)
            send(fd, CPTC_root, strlen(CPTC_root), 0);
        free(length);
        return;
    }

    if (strchr(path, '.') && isnumber(path + 1))
    {
        char *length = malloc(sizeof(*length) * 32);

        *strchr(path, '.') = '\0';
        generate_filename(filenamebuf, sizeof(filenamebuf), "png");
        char *in = CPTC_downloadAvatar(atoll(path + 1), filenamebuf);
        if (!in)
        {
            strcpy(response, "HTTP/1.0 403 Forbidden\r\n");
            strcat(response, "\r\n");
            send(fd, response, strlen(response), 0);
            return;
        }
        generate_filename(filenamebuf, sizeof(filenamebuf), "gif");
        CPetPet(in, filenamebuf, 2);
        printf("From %s to %s\n", in, filenamebuf);

        fp = fopen(filenamebuf, "rb");
        if (!fp)
        {
            perror("fopen()");
            strcpy(response, "HTTP/1.0 500 Internal server error\r\n");
            strcat(response, "\r\n");
            send(fd, response, strlen(response), 0);
            free(in);
            free(length);
            return;
        }
        fseek(fp, 0, SEEK_END);
        snprintf(length, 32, "Content-Lnegth: %ld\r\n", ftell(fp));
        fseek(fp, 0, SEEK_SET);

        strcpy(response, "HTTP/1.0 200 OK\r\n");
        strcat(response, "Content-Type: image/gif\r\n");
        strcat(response, length);
        strcat(response, "\r\n");
        send(fd, response, strlen(response), 0);
        if (method == CPTC_GET)
        {
            responseadd = response;
            while ((ch = getc(fp)) >= 0)
            {
                *responseadd++ = ch;
                if (responseadd == response + sizeof(response))
                {
                    if (send(fd, response, sizeof(response), 0) < 0)
                    {
                        perror("send()");
                        goto err_send;
                    }
                    responseadd = response;
                }
            }
            send(fd, response, responseadd - response, 0);
        }

err_send:
        fclose(fp);

        remove(in);
        remove(filenamebuf);

        free(in);
        free(length);
        return;
    }

    strcpy(response, "HTTP/1.0 404 Not found");
    strcat(response, "\r\n");
    send(fd, response, strlen(response), 0);
}
