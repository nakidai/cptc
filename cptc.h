#ifndef __CPTC_H__
#define __CPTC_H__

#include <netinet/in.h>


/**
 * HTTP methods that CPTC supports
 */
enum CPTC_Method
{
    CPTC_GET = 'G',
    CPTC_HEAD = 'H',
};

/**
 * Discord bot's token. CPTC_downloadAvatar requires it when requests avatar ID
 *
 * Library users should create it and set before calling CPTC
 */
extern const char *CPTC_token;

/**
 * Contents of README file. CPTC_requestHandler responses with this when
 * requesting on "/"
 */
extern const char *CPTC_root;

/**
 * Starts CPTC server
 * @param address Adress to bind
 * @param port Port to bind
 * @param id Worker ID
 */
void CPTC(const char *address, in_port_t port, unsigned id);

/**
 * Handles request
 * Paths:
 *  /         README
 *  /<UID>.*  GIF that pets given UID
 * @param fd Request
 * @param n Any number, used for generating random files in /tmp/
 */
void CPTC_requestHandler(int fd, int n);

/**
 * Downloads given user's avatar
 * @param uid UserID
 * @param download_path Path where to save avatar.
 */
char *CPTC_downloadAvatar(long long uid, const char *download_path);

#endif /* __CPTC_H__ */
