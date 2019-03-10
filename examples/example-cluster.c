#include <stdio.h>
#include <hiredis-vip/hircluster.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    srand(time(NULL));
    const int randNumber = rand() % 20;
    //Set up CLUSTER_ADDRESS IP:Port String
    char *tempClusterString[1024];
    if (argc > 1){
        int i;
        for(i = 1; i < argc; i++) {
            strcat(tempClusterString, argv[i]);
        }
    } else {
        strcat(tempClusterString, "127.0.0.1:7001");
    }
    printf("tempClusterString:[%s]\n", tempClusterString);

    const char *CLUSTER_ADDRESS = tempClusterString;
    char *key="key-a";
    char *field="field-a";
    char *value="value-a";
    char *field1="field1";
    char *value1="value1";
    char *field2="field/3";
    char *value2="value/3";
    redisClusterContext *cc;
    redisReply *reply;

    //Set up Redis Cluster Connection

    cc = redisClusterContextInit();
    redisClusterSetOptionRouteUseSlots(cc);
    redisClusterSetOptionMaxRedirect(cc, 3);
    //redisClusterSetOptionConnectTimeout(cc, tv);
    //redisClusterSetOptionTimeout(cc, tv);
    redisClusterSetOptionAddNodes(cc, CLUSTER_ADDRESS);
    redisClusterConnect2(cc);
    if(cc == NULL || cc->err)
    {
        printf("connect error : %s\n", cc == NULL ? "NULL" : cc->errstr);
        return -1;
    }

    //Ping (Unsupported)
    //Pub/Sub (Unsure)
    //Cluster Nodes (Unsupported)
    //Cluster Info (Unsupported)

    // publish
    reply = redisClusterCommand(cc, "publish channel msg");
    if (reply == NULL) {
        printf("Error with publish, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    printf("publish channel msg:%d\n", reply->integer);
    freeReplyObject(reply);

    //lists
    int listCounter;
    //lpush
    reply = redisClusterCommand(cc, "lpush ExampleList ListValue1 ListValueTwo");
    if (reply == NULL) {
        printf("Error with lpush, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    printf("Lpush[ExampleList]:%s\n", reply->str);
    freeReplyObject(reply);

    //lrange
    reply = redisClusterCommand(cc, "lrange ExampleList 0  -1");
    if (reply == NULL) {
        printf("Error with lrange, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    for(listCounter = 0; listCounter < reply->elements; listCounter++) {
        printf("lrange[ExampleList]:%s\n", reply->element[listCounter]->str);
    }
    freeReplyObject(reply);

    //lpop
    int newCounter;
    for (newCounter = 0; newCounter < listCounter; newCounter++) {
        reply = redisClusterCommand(cc, "lpop ExampleList");
        if (reply == NULL) {
            printf("Error with lpop, reply is null[%s]\n", cc->errstr);
            redisClusterFree(cc);
            return -1;
        }
        printf("lpop[ExampleList]:%s\n", reply->str);
        freeReplyObject(reply);
    }
    
    //del
    reply = redisClusterCommand(cc, "del ExampleCounter");
    if (reply == NULL) {
        printf("Error with del, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    printf("del[ExampleCounter]:%d\n", reply->str);
    freeReplyObject(reply);

    //incr
    int incrCounter;
    for(incrCounter=0; incrCounter < randNumber; incrCounter++) {
    	reply = redisClusterCommand(cc, "incr ExampleCounter");
    	if (reply == NULL){
    		printf("Error with incr, reply is null[%s]\n", cc->errstr);
		    redisClusterFree(cc);
		return -1;
    	}
   	    printf("INCR[ExampleCounter]:%d\n", reply->integer);
    	freeReplyObject(reply);
    }

    //del
    reply = redisClusterCommand(cc, "del ExampleCounter");
    if (reply == NULL) {
        printf("Error with del, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    printf("del[ExampleCounter]:%d\n", reply->str);
    freeReplyObject(reply);

    //decr
    int decrCounter;
    for(decrCounter=0; decrCounter < randNumber; decrCounter++) {
        reply = redisClusterCommand(cc, "decr ExampleCounter");
        if (reply == NULL){
            printf("Error with decr, reply is null[%s]\n", cc->errstr);
            redisClusterFree(cc);
        return -1;
        }
        printf("DECR[ExampleCounter]:%d\n", reply->integer);
        freeReplyObject(reply);
    }

    //hmset
    reply = redisClusterCommand(cc, "hmset %s %s %s %s %s %s %s", key, field, value, field1, value1, field2, value2);
    if(reply == NULL)
    {
        printf("Error with hmset, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    printf("HMSET:%s\n", reply->str);
    freeReplyObject(reply);

    //hmget
    reply = redisClusterCommand(cc, "hgetall %s", key);
    if(reply == NULL)
    {
        printf("Error with hmget, reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    printf("HMGET:\n");
    int hmgetCounter;
    for(hmgetCounter = 0; hmgetCounter < reply->elements; hmgetCounter++) {
        printf("%s\n", reply->element[hmgetCounter]->str);
    }
    freeReplyObject(reply);

    redisClusterFree(cc);
    return 0;
}

