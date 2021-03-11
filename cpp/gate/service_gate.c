#include "skynet.h"
#include "skynet_socket.h"
#include "databuffer.h"
#include "hashid.h"
#include "skynet_env.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>


#define BACKLOG 32

struct connection {
    int id;
    uint32_t agent;
    uint32_t client;
    char remote_name[32];
    struct databuffer buffer;
};

struct gate {
    struct skynet_context *ctx;
    int listen_id;      // 用于监听的id???--comment by dengkai
    uint32_t watchdog;
    uint32_t broker;
    int client_tag;
    int header_size;
    int max_connection;
    struct hashid hash;
    struct connection *conn; // 客户端的连接???--comment by dengkai
    struct messagepool mp;

    char bind[200];
};

static void
_parm(char *msg, int sz, int command_sz) {
    while (command_sz < sz) {
        if (msg[command_sz] != ' ') {
            break;
        }
        ++command_sz;
    }

    int i;
    for (i = command_sz; i < sz; ++i) {
        msg[i-command_sz] = msg[i];
    }
    msg[i-command_sz] = '\0';
}

static void
_forward_agent(struct gate *g, int fd, uint32_t agentaddr, uint32_t clientaddr) {
    int id = hashid_lookup(&g->hash, fd);
    if (id > 0) {
        struct connection * agent = &g->conn[id];
        agent->agent = agentaddr;
        agent->client = clientaddr;
    }
}

static int
start_listen(struct gate *g, char * listen_addr) {
    // listen_addr的格式为"ip:port"或"port"
    struct skynet_context *ctx = g->ctx;
    char * portstr = strchr(listen_addr, ':');
    const char * host = "";
    int port;
    if (portstr == NULL) {
        port = strtol(listen_addr, NULL, 10);
        if (port <= 0) {
            skynet_error(ctx, "Invalid gate address %s", listen_addr);
            return 1;
        }
    } else {
        port = strtol(portstr + 1, NULL, 10);
        if (port <= 0) {
            skynet_error(ctx, "Invalid gate address %s", listen_addr);
            return 1;
        }
        portstr[0] = '\0';
        host = listen_addr;
    }

    g->listen_id = skynet_socket_listen(ctx, host, port, BACKLOG);
    if (g->listen_id < 0) {
        return 1;
    }
    skynet_socket_start(ctx, g->listen_id);
    skynet_error(ctx, "start listen address %s", listen_addr);
    return 0;
}

static void
_forward(struct gate *g, struct connection *c, int size) {
    struct skynet_context * ctx = g->ctx;
    if (g->broker) {
        //TODO --comment by dengkai
        return;
    }

    if (c->agent) {
        // TODO --comment by dengkai
    } else if (g->watchdog) {
        char * tmp = skynet_malloc(size + sizeof(int));
        int n = snprintf(tmp, sizeof(int), "%d", c->id); // watchdog中解析出来的fd是从这里来的--comment by dengkai
        databuffer_read(&c->buffer, &g->mp, tmp+n, size);
        // 由gate发送到watchdog, 协议类型是PTYPE_CLIENT 大小为size + 32byte的数据
        skynet_send(ctx, 0, g->watchdog, g->client_tag | PTYPE_TAG_DONTCOPY, 0, tmp, size+n);
    }
}

static void
dispatch_message(struct gate *g, struct connection *c, int id, void *data, int sz) {
    // data参数中的数据是什么格式呢???--comment by dengkai
    databuffer_push(&c->buffer, &g->mp, data, sz);
    for (;;) {
        int size = databuffer_readheader(&c->buffer, &g->mp, g->header_size);
        if (size < 0) {
            return;
        } else if (size > 0) {
            if (size >= 0x1000000) {
                struct skynet_context * ctx = g->ctx;
                databuffer_clear(&c->buffer,&g->mp);
                skynet_socket_close(ctx, id);
                skynet_error(ctx, "Recv socket message > 16M");
                return;
            } else {
                //  接受socket数据，并转发到(watchdog, agent, broker)
                //skynet_error(NULL, "Accept socket and send to watchdog, agent, broker");
                _forward(g, c, size);
                databuffer_reset(&c->buffer);
            }
        }
    }
}

