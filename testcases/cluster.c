#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <hircluster.h>


void print_qps(struct timeval* begin, int* count) {
    struct timeval now;
    gettimeofday(&now, NULL);
    long diff_ms = (now.tv_sec - begin->tv_sec) * 1000 + (now.tv_usec - begin->tv_usec) / 1000;
    if (diff_ms > 1000) {
        printf("qps = %d\n", *count*1000 / (int)diff_ms);
        if (diff_ms > 60000) {
            *begin = now;
            *count = 0;
        }
    }
}

void _print_reply(redisReply* reply, size_t index, size_t depth) {
    size_t i;
    for (i=1; i<depth; i++) {
        printf("  ");
    }
    if (index > 0) {
        printf("%d) ", (int)index-1);
    }
    
    switch(reply->type) {
    case REDIS_REPLY_STRING:
        printf("\"%s\"", reply->str);
        break;
    case REDIS_REPLY_INTEGER:
        printf("%lld", reply->integer);
        break;
    case REDIS_REPLY_ERROR:
        printf("(ERROR) %s", reply->str);
        break;
    case REDIS_REPLY_STATUS:
        printf("(STATUS) %s", reply->str);
        break;
    case REDIS_REPLY_NIL:
        printf("(NIL)");
        break;
    case REDIS_REPLY_ARRAY:
        for (i=0; i<reply->elements; i++) {
            _print_reply(reply->element[i], i+1, depth+1);
        }
        return;
    default:
        assert(0);
        break;
    }
    printf("\n");
}

void print_reply(redisReply* reply) {
    _print_reply(reply, 0, 0);
}

