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
#include <libhttpc/libhttpc.h>
#ifndef  LibHTTPC_SOCK
#error LibHTTPC should be built with support of sockets for this project
#endif


static bool isnumber(const char *s)
{
    for (const char *cp = s; *cp != '\0' && *cp != '.'; ++cp)
        if (!isdigit(*cp))
            return false;
    return true;
}

static char *generate_filename(char *buf, size_t size, const char *ext, int n)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    snprintf(buf, size, "/tmp/%d-%lld.%s", n, tp.tv_sec * 1000000000LL + tp.tv_nsec, ext);
    return buf;
}

void CPTC_requestHandler(int fd, int n)
{
    int ch;
    char chbuf;
    char raw_request[512];
    char filenamebuf[64];
    char raw_response[512], *responseadd = raw_response;
    ssize_t received = recv(fd, raw_request, sizeof(raw_request), 0);
    FILE *fp;

    enum LibHTTPC_Method method;
    struct LibHTTPC_Request request = {0};

    if (received == -1)
    {
        perror("recv()");
        return;
    }

    LibHTTPC_loadRequest(&request, raw_request);

    method = LibHTTPC_loadMethod(request.method);
    if (method != LibHTTPC_Method_GET && method != LibHTTPC_Method_HEAD)
    {
        LibHTTPC_dumpResponse(
            &(struct LibHTTPC_Response){.status = LibHTTPC_Status_NOT_IMPLEMENTED},
            raw_response,
            sizeof(raw_response)
        );
        send(fd, raw_response, strlen(raw_response), 0);
        return;
    }

    if (strlen(request.uri) == 1)
    {
        char length[20];
        snprintf(length, sizeof(length), "%ld", strlen(CPTC_root));

        LibHTTPC_dumpResponse(
            &(struct LibHTTPC_Response)
            {
                .headers = (struct LibHTTPC_Header[])
                {
                    {
                        LibHTTPC_dumpHeader(LibHTTPC_Header_CONTENT_TYPE),
                        "text/plain",
                    },
                    {
                        LibHTTPC_dumpHeader(LibHTTPC_Header_CONTENT_LENGTH),
                        length,
                    }
                },
                .header_count = 2,
                .body = (method == LibHTTPC_Method_GET) ? CPTC_root : NULL,
            },
            raw_response,
            sizeof(raw_response)
        );

        send(fd, raw_response, strlen(raw_response), 0);
        return;
    }

    if (strchr(request.uri, '.') && isnumber(request.uri + 1))
    {
        char length[20];

        *strchr(request.uri, '.') = '\0';
        generate_filename(filenamebuf, sizeof(filenamebuf), "png", n);
        char *in = CPTC_downloadAvatar(atoll(request.uri + 1), filenamebuf);
        if (!in)
        {
            LibHTTPC_dumpResponse(
                &(struct LibHTTPC_Response){.status = LibHTTPC_Status_FORBIDDEN},
                raw_response,
                sizeof(raw_response)
            );
            send(fd, raw_response, strlen(raw_response), 0);
            return;
        }
        generate_filename(filenamebuf, sizeof(filenamebuf), "gif", n);
        CPetPet(in, filenamebuf, 2);
        printf("From %s to %s\n", in, filenamebuf);

        fp = fopen(filenamebuf, "rb");
        if (!fp)
        {
            perror("fopen()");
            LibHTTPC_dumpResponse(
                &(struct LibHTTPC_Response){.status = LibHTTPC_Status_INTERNAL_SERVER_ERROR},
                raw_response,
                sizeof(raw_response)
            );
            send(fd, raw_response, strlen(raw_response), 0);

            goto gif_end;
        }
        fseek(fp, 0, SEEK_END);
        snprintf(length, sizeof(length), "%ld", ftell(fp));
        fseek(fp, 0, SEEK_SET);

        LibHTTPC_dumpResponse(
            &(struct LibHTTPC_Response)
            {
                .headers = (struct LibHTTPC_Header[])
                {
                    {
                        LibHTTPC_dumpHeader(LibHTTPC_Header_CONTENT_TYPE),
                        "image/gif",
                    },
                    {
                        LibHTTPC_dumpHeader(LibHTTPC_Header_CONTENT_LENGTH),
                        length,
                    }
                },
                .header_count = 2,
            },
            raw_response,
            sizeof(raw_response)
        );
        send(fd, raw_response, strlen(raw_response), 0);
        if (method == LibHTTPC_Method_GET)
        {
            responseadd = raw_response;
            while ((ch = getc(fp)) >= 0)
            {
                *responseadd++ = ch;
                if (responseadd == raw_response + sizeof(raw_response))
                {
                    if (send(fd, raw_response, sizeof(raw_response), 0) < 0)
                    {
                        perror("send()");
                        goto gif_end;
                    }
                    responseadd = raw_response;
                }
            }
            send(fd, raw_response, responseadd - raw_response, 0);
        }

gif_end:
        if (fp)
            fclose(fp);

        remove(in);
        remove(filenamebuf);

        free(in);
        return;
    }

    LibHTTPC_dumpResponse(
        &(struct LibHTTPC_Response){.status=LibHTTPC_Status_NOT_FOUND},
        raw_response,
        sizeof(raw_response)
    );
    send(fd, raw_response, strlen(raw_response), 0);
}