//把消息转发给watchdog, type为PTYPE_TEXT, sub_type为SUBTYPE_PLAIN_TEXT
static void
_report(struct gate *g, const char * data, ...) {
    if (g->watchdog == 0) {
        return;
    }

    struct skynet_context * ctx = g->ctx;

    va_list ap;
    va_start(ap, data);
    char tmp[1024];
    int n = vsnprintf(tmp, sizeof(tmp), data, ap);
    va_end(ap);

    uint32_t size = sizeof(uint32_t) + n;
    char str[1024];
    uint32_t sub_type = htonl(SUBTYPE_PLAIN_TEXT);
    memcpy(str, &sub_type, sizeof(uint32_t));
    memcpy(str+sizeof(uint32_t), tmp, n);

    skynet_send(ctx, 0, g->watchdog, PTYPE_TEXT, 0, str, size);
}

static void
_ctrl(struct gate *g, const void *msg, int sz, uint32_t source) {
    struct skynet_context *ctx = g->ctx;
    char tmp[sz+1];
    memcpy(tmp, msg, sz);
    tmp[sz] = '\0';
    char * command = tmp;
    int i;
    if (sz == 0) {
        return;
    }
    for (i = 0; i < sz; ++i) {
        if (command[i] == ' ') {
            break;
        }
    }
    if (memcmp(command, "kick", i) == 0) { // 命令格式："kick 1000", 其中1000表示uid
        _parm(tmp, sz, i);
        int uid = strtol(command, NULL, 10);
        int id = hashid_lookup(&g->hash, uid);
        if (id >= 0) {
            skynet_socket_close(ctx, uid); // 这里传入的是uid????--comment by dengkai
        }
        return;
    }
    if (memcmp(command, "forward", i) == 0) {
        // 指令格式：("forward %d :%x :%d", a, b, c), 其中a表示连接的套接字(不知道是不是系统原生的套接字!!!)--comment by dengkai
        _parm(tmp, sz, i);
        char * client = tmp;
        char * idstr = strsep(&client, " ");

        if (client == NULL) {
            return;
        }
        int id = strtol(idstr, NULL, 10);
        char * agent = strsep(&client, " ");
        if (client == NULL) {
            return;
        }
        uint32_t agent_handle = strtoul(agent+1, NULL 16);
        uint32_t client_handle = strtoul(client+1, NULL, 16);
        _forward_agent(g, id, agent_handle, client_handle);
        return;
    }
    if (memcmp(command, "broker", i) == 0) {
        // 代码里暂时没有用到,因此先不实现--comment by dengkai
        // TODO
    }
    if (memcmp(command, "start", i) == 0) {
        // 指令格式：("start %d", a) 其中a表示连接的套接字(不知道是不是系统原生的套接字!!!)--comment by dengkai
        _parm(tmp, sz, i);
        int uid = strtol(command, NULL, 10);
        int id = hashid_lookup(&g->hash, uid);
        if (id >= 0) {
            skynet_socket_start(ctx, id);
        }
        return;
    }
    if (memcmp(command, "close", i) == 0) {
        if (g->listen_id >= 0) {
            skynet_socket_close(ctx, g->listen_id);
            g->listen_id = -1;
        }
        return;
    }
    // 添加的新逻辑connect ip port
    if (memcmp(command, "connect", i) == 0) {
        // 代码里暂时没有用到,因此先不实现--comment by dengkai
        // TODO
    }
    if (memcmp(command, "listen", i) == 0) {
        start_listen(g, g->bind);
        return;
    }

    skynet_error(ctx, "[gate] Unknow command : %s", command);
}

