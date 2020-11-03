#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hircluster.h"

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    struct timeval timeout = { 1, 500000 }; // 1.5s

    redisClusterContext *cc = redisClusterContextInit();
    assert(cc);
    redisClusterSetOptionConnectTimeout(cc, timeout);
    redisClusterSetOptionRouteUseSlots(cc);

    if (redisClusterSetOptionAddNodes(cc, "::1:30001") != REDIS_OK) {
        printf("Error: %s\n", cc->errstr);
        exit(-1);
    }
    if (redisClusterConnect2(cc) != REDIS_OK) {
        printf("Error: %s\n", cc->errstr);
        exit(-1);
    }

    redisReply *reply = (redisReply*)redisClusterCommand(cc, "SET %s %s", "key", "value");
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    redisReply *reply2 = (redisReply*)redisClusterCommand(cc, "GET %s", "key");
    printf("GET: %s\n", reply2->str);
    freeReplyObject(reply2);

    redisClusterFree(cc);
    return 0;
}
