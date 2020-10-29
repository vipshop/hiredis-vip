#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "hiredis_vip/hircluster.h"

int main(int argc, char **argv)
{
    redisContext *c;
    redisSSLContext *ssl;
    redisSSLContextError ssl_error;
    struct timeval timeout = { 1, 500000 }; // 1.5s

    // Get current work directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("getcwd() error");
        exit(1);
    }
    printf("Current path: %s\n", cwd);

    char cert[PATH_MAX];
    strcpy(cert, cwd);
    strcat(cert, "/../../configs/tls/redis.crt");
    printf("Cert: %s\n", cert);
    char key[PATH_MAX];
    strcpy(key, cwd);
    strcat(key, "/../../configs/tls/redis.key");
    printf("Key: %s\n", key);
    char ca[PATH_MAX];
    strcpy(ca, cwd);
    strcat(ca, "/../../configs/tls/ca.crt");
    printf("Ca: %s\n", ca);

    redisInitOpenSSL();
    ssl = redisCreateSSLContext(ca, NULL, cert, key, NULL, &ssl_error);
    if (!ssl) {
        printf("SSL Context error: %s\n",
                redisSSLContextGetError(ssl_error));
        exit(1);
    }

    redisOptions options = {0};
    REDIS_OPTIONS_SET_TCP(&options, "127.0.0.1", 33001);
    options.connect_timeout = &timeout;
    c = redisConnectWithOptions(&options);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    if (redisInitiateSSLWithContext(c, ssl) != REDIS_OK) {
        printf("Couldn't initialize SSL!\n");
        printf("Error: %s\n", c->errstr);
        redisFree(c);
        exit(1);
    }

    redisReply *reply = (redisReply*)redisCommand(c,"SET %s %s", "key", "value");
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    redisReply *reply2 = (redisReply*)redisCommand(c, "GET %s", "key");
    printf("GET: %s\n", reply2->str);
    freeReplyObject(reply2);

    redisFree(c);
    redisFreeSSLContext(ssl);
    return 0;
}
