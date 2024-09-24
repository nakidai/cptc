#include "cptc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>



static const char *ok               = "HTTP/1.0 200 OK\r\n";
static const char *notfound         = "HTTP/1.0 404 Not found\r\n";
static const char *not_implemented  = "HTTP/1.0 501 Not implemented\r\n";

static const char *content_length   = "Content-Length: %d\r\n";
static const char *text_plain       = "Content-Type: text/plain\r\n";

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

void CPTC_requestHandler(int fd)
{
    enum CPTC_Method method;
    char *path;
    char *buffer = (char *)malloc(sizeof(*buffer) * 512);
    ssize_t received = recv(fd, buffer, sizeof(*buffer) * 512, 0);

    if (received == -1)
    {
        perror("recv()");
        goto end;
    }

    method = buffer[0];
    if (method != CPTC_GET && method != CPTC_HEAD)
    {
        send(fd, not_implemented, strlen(not_implemented), 0);
        goto end;
    }

    path = strchr(buffer, ' ') + 1;
    *strchr(path, ' ') = '\0';
    puts(path);
    if (strlen(path) == 1)
    {
        send(fd, ok, strlen(ok), 0);
        char *length = malloc(sizeof(*length) * 32);
        snprintf(length, 32, content_length, strlen(root));
        send(fd, text_plain, strlen(text_plain), 0);
        send(fd, length, strlen(length), 0);
        send(fd, end, strlen(end), 0);
        send(fd, root, strlen(root), 0);
        free(length);
        goto end;
    }

    send(fd, notfound, strlen(notfound), 0);

end:
    free(buffer);
}
