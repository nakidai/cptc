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


static const char *ok               = "HTTP/1.0 200 OK\r\n";
static const char *notfound         = "HTTP/1.0 404 Not found\r\n";
static const char *forbidden        = "HTTP/1.0 403 Forbidden\r\n";
static const char *intserverror     = "HTTP/1.0 500 Internal server error\r\n";
static const char *not_implemented  = "HTTP/1.0 501 Not implemented\r\n";

static const char *content_length   = "Content-Length: %d\r\n";
static const char *text_plain       = "Content-Type: text/plain\r\n";
static const char *image_gif        = "Content-Type: image/gif\r\n";

static const char *end              = "\r\n";

static const char *root = "\
CPTC (https://github.com/nakidai/cptc)\n\
--------------------------------------\n\
PetTheCord but rewritten in the C cuz some people was annoying me about that a\n\
lot.\n\
\n\
Paths:\n\
/           Show this text\n\
/<UID>.*    Return a gif that pets given UID\n";

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
    char buffer[512], filenamebuf[64];
    ssize_t received = recv(fd, buffer, sizeof(buffer), 0);
    FILE *fp;

    if (received == -1)
    {
        perror("recv()");
        return;
    }

    method = buffer[0];
    if (method != CPTC_GET && method != CPTC_HEAD)
    {
        send(fd, not_implemented, strlen(not_implemented), 0);
        return;
    }

    path = strchr(buffer, ' ') + 1;
    *strchr(path, ' ') = '\0';
    puts(path);

    if (strlen(path) == 1)
    {
        char *length = malloc(sizeof(*length) * 32);
        snprintf(length, 32, content_length, strlen(root));
        send(fd, ok, strlen(ok), 0);
        send(fd, text_plain, strlen(text_plain), 0);
        send(fd, length, strlen(length), 0);
        send(fd, end, strlen(end), 0);
        send(fd, root, strlen(root), 0);
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
            send(fd, forbidden, strlen(forbidden), 0);
            return;
        }
        generate_filename(filenamebuf, sizeof(filenamebuf), "gif");
        CPetPet(in, filenamebuf, 2);
        printf("From %s to %s\n", in, filenamebuf);

        fp = fopen(filenamebuf, "rb");
        if (!fp)
        {
            perror("fopen()");
            send(fd, intserverror, strlen(intserverror), 0);
            free(in);
            free(length);
            return;
        }
        fseek(fp, 0, SEEK_END);
        snprintf(length, 32, content_length, ftell(fp));
        fseek(fp, 0, SEEK_SET);

        send(fd, ok, strlen(ok), 0);
        send(fd, image_gif, strlen(image_gif), 0);
        send(fd, length, strlen(length), 0);
        send(fd, end, strlen(end), 0);
        while ((ch = getc(fp)) >= 0)
        {
            chbuf = ch;
            send(fd, &chbuf, 1, 0);
        }

        fclose(fp);
        free(in);
        free(length);
    }

    send(fd, notfound, strlen(notfound), 0);
}
