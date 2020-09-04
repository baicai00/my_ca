#ifndef __AGENT_HANDLE_H__
#define __AGENT_HANDLE_H__

class AgentFd;
class AgentHandle
{
public:
    AgentHandle();

    uint32_t    m_handle;
    int64_t     m_uid;
    AgentFd*    m_fd;
    int         m_disconnect_timer;
};

class AgentFd
{
public:
    AgentFd();

    int             m_fd;   // socket fd
    std::string     m_ip_port;
    AgentHandle*    m_handle;
};


#endif