/*
 * example-sync.c
 *
 *  Created on: 12.10.2018
 *      Author: tolj
 */

#include<stdio.h>
#include<hircluster.h>
int main()
{
    char *key="key-a";
    char *field="field-1";
    char *key1="key1";
    char *value1="value-1";
    char *key2="key1";
    char *value2="value-1";
    redisClusterContext *cc;

    cc = redisClusterContextInit();
    redisClusterSetOptionAddNodes(cc, "127.0.0.1:7000,127.0.0.1:7001,127.0.0.1:7002");
    redisClusterSetOptionParseSlaves(cc);
    redisClusterConnect2(cc);
    if(cc == NULL || cc->err)
    {
        printf("connect error : %s\n", cc == NULL ? "NULL" : cc->errstr);
        return -1;
    }

    redisReply *reply = redisClusterCommand(cc, "dbsize");
    if(reply == NULL)
    {
        printf("reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }

    printf("reply->type:%d\n", reply->type);
    printf("reply->str:%s\n", reply->str);

    freeReplyObject(reply);

    redisReply *reply = redisClusterCommand(cc, "hmget %s %s", key, field);
    if(reply == NULL)
    {
        printf("reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }

    printf("reply->type:%d", reply->type);

    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "mset %s %s %s %s", key1, value1, key2, value2);
    if(reply == NULL)
    {
        printf("reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }

    printf("reply->str:%s", reply->str);

    freeReplyObject(reply);
    redisClusterFree(cc);
    return 0;
}