int main(int argc, char **argv) {
    const int keyrange = 10000;
    const int interval = 10000;
    redisClusterContext *cc;
    redisReply *reply;
    const char *addrs = (argc > 1) ? argv[1] : "127.0.0.1:6379";
    const char *testcase = (argc > 2) ? argv[2] : "setget";
    int keystart = (argc > 3) ? atoi(argv[3]) * keyrange : keyrange;
    int limit = (argc > 4) ? atoi(argv[4]) : 0;
    int count;
    char key[256];
    char member[256];
    char value[256];
    char msg[1024];
    int line;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    cc = redisClusterContextInit();
    redisClusterSetOptionAddNodes(cc, addrs);
    redisClusterSetOptionRouteUseSlots(cc);
    redisClusterSetOptionParseSlaves(cc);
    redisClusterSetOptionSetAuth(cc, "abc");
    redisClusterSetOptionConnectTimeout(cc, timeout);
    redisClusterConnect2(cc);
    if (cc == NULL || cc->err) {
        if (cc) {
            printf("Connection error: %s\n", cc->errstr);
            redisClusterFree(cc);
        } else {
            printf("Connection error: can't allocate redis cluster context\n");
        }
        exit(1);
    }

    struct timeval begin;
    gettimeofday(&begin, NULL);
    srand(time(NULL));
    if (strcmp(testcase, "setget") == 0) {
        for (count=1;limit<=0||count<limit;count++) {
            sprintf(key, "%s_key_%d", testcase, keystart + count % keyrange);
            sprintf(value, "%d", rand());
            reply = redisClusterCommand(cc, "SET %s %s", key, value);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_STATUS);
            freeReplyObject(reply);
            reply = redisClusterCommand(cc, "GET %s", key);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            if (reply->type == REDIS_REPLY_NIL) {
                line = __LINE__;
                sprintf(msg, "nil reply value");
                goto reply_fail;
            }
            assert(reply->type == REDIS_REPLY_STRING);
            if (strcmp(reply->str, value) != 0) {
                line = __LINE__;
                sprintf(msg, "unexpected reply value '%s' vs '%s'", reply->str, value);
                goto reply_fail;
            }
            freeReplyObject(reply);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    } else if (strcmp(testcase, "hsethget") == 0) {
        for (count=1;limit<=0||count<limit;count++) {
            sprintf(key, "%s_key_%d", testcase, keystart + count % keyrange);
            sprintf(member, "%d", rand());
            sprintf(value, "%d", rand());
            reply = redisClusterCommand(cc, "HSET %s %s %s", key, member, value);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            freeReplyObject(reply);
            reply = redisClusterCommand(cc, "HGET %s %s", key, member);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            if (reply->type == REDIS_REPLY_NIL) {
                line = __LINE__;
                sprintf(msg, "nil reply value");
                goto reply_fail;
            }
            assert(reply->type == REDIS_REPLY_STRING);
            if (strcmp(reply->str, value) != 0) {
                line = __LINE__;
                sprintf(msg, "unexpected reply value '%s' vs '%s'", reply->str, value);
                goto reply_fail;
            }
            freeReplyObject(reply);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    }else if (strcmp(testcase, "zaddzrank") == 0) {
        const int ZSET_SIZE = 1000;
        for (count=1;limit<=0||count<limit;count++) {
            sprintf(key, "%s_key_%d", testcase, keystart + count % keyrange);
            sprintf(member, "%d", rand() % ZSET_SIZE);
            sprintf(value, "%d", rand());
            reply = redisClusterCommand(cc, "ZADD %s %s %s", key, value, member);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            freeReplyObject(reply);
            reply = redisClusterCommand(cc, "ZRANK %s %s", key, member);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            if (reply->type == REDIS_REPLY_NIL) {
                line = __LINE__;
                sprintf(msg, "nil reply rank");
                goto reply_fail;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            /*if (reply->integer) {
                line = __LINE__;
                sprintf(msg, "unexpected reply rank %d vs %d", reply->integer);
                goto reply_fail;
            }*/
            freeReplyObject(reply);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    }else if (strcmp(testcase, "saddsismem") == 0) {
        const int SET_SIZE = 1000;
        for (count=1;limit<=0||count<limit;count++) {
            sprintf(key, "%s_key_%d", testcase, keystart + count % keyrange);
            sprintf(member, "%d", rand() % SET_SIZE);
            reply = redisClusterCommand(cc, "SADD %s %s", key, member);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            freeReplyObject(reply);
            reply = redisClusterCommand(cc, "ZISMEMBER %s %s", key, member);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            if (reply->integer != 1) {
                line = __LINE__;
                sprintf(msg, "unexpected reply rank %d vs %d", reply->integer, 1);
                goto reply_fail;
            }
            freeReplyObject(reply);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    }else if (strcmp(testcase, "lpushlpop") == 0) {
        for (count=1;limit<=0||count<limit;count++) {
            sprintf(key, "%s_key_%d", testcase, keystart + count % keyrange);
            sprintf(value, "%d", rand());
            reply = redisClusterCommand(cc, "LPUSH %s %s", key, value);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            freeReplyObject(reply);
            reply = redisClusterCommand(cc, "LPOP %s", key);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            if (reply->type == REDIS_REPLY_NIL) {
                line = __LINE__;
                sprintf(msg, "nil reply value");
                goto reply_fail;
            }
            assert(reply->type == REDIS_REPLY_STRING);
            if (strcmp(reply->str, value) != 0) {
                line = __LINE__;
                sprintf(msg, "unexpected reply value '%s' vs '%s'", reply->str, value);
                goto reply_fail;
            }
            freeReplyObject(reply);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    }else if (strcmp(testcase, "msetmget") == 0) {
        const size_t MSET_MGET_COUNT = 5;
        const char* _argv[MSET_MGET_COUNT*2+1];
        char keyvalues[MSET_MGET_COUNT*2][256];
        size_t _argc, i;
        for (count=1;limit<=0||count<limit;count++) {
            _argc = 0;
            _argv[_argc] = "MSET";
            _argc ++;
            for (i=0; i<MSET_MGET_COUNT; i++) {
                sprintf(keyvalues[i*2], "%s_key_%d", testcase, keystart + (count+(int)i) % keyrange);
                _argv[_argc] = keyvalues[i*2];
                _argc ++;
                sprintf(keyvalues[i*2+1], "%d", rand());
                _argv[_argc] = keyvalues[i*2+1];
                _argc ++;
            }
            reply = redisClusterCommandArgv(cc, _argc, _argv, NULL);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_STATUS);
            freeReplyObject(reply);

            _argc = 0;
            _argv[_argc] = "MGET";
            _argc ++;
            for (i=0; i<MSET_MGET_COUNT; i++) {
                _argv[_argc] = keyvalues[i*2];
                _argc ++;
            }
            reply = redisClusterCommandArgv(cc, _argc, _argv, NULL);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_ARRAY);
            for (i=0; i<reply->elements; i++) {
                if (reply->element[i]->type == REDIS_REPLY_NIL) {
                    line = __LINE__;
                    sprintf(msg, "nil reply value");
                    goto reply_fail;
                }
                assert(reply->element[i]->type == REDIS_REPLY_STRING);
                if (strcmp(reply->element[i]->str, keyvalues[i*2+1]) != 0) {
                    line = __LINE__;
                    sprintf(msg, "unexpected reply value '%s' vs '%s'", reply->element[i]->str, keyvalues[i*2+1]);
                    goto reply_fail;
                }
            }
            freeReplyObject(reply);

            if (count % (interval/MSET_MGET_COUNT) == 0) {
                print_qps(&begin, &count);
            }
        }
    } else if (strcmp(testcase, "eval") == 0) {
        const char* SCRIPT = "local sum=0; for i,k in ipairs(KEYS) do sum=sum+redis.call('INCR', k) end return sum";
        const size_t SCRIPT_KEYS_COUNT = 5;
        const char* _argv[SCRIPT_KEYS_COUNT+3];
        char keys[SCRIPT_KEYS_COUNT][256];
        size_t _argc, i;
        for (count=0;limit<=0||count<limit;count++) {
            _argc = 0;
            _argv[_argc] = "EVAL";
            _argc ++;
            _argv[_argc] = SCRIPT;
            _argc ++;
            sprintf(value, "%d", (int)SCRIPT_KEYS_COUNT);
            _argv[_argc] = value;
            _argc ++;
            for (i=0; i<SCRIPT_KEYS_COUNT; i++) {
                sprintf(keys[i], "{%s_%d}_key_%d", testcase, keystart, rand()%keyrange);
                _argv[_argc] = keys[i];
                _argc ++;
            }
            reply = redisClusterCommandArgv(cc, _argc, _argv, NULL);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_INTEGER);
            if ((int)reply->integer < (int)SCRIPT_KEYS_COUNT) {
                line = __LINE__;
                sprintf(msg, "unexpected reply value %d < %d", (int)reply->integer, (int)SCRIPT_KEYS_COUNT);
                goto reply_fail;
            }
            freeReplyObject(reply);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    } else if (strcmp(testcase, "pipeline") == 0) {
        for (count=1;limit<=0||count<limit;count++) {
            sprintf(key, "%s_key_%d", testcase, keystart + count % keyrange);
            sprintf(value, "%d", rand());
            redisClusterAppendCommand(cc, "SET %s %s", key, value);
            redisClusterAppendCommand(cc, "GET %s", key);
            redisClusterGetReply(cc, &reply);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_STATUS);
            freeReplyObject(reply);
            redisClusterGetReply(cc, &reply);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            assert(reply->type == REDIS_REPLY_STRING);
            if (strcmp(reply->str, value) != 0) {
                line = __LINE__;
                sprintf(msg, "unexpected reply value '%s' vs '%s'", reply->str, value);
                goto reply_fail;
            }
            freeReplyObject(reply);
            redisClusterReset(cc);

            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    } else if (strcmp(testcase, "once") == 0) {
        reply = redisClusterCommandArgv(cc, argc-3, argv+3, NULL);
        if (reply == NULL) {
            line = __LINE__;
            goto reply_error;
        }
        print_reply(reply);
        freeReplyObject(reply);
    } else if (strcmp(testcase, "loop") == 0) {
        for (count=1;limit<=0||count<limit;count++) {
            reply = redisClusterCommandArgv(cc, argc-3, argv+3, NULL);
            if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
                line = __LINE__;
                goto reply_error;
            }
            freeReplyObject(reply);
            if (count % interval == 0) {
                print_qps(&begin, &count);
            }
        }
    } else {
        printf("invalid testcase: %s\n", testcase);
    }
    
    redisClusterFree(cc);
    return 0;

reply_error:
    if (reply) {
        printf("line %d: Command error: %s\n", line, reply->str);
        freeReplyObject(reply);
    } else {
        printf("line %d: Connection error: %s\n", line, cc->errstr);
    }
    redisClusterFree(cc);
    return 1;

reply_fail:
    printf("line %d: %s\n", line, msg);
    freeReplyObject(reply);
    redisClusterFree(cc);
    return 2;
}
