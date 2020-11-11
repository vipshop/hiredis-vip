#include "hircluster.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_MSG(_x, _msg)                                                   \
    if (!(_x)) {                                                               \
        fprintf(stderr, "ERROR: %s\n", _msg);                                  \
        assert(_x);                                                            \
    }
#define REPLY(_ctx, _reply)                                                    \
    if (!(_reply)) {                                                           \
        ASSERT_MSG(_reply, _ctx->errstr);                                      \
    }
#define REPLY_TYPE(_reply, _type)                                              \
    ASSERT_MSG((_reply->type == _type), "Reply type incorrect");

#define CHECK_REPLY_OK(_ctx, _reply)                                           \
    {                                                                          \
        REPLY(_ctx, _reply);                                                   \
        REPLY_TYPE(_reply, REDIS_REPLY_STATUS);                                \
        ASSERT_MSG((strcmp(_reply->str, "OK") == 0), _ctx->errstr);            \
    }
#define CHECK_REPLY_INT(_ctx, _reply, _value)                                  \
    {                                                                          \
        REPLY(_ctx, _reply);                                                   \
        REPLY_TYPE(_reply, REDIS_REPLY_INTEGER);                               \
        ASSERT_MSG((_reply->integer == _value), _ctx->errstr);                 \
    }

void test_exists(redisClusterContext *cc) {
    redisReply *reply;
    reply = (redisReply *)redisClusterCommand(cc, "SET key1 Hello");
    CHECK_REPLY_OK(cc, reply);
    freeReplyObject(reply);

    reply = (redisReply *)redisClusterCommand(cc, "EXISTS key1");
    CHECK_REPLY_INT(cc, reply, 1);
    freeReplyObject(reply);

    reply = (redisReply *)redisClusterCommand(cc, "EXISTS nosuchkey");
    CHECK_REPLY_INT(cc, reply, 0);
    freeReplyObject(reply);

    reply = (redisReply *)redisClusterCommand(cc, "SET key2 World");
    CHECK_REPLY_OK(cc, reply);
    freeReplyObject(reply);

    reply = (redisReply *)redisClusterCommand(cc, "EXISTS key1 key2 nosuchkey");
    CHECK_REPLY_INT(cc, reply, 2);
    freeReplyObject(reply);
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    struct timeval timeout = {0, 500000};

    redisClusterContext *cc = redisClusterContextInit();
    assert(cc);
    redisClusterSetOptionAddNodes(cc, "127.0.0.1:30001");
    redisClusterSetOptionConnectTimeout(cc, timeout);
    redisClusterSetOptionRouteUseSlots(cc);
    redisClusterConnect2(cc);
    assert(cc->err == 0);

    test_exists(cc);

    redisClusterFree(cc);
    return 0;
}
