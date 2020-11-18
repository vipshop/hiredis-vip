#include "adapters/libevent.h"
#include "hircluster.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLUSTER_NODE "127.0.0.1:30001"

// Test of two pipelines using sync API
void test_pipeline() {
    redisClusterContext *cc = redisClusterContextInit();
    assert(cc);
    redisClusterSetOptionAddNodes(cc, CLUSTER_NODE);
    redisClusterConnect2(cc);

    assert(cc->err == 0);

    int status;
    status = redisClusterAppendCommand(cc, "SET foo one");
    assert(status == REDIS_OK);
    status = redisClusterAppendCommand(cc, "SET bar two");
    assert(status == REDIS_OK);
    status = redisClusterAppendCommand(cc, "GET foo");
    assert(status == REDIS_OK);
    status = redisClusterAppendCommand(cc, "GET bar");
    assert(status == REDIS_OK);

    redisReply *reply;
    redisClusterGetReply(cc, (void *)&reply); // reply for: SET foo one
    assert(reply != NULL);
    assert(reply->type == REDIS_REPLY_STATUS);
    assert(strcmp(reply->str, "OK") == 0);
    freeReplyObject(reply);

    redisClusterGetReply(cc, (void *)&reply); // reply for: SET bar two
    assert(reply != NULL);
    assert(reply->type == REDIS_REPLY_STATUS);
    assert(strcmp(reply->str, "OK") == 0);
    freeReplyObject(reply);

    redisClusterGetReply(cc, (void *)&reply); // reply for: GET foo
    assert(reply != NULL);
    assert(reply->type == REDIS_REPLY_STRING);
    assert(strcmp(reply->str, "one") == 0);
    freeReplyObject(reply);

    redisClusterGetReply(cc, (void *)&reply); // reply for: GET bar
    assert(reply != NULL);
    assert(reply->type == REDIS_REPLY_STRING);
    assert(strcmp(reply->str, "two") == 0);
    freeReplyObject(reply);

    redisClusterFree(cc);
}

//------------------------------------------------------------------------------
// Async API
//------------------------------------------------------------------------------

typedef struct ExpectedResult {
    int type;
    char *str;
    bool disconnect;
} ExpectedResult;

// Callback for Redis connects and disconnects
void callbackExpectOk(const redisAsyncContext *ac, int status) {
    UNUSED(ac);
    assert(status == REDIS_OK);
}

// Callback for async commands, verifies the redisReply
void commandCallback(redisClusterAsyncContext *cc, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    ExpectedResult *expect = (ExpectedResult *)privdata;
    assert(reply != NULL);
    assert(reply->type == expect->type);
    assert(strcmp(reply->str, expect->str) == 0);

    if (expect->disconnect) {
        redisClusterAsyncDisconnect(cc);
    }
}

// Test of two pipelines using async API
// In an asynchronous context, commands are automatically pipelined due to the
// nature of an event loop. Therefore, unlike the synchronous API, there is only
// a single way to send commands.
void test_async_pipeline() {
    redisClusterAsyncContext *acc = redisClusterAsyncContextInit();
    assert(acc);
    redisClusterAsyncSetConnectCallback(acc, callbackExpectOk);
    redisClusterAsyncSetDisconnectCallback(acc, callbackExpectOk);
    redisClusterSetOptionAddNodes(acc->cc, CLUSTER_NODE);
    redisClusterConnect2(acc->cc);

    assert(acc->err == 0);

    struct event_base *base = event_base_new();
    redisClusterLibeventAttach(acc, base);

    int status;
    ExpectedResult r1 = {.type = REDIS_REPLY_STATUS, .str = "OK"};
    status = redisClusterAsyncCommand(acc, commandCallback, &r1, "SET foo six");
    assert(status == REDIS_OK);

    ExpectedResult r2 = {.type = REDIS_REPLY_STATUS, .str = "OK"};
    status = redisClusterAsyncCommand(acc, commandCallback, &r2, "SET bar ten");
    assert(status == REDIS_OK);

    ExpectedResult r3 = {.type = REDIS_REPLY_STRING, .str = "six"};
    status = redisClusterAsyncCommand(acc, commandCallback, &r3, "GET foo");
    assert(status == REDIS_OK);

    ExpectedResult r4 = {
        .type = REDIS_REPLY_STRING, .str = "ten", .disconnect = true};
    status = redisClusterAsyncCommand(acc, commandCallback, &r4, "GET bar");
    assert(status == REDIS_OK);

    event_base_dispatch(base);

    redisClusterAsyncFree(acc);
    event_base_free(base);
}

int main() {

    test_pipeline();

    test_async_pipeline();

    return 0;
}
