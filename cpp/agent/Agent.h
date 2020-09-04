#ifndef __AGENT_H__
#define __AGENT_H__

#include <stdint.h>
#include "Pack.h"
#include "Dispatcher.h"

using namespace std;

typedef boost::function<ServiceType(Message*)> AgentFilterFunc;

class skynet_context;
class Agent
{
public:
    Agent();
    ~Agent();

    bool agent_init(skynet_context* ctx, int fd, uint32_t gate, uint32_t dog, int64_t uid, const string& ip, uint32_t port);

private:
    int                 m_agent_fd;
    skynet_context*     m_agent_ctx;
    string              m_agent_ip;
    uint32_t            m_agent_port;

    bool                m_agent_alive;

    map<string, AgentFilterFunc> m_filters;
};

#endif