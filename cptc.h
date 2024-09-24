#ifndef __CPTC_H__
#define __CPTC_H__

#include <netinet/in.h>


enum CPTC_Method
{
    CPTC_GET = 'G',
    CPTC_HEAD = 'H',
};

void CPTC(const char *address, in_port_t port);
void CPTC_requestHandler(int fd);

#endif /* __CPTC_H__ */
