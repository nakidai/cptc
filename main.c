#include "cptc.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


const char *CPTC_token;

int main(void)
{
    CPTC_token = getenv("CPTC_TOKEN");
    CPTC("127.0.0.1", 8080);
}
