#include "Agent.h"

extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
}

Agent::Agent()
{

}

Agent::~Agent()
{

}

bool Agent::agent_init(skynet_context* ctx, int fd, uint32_t gate, uint32_t dog, int64_t uid, const string& ip, uint32_t port)
{
    m_agent_ctx = ctx;
    m_agent_fd = fd;
    m_agent_gate = gate;
    m_agent_dog = dog;
    m_agent_uid = uid;
    m_agent_ip = ip;
    m_agent_port = port;
    m_agent_alive = true;

    filter_init();

    pb::iAgentInit req;
    req.set_domain()
}

void Agent::agent_message(char* data, uint32_t size)
{

}

void Agent::filter_init()
{

}