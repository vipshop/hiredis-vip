#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hircluster.h>

int main(int argc, char **argv) {
    redisReply *reply;
    redisClusterContext *cc = redisClusterContextInit();
	cc->passwd = "foobared";
    redisClusterSetOptionAddNodes(cc, "127.0.0.1:7000,127.0.0.1:7001,127.0.0.1:7002,127.0.0.1:7003,127.0.0.1:7004,127.0.0.1:7005");
    redisClusterConnect2(cc);
    if (cc != NULL && cc->err) {
        printf("Error: %s\n", cc->errstr);
        // handle error
		return 1;
    }

    reply = redisClusterCommand(cc, "MSET %s %s %s %s", "name", "david", "company", "kugou");
    printf("MSET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "MSET %s %s %s %s", "fuck", "you", "123", "value");
    printf("MSET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "MGET %s %s", "name", "company");
    // printf("MSET: %s\n", reply->str);
    printf("MSET: %s\n", reply->element[0]->str);
    printf("MSET: %s\n", reply->element[1]->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "MGET %s %s", "fuck", "123");
    printf("MSET: %s\n", reply->element[0]->str);
    printf("MSET: %s\n", reply->element[1]->str);
    freeReplyObject(reply);

    redisClusterFree(cc);

    return 0;
}
