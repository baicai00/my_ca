/*
 * comment by dengkai 20201003 11:58
 * agent服务表示一条客户端连接,在watchdog进程中通过new_agent函数创建
*/

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
    void filter_init();
    void agent_message(char* data, uint32_t size);

    ServiceType agent_message_filter(InPack& pack);

    void agent_send(const Message& msg); //send to client
    void agent_send(void* msg, uint32_t sz); //send to client
    void agent_isend(const Message& msg, uint32_t dest); //send to service
    void service_broadcast(const Message& msg);
    //客户端的包发到内部，转化为内部包
    void client_pack_send(const char* data, uint32_t size, uint32_t dest);

    void agent_disconnect_self();
    void agent_kill_self();
    void agent_on_disconnect();
    void agent_on_rebind(int fd, const std::string& ip, uint32_t port);

    ServiceType filter_hearbeat(Message* msg);
    ServiceType filter_user_login(Message* data);

private:
    int                 m_agent_fd;
    int64_t             m_agent_uid;
    uint32_t            m_agent_gate;
    uint32_t            m_agent_dog;
    skynet_context*     m_agent_ctx;
    string              m_agent_ip;
    uint32_t            m_agent_port;
    uint32_t            m_agent_room; // 应该保存的是房间的handle--comment by dengkai

    int                 m_roomid;

    bool                m_agent_alive;

    map<string, AgentFilterFunc> m_filters;
};

#endif