
#ifndef __HIRCLUSTER_H
#define __HIRCLUSTER_H

#include "hiredis.h"
#include "adlist.h"
#include "async.h"

#define REDIS_CLUSTER_SLOTS 16384

struct dict;

typedef struct cluster_node
{
	sds name;
	sds addr;
	sds host;
	int port;
	int count;
	uint8_t master;
	redisContext *con;
	redisAsyncContext *acon;
	list *slots;
	struct cluster_node *slave_of;
}cluster_node;

typedef struct cluster_slot
{
	uint32_t start;
	uint32_t end;
	cluster_node *node;
}cluster_slot;

#ifdef __cplusplus
extern "C" {
#endif

/* Context for a connection to Redis cluster */
typedef struct redisClusterContext {
    int err; /* Error flags, 0 when there is no error */
    char errstr[128]; /* String representation of error when applicable */
	sds ip;
	int port;

	int flags;

    enum redisConnectionType connection_type;
    struct timeval *timeout;
	
	struct hiarray *slots;

	struct dict *nodes;
	cluster_node *table[REDIS_CLUSTER_SLOTS];

	uint64_t route_version;

	int max_redirect_count;
	int retry_count;

	list *requests;

	int need_update_route;
} redisClusterContext;

redisClusterContext *redisClusterConnect(const char *addrs);
redisClusterContext *redisClusterConnectWithTimeout(const char *addrs, 
	const struct timeval tv);
redisClusterContext *redisClusterConnectNonBlock(const char *addrs);


void redisClusterFree(redisClusterContext *cc);

void redisClusterSetMaxRedirect(redisClusterContext *cc, int max_redirect_count);

void *redisClusterCommand(redisClusterContext *cc, const char *format, ...);

redisContext *ctx_get_by_node(struct cluster_node *node, const struct timeval *timeout, int flags);

int redisClusterAppendCommand(redisClusterContext *cc, const char *format, ...);
int redisClusterAppendCommandArgv(redisClusterContext *cc, int argc, const char **argv);
int redisClusterGetReply(redisClusterContext *cc, void **reply);
void redisCLusterReset(redisClusterContext *cc);


/*############redis cluster async############*/

struct redisClusterAsyncContext;

typedef int (adapterAttachFn)(struct redisAsyncContext*, void*);

typedef void (redisClusterCallbackFn)(struct redisClusterAsyncContext*, void*, void*);

/* Context for an async connection to Redis */
typedef struct redisClusterAsyncContext {
    
    redisClusterContext *cc;

    /* Setup error flags so they can be used directly. */
    int err;
    char errstr[128]; /* String representation of error when applicable */

    /* Not used by hiredis */
    void *data;

	void *adapter;
	adapterAttachFn *attach_fn;

    /* Called when either the connection is terminated due to an error or per
     * user request. The status is set accordingly (REDIS_OK, REDIS_ERR). */
    redisDisconnectCallback *onDisconnect;

    /* Called when the first write event was received. */
    redisConnectCallback *onConnect;

} redisClusterAsyncContext;

redisClusterAsyncContext *redisClusterAsyncConnect(const char *addrs);
int redisClusterAsyncSetConnectCallback(redisClusterAsyncContext *acc, redisConnectCallback *fn);
int redisClusterAsyncSetDisconnectCallback(redisClusterAsyncContext *acc, redisDisconnectCallback *fn);
int redisClusterAsyncCommand(redisClusterAsyncContext *acc, redisClusterCallbackFn *fn, void *privdata, const char *format, ...);
void redisClusterAsyncDisconnect(redisClusterAsyncContext *acc);
void redisClusterAsyncFree(redisClusterAsyncContext *acc);

#ifdef __cplusplus
}
#endif

#endif
