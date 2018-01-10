#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hircluster.h>

int main(int argc, char **argv) {
    unsigned int j;
    redisClusterContext *cc;
    redisReply *reply;
    int ret;
    const char *address = (argc > 1) ? argv[1] : "127.0.0.1:6379";
    int way = (argc > 2) ? atoi(argv[2]) : 0;
    const char *passwd = (argc > 3) ? argv[3] : NULL;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    if (way)
    {
        cc = redisClusterContextInit();
        if (cc)
        {
            ret = redisClusterSetOptionAddNodes(cc, address);
            if (ret != REDIS_OK)
            {
                printf("Connection error: %s\n", cc->errstr);
                redisClusterFree(cc);
                exit(1);
            }
            
            redisClusterSetOptionConnectTimeout(cc, timeout);
            if (passwd)
            {
                redisClusterSetOptionPassword(cc, passwd);
            }

            ret = redisClusterConnect2(cc);
            if (ret != REDIS_OK)
            {
                printf("Connection error: %s\n", cc->errstr);
                redisClusterFree(cc);
                exit(1);
            }
        }
    }
    else
    {
        cc = redisClusterConnectWithTimeout(address, timeout, 0);
    }
    
    if (cc == NULL || cc->err) {
        if (cc) {
            printf("Connection error: %s\n", cc->errstr);
            redisClusterFree(cc);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* not support PING server */
    //reply = redisClusterCommand(cc,"PING");
    //printf("PING: %s\n", reply->str);
    //freeReplyObject(reply);

    /* Set a key */
    reply = redisClusterCommand(cc,"SET %s %s", "foo", "hello world");
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set a key using binary safe API */
    reply = redisClusterCommand(cc,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    printf("SET (binary API): %s\n", reply->str);
    freeReplyObject(reply);

    /* Try a GET and two INCR */
    reply = redisClusterCommand(cc,"GET foo");
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);
    /* again ... */
    reply = redisClusterCommand(cc,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);

    /* Create a list of numbers, from 0 to 9 */
    reply = redisClusterCommand(cc,"DEL mylist");
    freeReplyObject(reply);
    for (j = 0; j < 10; j++) {
        char buf[64];

        snprintf(buf,64,"%d",j);
        reply = redisClusterCommand(cc,"LPUSH mylist element-%s", buf);
        freeReplyObject(reply);
    }

    /* Let's check what we have inside the list */
    reply = redisClusterCommand(cc,"LRANGE mylist 0 -1");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);

    /* Disconnects and frees the context */
    redisClusterFree(cc);

    return 0;
}
