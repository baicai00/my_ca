#include <stdint.h>
#include "Agent.h"

extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
}

extern "C"
{
    static int agent_cb(struct skynet_context* ctx, void* ud, int type, int session, uint32_t source, const void* msg, size_t sz)
    {
    }

    Agent* agent_create()
    {
        Agent* agent = new Agent();
        return agent;
    }

    void agent_release(Agent* agent)
    {
        delete agent;
    }

    int agent_init(Agent* agent, struct skynet_context* ctx, char* parm)
    {
    }
}
