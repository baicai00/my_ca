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
    //req.set_domain(); // 这个成员好像没什么作用!!!--comment by dengkai
    req.set_uid(m_agent_uid);
    req.set_handle(skynet_context_handle(m_agent_ctx));
    req.set_fd(m_agent_fd);
    req.set_ip(m_agent_ip);
    req.set_port(m_agent_port);
    service_broadcast(req);

    pb::UserLocationRSP rsp;
    rsp.set_roomid(m_roomid);
    agent_send(rsp);
    return true;
}

void Agent::filter_init()
{
    m_filters[pb::HeartBeatREQ::descriptor()->full_name()] = boost::bind(&Agent::filter_hearbeat, this, _1);
    m_filters[pb::UserLoginREQ::descriptor()->full_name()] = boost::bind(&Agent::filter_user_login, this, _1);
}

void Agent::agent_message(char* data, uint32_t size)
{
    // data已经经过gate去掉了头4个byte的数据长度
    InPack pack;
    if (!pack.reset(data, size))
    {
        // 解析错误则断开连接
        LOG(ERROR) << "pack error";
        agent_disconnect_self();
        return;
    }

    ServiceType type = agent_message_filter(pack);
    if (type == SERVICE_NULL)
    {
        return;
    }

    if (type == SERVICE_NONE)
    {
        // 根据协议名称获取服务类型
        type = g_watchdog->agent_route_to(pack.m_type_name);
    }

    if (type == SERVICE_ROOM)
    {
        if (m_agent_room != 0)
        {
            // 房间类型的消息协议首先发送给房间
            client_pack_send(data, size, m_agent_room);
        }
        else
        {
            // 如果没有房间则放给RoomRouter
            // 根据服务类型获取服务的handle
            uint32_t dest = g_watchdog->agent_route_dest(SERVICE_ROUTER);
            client_pack_send(data, size, dest);
        }
    }
    else if (type == SERVICE_NONE)
    {
        LOG(WARNING) << "route error. prototype:" << pack.m_type_name << " uid:" << m_agent_uid;
    }
    else
    {
        // 其他的消息类型
        // 根据服务类型获取服务的handle
        uint32_t dest = g_watchdog->agent_route_dest(type);
        if (dest != 0)
        {
            client_pack_send(data, size, dest);
        }
        else
        {
            LOG(WARNING) << "route error prototype:" << pack.m_type_name << " servicetype:" << type;
        }
    }
}

ServiceType Agent::agent_message_filter(InPack& pack)
{
    //协议过滤
    map<string, AgentFilterFunc>::iterator it = m_filters.find(pack.m_type_name);
    if (it != m_filters.end())
    {
        Message* msg = pack.create_message();
        if (msg == NULL)
        {
            log_error("dispatch message pb error type:%s", pack.m_type_name.c_str());
            return SERVICE_NULL;
        }

        ServiceType type = it->second(msg);
        delete msg;
        return type;
    }
    return SERVICE_NONE;
}

void Agent::agent_send(const Message& msg)
{
    char* data;
    uint32_t size;
    serialize_msg(msg, data, size, 0);
    skynet_socket_send(m_agent_ctx, m_agent_fd, data, size);
}

void Agent::agent_send(void* msg, uint32_t sz)
{
    void* data = skynet_malloc(sz);
    memcpy(data, msg, sz);
    skynet_socket_send(m_agent_ctx, m_agent_fd, data, sz);
}

void Agent::agent_isend(const Message& msg, uint32_t dest)
{
    char* data;
    uint32_t size;
    serialize_imsg_type(msg, data, size, m_agent_uid, SUBTYPE_PROTOBUF);
    // skynet_send的具体用法是什么呢???--comment by dengkai
    skynet_send(m_agent_ctx, 0, dest, PTYPE_TEXT | PTYPE_TAG_DONTCOPY, 0, data, size);
}

void Agent::service_broadcast(const Message& msg)
{
    std::map<ServiceType, uint32_t>::iterator it = g_watchdog->m_service.begin();
    for (; it != g_watchdog->m_service.end(); ++it)
    {
        agent_isend(msg, it->second);
    }
    if (m_agent_room != 0)
    {
        agent_isend(msg, m_agent_room);
    }
}

void Agent::client_pack_send(const char* data, uint32_t size, uint32_t dest)
{

}

void Agent::agent_disconnect_self()
{
    char sendline[100];
    sprintf(sendline, "kill %d", m_agent_fd);
    skynet_send(m_agent_ctx, 0, m_agent_gate, PTYPE_TEXT, 0, sendline, strlen(sendline));
}

void Agent::agent_kill_self()
{

}

void Agent::agent_on_disconnect()
{

}

void Agent::agent_on_rebind(int fd, const std::string& ip, uint32_t port)
{

}

ServiceType Agent::filter_hearbeat(Message* msg)
{

}

ServiceType Agent::filter_user_login(Message* data)
{
    
}