#ifndef __WATCH_DOG_H__
#define __WATCH_DOG_H__

#include <string>
#include <map>
#include <stdint.h>
#include "AgentHandle.h"
#include "Service.h"


struct skynet_context;
class Watchdog : public Service
{
public:
    Watchdog();

    bool dog_init(const std::string& parm);

    // 创建agent
    uint32_t new_agent(int fd, int64_t uid);

    // 重新绑定agent
    uint32_t rebind_agent(int fd, AgentHandle* ah);

    // 销毁agent,根据handle
    void destroy_agent(uint32_t handle);
    // 销毁agent,根据fd
    void destroy_fd_agent(int fd);
    // 销毁agent 和fd 根据handle
    void destroy_fd_agent(uint32_t handle);

    void new_connection(int fd, const std::string& name);
    void client_disconnect(int fd);
    void dog_disconnect(int fd);

    void dog_message(char* data, uint32_t size, int fd);
    void watchdog_poll(const char* data, uint32_t size, uint32_t source, int session, int type);
    virtual void text_message(const char* msg, size_t sz, uint32_t source, int session) override;

    void dog_send(Message& msg, int fd);

    // 客户端登录
    void msg_user_login(Message* data, int fd);
    void rpcc_user_login(Message* data, int64_t uid, const std::string& rdkey, int fd);

    inline bool serverstop()
    {
        return m_server_stop == STOP_SERVER;
    }

private:
    uint32_t m_gate;

    std::map<int64_t, AgentHandle*> m_agent_uid; // key => uid
    std::map<int, AgentFd> m_agent_fd;  // key => fd
    std::map<uint32_t, AgentHandle> m_agent_handle; // key => 服务handle

    DispatcherT<int> m_dog_dsp;

    ServerStopType m_server_stop;
};


#endif