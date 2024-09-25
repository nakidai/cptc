#ifndef __CPTC_H__
#define __CPTC_H__

#include <netinet/in.h>


enum CPTC_Method
{
    CPTC_GET = 'G',
    CPTC_HEAD = 'H',
};

extern const char *CPTC_token;

void CPTC(const char *address, in_port_t port);
void CPTC_requestHandler(int fd);
char *CPTC_downloadAvatar(long long uid, const char *download_path);

#endif /* __CPTC_H__ */
