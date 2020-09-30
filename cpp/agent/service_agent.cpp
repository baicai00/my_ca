// agent.so  被watchdog进程启动

#include <glog/logging.h>
#include "Agent.h"

extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
}

using namespace std;

extern "C"
{
    static int agent_cb(struct skynet_context * ctx, void * ud, int type, int session, uint32_t source, const void * msg, size_t sz)
    {
        Agent* agent = (Agent*)ud;
        switch(type)
        {
        case PTYPE_TEXT:
        case PTYPE_CLIENT:
        default:
            break;
        }
        return 0;
    }

    Agent* agent_create()
    {
        Agent* agent = new Agent();
        return agent;
    }

    void agent_release()
    {
        delete agent;
    }

    // parm参数来自于watchdog,格式："fd gate watchdog ctx uid ip_port"
    int agent_init(Agent* agent, struct skynet_context* ctx, char* parm)
    {
        int fd;
        uint32_t gate;
        uint32_t dog;
        int64_t uid;
        char ip[64] = {0};
        uint32_t port;
        sscanf(parm, "%d %u %u %ld %[^:]%u", &fd, &gate, &dog, &uid, ip, &port);
        agent->agent_init(ctx, fd, gate, dog, uid, ip, port);
        skynet_callback(ctx, agent, agent_cb);
    }
}