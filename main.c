#define _POSIX_C_SOURCE 200112L

#include "cptc.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>


const char *CPTC_token;

static struct option long_options[] =
{
    {"help", no_argument, NULL, 'h'},
    {"host", no_argument, NULL, 'i'},
    {"port", no_argument, NULL, 'p'},
    {0}
};

noreturn void usage(char *name, bool full)
{
    fprintf(
        full ? stdout : stderr,
        full ?
            "usage: %s [-h] [-i/--host IP] [-p/--port PORT]\n"
            "Also requires CPTC_TOKEN environment variable set to some discord\n"
            "bot's token\n\n"
            "Options:\n"
            "  -h, --help       show this help message and quit\n"
            "  -i, --host IP    set bind address to IP )default: 127.0.0.1)\n"
            "  -p, --port PORT  set bind port to PORT (default: 8080)\n"
        :
            "Try '%s --help' for more information\n",
        name
    );
    exit(!full);
}

static bool isnumber(const char *s)
{
    for (const char *cp = s; *cp != '\0'; ++cp)
        if (!isdigit(*cp))
            return false;
    return true;
}

int main(int argc, char **argv)
{
    char *address = "127.0.0.1";
    int port = 8080;
    int ch;

    CPTC_token = getenv("CPTC_TOKEN");
    while ((ch = getopt_long(argc, argv, "hi:p:", long_options, NULL)) != EOF)
    {
        switch (ch)
        {
        case 'i':
            address = optarg;
            break;
        case 'p':
            if (!isnumber(optarg))
            {
                fprintf(stderr, "%s: port should be an unsigned integer\n", argv[0]);
                usage(argv[0], false);
            }
            port = atoi(optarg);
            break;
        case 'h':
            usage(argv[0], true);
            /* NOTREACHED */
        default:
            usage(argv[0], false);
            /* NOTREACHED */
        }
    }

    CPTC(address, port);
}
