#include <stdio.h>
#include <stdlib.h>
#include "hircluster.h"

int main(int argc, char **argv)
{
    struct timeval timeout = { 1, 500000 }; // 1.5s

    redisClusterContext *cc = redisClusterContextInit();
    redisClusterSetOptionAddNodes(cc, "::1:30001");
    redisClusterSetOptionConnectTimeout(cc, timeout);
    redisClusterSetOptionRouteUseSlots(cc);
    redisClusterConnect2(cc);
    if (cc && cc->err) {
        printf("Error: %s\n", cc->errstr);
        // handle error
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
