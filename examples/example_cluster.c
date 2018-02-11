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
    int flag = (argc > 4) ? atoi(argv[4]) : 0;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    if(way == 1)
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

            if(flag == 2)
            {
                redisClusterSetOptionParseSlaves(cc);
            }

            if(flag == 3)
            {
                redisClusterSetOptionParseOpenSlots(cc);
            }

            if(flag == 4)
            {
                redisClusterSetOptionRouteUseSlots(cc);
            }

            if(flag == 5)
            {
                redisClusterSetOptionParseSlaves(cc);
                redisClusterSetOptionParseOpenSlots(cc);
            }

            if(flag == 6)
            {
                redisClusterSetOptionParseSlaves(cc);
                redisClusterSetOptionRouteUseSlots(cc);
            }

            if(flag == 7)
            {
                redisClusterSetOptionParseOpenSlots(cc);
                redisClusterSetOptionRouteUseSlots(cc);
            }

            if(flag == 8)
            {
                redisClusterSetOptionParseSlaves(cc);
                redisClusterSetOptionParseOpenSlots(cc);
                redisClusterSetOptionRouteUseSlots(cc);
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
        if(way == 2) flag = HIRCLUSTER_FLAG_ADD_SLAVE;
        if(way == 3) flag = HIRCLUSTER_FLAG_ADD_OPENSLOT;
        if(way == 4) flag = HIRCLUSTER_FLAG_ROUTE_USE_SLOTS;
        if(way == 5) flag = HIRCLUSTER_FLAG_ADD_SLAVE | HIRCLUSTER_FLAG_ADD_OPENSLOT;
        if(way == 6) flag = HIRCLUSTER_FLAG_ADD_SLAVE | HIRCLUSTER_FLAG_ROUTE_USE_SLOTS;
        if(way == 7) flag = HIRCLUSTER_FLAG_ADD_OPENSLOT | HIRCLUSTER_FLAG_ROUTE_USE_SLOTS;
        if(way == 8) flag = HIRCLUSTER_FLAG_ADD_SLAVE | HIRCLUSTER_FLAG_ADD_OPENSLOT | HIRCLUSTER_FLAG_ROUTE_USE_SLOTS;
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

    /* test */
    reply = redisClusterCommand(cc, "get 1");
    printf( "get 1: %lld\n", reply->integer );
    freeReplyObject(reply);
    reply = redisClusterCommand(cc, "get 2");
    printf( "get 2: %lld\n", reply->integer );
    freeReplyObject(reply);
    reply = redisClusterCommand(cc, "get 3");
    printf( "get 3: %lld\n", reply->integer );
    freeReplyObject(reply);
    reply = redisClusterCommand(cc, "get 4");
    printf( "get 4: %lld\n", reply->integer );
    freeReplyObject(reply);
    reply = redisClusterCommand(cc, "get 5");
    printf( "get 5: %lld\n", reply->integer );
    freeReplyObject(reply);
    reply = redisClusterCommand(cc, "get 6");
    printf( "get 6: %lld\n", reply->integer );
    freeReplyObject(reply);

    /* del */
    reply = redisClusterCommand(cc, "del foo");
    printf( "del foo: %lld\n", reply->integer );
    freeReplyObject(reply);

    /* hset */
    reply = redisClusterCommand(cc, "hset foo abc 1");
    printf( "hset foo abc 1: %lld\n", reply->integer );
    freeReplyObject(reply);

    /* eval */
    const char *cmds[] =
    {
        "eval",
        "return redis.call('hget',KEYS[1],ARGV[1])",
        "1",
        "foo",
        "abc"
    };
    reply = redisClusterCommandArgv(cc, sizeof(cmds)/sizeof(const char*), cmds, NULL );
    printf( "hget foo abc: %s\n", reply->str );
    freeReplyObject(reply);

    /* del */
    reply = redisClusterCommand(cc, "del foo");
    printf( "del foo: %lld\n", reply->integer );
    freeReplyObject(reply);

    /* support rejson */
    reply = redisClusterCommand(cc, "JSON.SET %s . %s", "json", "{}");
    printf("JSON.SET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.GET %s", "json");
    printf("JSON.GET json: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.SET %s .%s %s", "json", "foo", "1");
    printf("JSON.SET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.SET json .arr []" );
    printf("JSON.SET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.TYPE json ." );
    printf("JSON.TYPE: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.TYPE json .arr" );
    printf("JSON.TYPE: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.NUMINCRBY json .foo 1" );
    printf("JSON.NUMINCRBY: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.ARRAPPEND json .arr 1" );
    printf("JSON.ARRAPPEND: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.NUMMULTBY json .arr[0] 2" );
    printf("JSON.NUMMULTBY: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.SET json .str \"\"");
    printf("JSON.SET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.STRAPPEND json .str \"1\"");
    printf("JSON.STRAPPEND: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.STRLEN json .str");
    printf("JSON.STRLEN: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.ARRINDEX json .arr 2" );
    printf("JSON.ARRINDEX: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.ARRINSERT json .arr 0 1" );
    printf("JSON.ARRINSERT: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.ARRLEN json .arr" );
    printf("JSON.ARRLEN: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.ARRPOP json .arr" );
    printf("JSON.ARRPOP: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.ARRTRIM json .arr 1 1" );
    printf("JSON.ARRTRIM: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.OBJKEYS json ." );
    printf("JSON.OBJKEYS: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.OBJLEN json ." );
    printf("JSON.OBJLEN: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.SET foo . {}" );
    printf("JSON.SET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.MGET json ." );
    printf("JSON.MGET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.MGET json foo ." );
    printf("JSON.MGET: %s\n", reply->str);
    freeReplyObject(reply);

    redisClusterAppendCommand(cc, "JSON.MGET json foo .");
    redisClusterGetReply(cc, &reply);
    printf("JSON.MGET: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.GET %s", "json");
    printf("JSON.GET json: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.GET %s .%s", "json", "foo");
    printf("JSON.GET json.foo: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.DEL %s .%s", "json", "foo");
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.GET %s", "json");
    printf("JSON.GET json: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisClusterCommand(cc, "JSON.DEL %s .", "json");
    freeReplyObject(reply);

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
