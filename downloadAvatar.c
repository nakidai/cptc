#define _POSIX_C_SOURCE 200112L

#include "cptc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>
#include <curl/easy.h>


static size_t write_function(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct {char *buf; size_t size;} *mem = userp;

    mem->buf = realloc(mem->buf, mem->size + realsize + 1);
    if (!mem->buf)
    {
        perror("realloc()");
        return 0;
    }

    memcpy(&(mem->buf[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->buf[mem->size] = 0;

    return realsize;
}

char *CPTC_downloadAvatar(long long uid, const char *filenamebuf)
{
    char *avatar, link[256], auth[128], *ret;
    CURL *curl;
    CURLcode res;
    FILE *fp;
    struct {char *buf; size_t size;} buffer;
    buffer.buf = malloc(1);
    buffer.size = 0;

    curl = curl_easy_init();
    if (curl)
    {
        snprintf(link, sizeof(link), "https://discord.com/api/v9/users/%lld", uid);
        snprintf(auth, sizeof(auth), "Authorization: %s", CPTC_token);

        curl_easy_setopt(curl, CURLOPT_URL, link);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, auth));

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform(): %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            free(buffer.buf);
            return NULL;
        }

        curl_easy_cleanup(curl);
    } else
    {
        free(buffer.buf);
        return NULL;
    }
    avatar = strstr(buffer.buf, "avatar\":\"") + 9;
    if (!(avatar - 9))
    {
        fprintf(stderr, "User %lld has no avatar\n", uid);
        free(buffer.buf);
        return NULL;
    }
    *strchr(avatar, '"') = '\0';

    fp = fopen(filenamebuf, "w");
    if (!fp)
    {
        perror("fopen()");
        free(buffer.buf);
        return NULL;
    }

    curl = curl_easy_init();
    if (curl)
    {
        snprintf(link, sizeof(link), "https://cdn.discordapp.com/avatars/%lld/%s.png", uid, avatar);

        curl_easy_setopt(curl, CURLOPT_URL, link);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, auth));

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform(): %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            fclose(fp);
            free(buffer.buf);
            return NULL;
        }

        curl_easy_cleanup(curl);
    } else
    {
        fclose(fp);
        free(buffer.buf);
        return NULL;
    }

    fclose(fp);
    ret = malloc(strlen(filenamebuf) + 1);
    memcpy(ret, filenamebuf, strlen(filenamebuf) + 1);
    free(buffer.buf);
    return ret;
}