static void
dispatch_socket_message(struct gate *g, const struct skynet_socket_message * msg, int sz) {
    struct skynet_context *ctx = g->ctx;

    switch (msg->type) {
    case SKYNET_SOCKET_TYPE_DATA:
    {
        int id = hashid_lookup(&g->hash, msg->id);
        if (id >= 0) {
            struct connection *c = &g->conn[id];
            dispatch_message(g, c, msg->id, msg->buffer, msg->ud);
        } else {
            skynet_socket_close(ctx, msg->id);
            skynet_free(msg->buffer);
        }
        break;
    }
    case SKYNET_SOCKET_TYPE_CONNECT: // 主动套接字发起connect，且connect连接成功
    {
        //TODO --comment by dengkai
    }
    case SKYNET_SOCKET_TYPE_CLOSE:
    case SKYNET_SOCKET_TYPE_ERROR:
    {
        int id = hashid_remove(&g->hash, message->id);
        if (id >= 0) {
            struct connection *c = &g->conn[id];
            uint32_t agent = c->agent;
            skynet_error(ctx, "socket close: %d, msg id:%d", c->id, message->id);
            databuffer_clear(&c->buffer,&g->mp);
            memset(c, 0, sizeof(*c));
            c->id = -1;

            _report(g, "gate close %d %u", message->id, agent);
        }
        break;
    }
    case SKYNET_SOCKET_TYPE_ACCEPT: // 被动套接字accept成功
    {
        assert(g->listen_id == msg->id);
        if (hashid_full(&g->hash)) {
            skynet_socket_close(ctx, msg->ud);
        } else {
            struct connection *c = &g->conn[hashid_insert(&g->hash, msg->ud)];
            if (sz >= sizeof(c->remote_name)) {
                sz = sizeof(c->remote_name) - 1;
            }
            c->id = msg->id;
            memcpy(c->remote_name, msg + 1, sz);
            c->remote_name[sz] = '\0';
            _report(g, "gate accept %d %s", c->id, c->remote_name);
            skynet_error(ctx, "socket open: %d", c->id);
        }
        break;
    }
    case SKYNET_SOCKET_TYPE_WARNING:
        skynet_error(ctx, "fd (%d) send buffer (%d)K", msg->id, msg->ud);
        break;
    }
}

static int
_cb(struct skynet_context * ctx, void * ud,, int type, int session, uint32_t source, const void * msg, size_t sz) {
    struct gate *g = ud;
    switch (type) {
    case PTYPE_TEXT:
        _ctrl(g, msg, (int)sz, source);
        break;
    case PTYPE_CLIENT:
        // gate会收到这个类型的消息吗???--comment by dengkai
        // TODO
    case PTYPE_SOCKET:
        // gate首先接受socket消息
        dispatch_socket_message(g, msg, (int)(sz - sizeof(struct skynet_socket_message)));
        break;
    }
    return 0;
}


struct gate *
gate_create(void) {
    struct gate *g = skynet_malloc(sizeof(struct gate));
    memset(g, 0, sizeof(struct gate));
    g->listen_id = -1;
    return g;
}

void
gate_release(struct gate *g) {
    int i;
    struct skynet_context *ctx = g->ctx;
    for (i = 0; i < g->max_connection; ++i) {
        struct connection *c = &g->conn[i];
        if (c->id >= 0) {
            skynet_socket_close(ctx, c->id);
        }
    }
    if (g->listen_id >= 0) {
        skynet_socket_close(ctx, g->listen_id);
    }
    messagepool_free(&g->mp);
    hashid_clear(&g->hash);
    skynet_free(g->conn);
    skynet_free(g);
}

int
gate_init(struct gate *g, struct skynet_context *ctx, char *parm) {
    if (parm == NULL) {
        return 1;
    }

    int sz = strlen(parm) + 1;
    unsigned watchdog;
    char binding[sz]; // 保存ip:port--comment by dengkai
    sscanf(parm, "%s %u", binding, &watchdog);
    g->watchdog = watchdog;
    g->ctx = ctx;

    int max = atoi(skynet_getenv("max_online"));
    hashid_init(&g->hash, max);
    g->conn = skynet_malloc(max * sizeof(struct connection));
    memset(g->conn, 0, max * sizeof(struct connection));
    g->max_connection = max;
    int i;
    for (i = 0; i < max; ++i) {
        g->conn[i].id = -1;
    }

    g->client_tag = PTYPE_CLIENT;
    g->header_size = 4;

    skynet_callback(ctx, g, _cb);

    if (binding[0] == "!") { // 为什么要这样判断????--comment by dengkai
        return 0;
    }

    memcpy(g->bind, binding, sz);

    //return start_listen(g,binding);
    return 0;
}

