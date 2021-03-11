#include "Watchdog.h"


Watchdog* g_watchdog;

Watchdog::Watchdog()
    : dispatcher_(boost::bind(&Watchdog::onUnknownMessageType, this, _1))
{
    m_is_listen = false;
}

void Watchdog::onUnknownMessageType(const MessagePtr& message)
{
    LOG(ERROR) << "onUnknownMessageType: " << message->GetTypeName();
}

bool Watchdog::dog_init(const std::string& parm)
{
    g_watchdog = this;

    m_domain = atoi(skynet_getenv("domain"));
    dog_register_callback();

    return true;
}

void Watchdog::dog_register_callback()
{
    service_callback(pb::iServicesRegister::descriptor()->full_name(), boost::bind(&Watchdog::register_proto, this, _1, _2));
    service_callback(pb::iLuaServicesRegister::descriptor()->full_name(), boost::bind(&Watchdog::register_lua_proto, this, _1, _2));

    m_dog_dsp.register_callback(pb::UserLoginREQ::descriptor()->full_name(), boost::bind(&Watchdog::msg_user_login, this, _1, _2));
}

uint32_t Watchdog::new_agent(int fd, int64_t uid)
{
    if (m_agent_fd.find(fd) == m_agent_fd.end())
    {
        LOG(ERROR) << "new agent. but fd not exist. fd:" << fd << " uid:" << uid;
        return 0;
    }

    AgentFd* af = &m_agent_fd[fd];
    char sendline[256];
    // 把fd, gate, watchdog ctx, uid, ip, port来创建agent服务
    sprintf(sendline, "%d %u %u %ld %s", fd, m_gate, skynet_context_handle(m_ctx), uid, af->m_ip_port.c_str());
    struct skynet_context* agent_ctx = skynet_context_new("agent", sendline);
    if (agent_ctx == NULL)
    {
        LOG(INFO) << "agent_ctx == NULL";
        return 0;
    }

    uint32_t handle = skynet_context_handle(agent_ctx);
    AgentHandle& ah = m_agent_handle[handle];
    ah.m_handle = handle;
    ah.m_fd = af;
    ah.m_uid = uid;

    af->m_handle = &ah;
    m_agent_uid[uid] = &ah;

    return handle;
}

uint32_t Watchdog::rebind_agent(int fd, AgentHandle* ah)
{
}

void Watchdog::destroy_agent(uint32_t handle)
{
}

void Watchdog::destroy_fd_agent(int fd)
{
}

void Watchdog::destroy_fd_agent(uint32_t handle)
{
}

void Watchdog::register_proto(Message*data, uint32_t source)
{
    pb::iServicesRegister *msg = dynamic_cast<pb::iServicesRegister*>(data);
    for (int i = 0; i < msg->item_size(); ++i)
    {
        const pb::iProtoRegister& s = msg->item(i);
        ServiceType type = (ServiceType)s.service();
        m_service[type] = s.handle();
        for (int j = 0; j < s.proto_size(); ++j)
        {
            m_agent_router[s.proto(j)] = type;
            LOG(INFO) << "register_proto s.proto(j) = " << s.proto(j) << " type = " << type;
        }
    }
    LOG(INFO) << "watchdog get route table. route type size:" << m_agent_router.size();
}

void Watchdog::register_lua_proto(Message*data, uint32_t source)
{
    pb::iLuaServicesRegister* msg = dynamic_cast<pb::iLuaServicesRegister*>(data);

    for (int i = 0; i < msg->item_size(); ++i)
    {
        const pb::iLuaProtoRegister& s = msg->item(i);
        ServiceType type = (ServiceType)s.service();
        m_service[type] = s.handle();   // 服务对应的handle--comment by dengkai
        for (int j = 0; j < s.proto_size(); ++j)
        {
            m_agent_router[s.proto(j)] = type;
        }
    }
    LOG(INFO) << "watchdog get route table. route type size:" << m_agent_router.size();
}

void Watchdog::new_connection(int fd, const std::string& name)
{
    AgentFd& af = m_agent_fd[fd];
    af.m_fd = fd;
    af.m_ip_port = name;
    af.m_handle = NULL;

    // let gate service to start request
    char sendline[100];
    sprintf(sendline, "start %d", fd);
    skynet_send(m_ctx, 0, m_gate, PTYPE_TEXT, 0, sendline, strlen(sendline));
}

void Watchdog::client_disconnect(int fd)
{
    std::map<int, AgentFd>::iterator it = m_agent_fd.find(fd);
    if (it == m_agent_fd.end())
    {
        LOG(ERROR) << "socket disconnect no find fd" << fd;
        return;
    }

    AgentHandle* handle = it->second.m_handle;
    if (handle != NULL)
    {
        handle->m_fd = NULL;
        handle->m_disconnect_timer = start_timer(10 * 60 * 100, boost::bind(&Watchdog::destroy_agent, this, handle->m_handle));

        char sendline[100];
        sprintf(sendline, "dog client_disconnect");
        size_t len = strlen(sendline);
        // todo--从这里开始
    }
}

void Watchdog::dog_disconnect(int fd)
{
    char sendline[100];
    sprintf(sendline, "kick %d", fd);
    skynet_send(m_ctx, 0, m_gate, PTYPE_TEXT, 0, sendline, strlen(sendline));
}

void Watchdog::dog_message(char* data, uint32_t size, int fd)
{
    DispatcherStatus status = m_dog_dsp.dispatch_client_message(data, size, fd);
    if (status != DISPATCHER_SUCCUSS)
    {
        destroy_fd_agent(fd);
        LOG(WARNING) << "watchdog destroy agent cause error msg. fd:" << fd;
    }
    if (status == DISPATCHER_CALLBACK_ERROR)
    {
        InPack pack;
        pack.reset(data, size);
        LOG(WARNING) << "dispatch error. pb name:" << pack.m_type_name;
    }
}

void Watchdog::watchdog_poll(const char* data, uint32_t size, uint32_t source, int session, int type)
{
    switch (type)
    {
    case PTYPE_TEXT:
    {
        uint32_t sub_type;
        memcpy(&sub_type, data, sizeof(uint32_t));
        sub_type = ntohl(sub_type);

        data += sizeof(uint32_t);
        size -= sizeof(uint32_t);

        if (sub_type == SUBTYPE_PROTOBUF)
        {
            //m_process_uid = get_uid_from_stream(data);
            //proto(data, size, source);
            //m_process_uid = 0;
        }
        else if (sub_type == SUBTYPE_RPC_SERVER)
        {
            //m_process_uid = get_uid_from_stream(data);
            //rpc_event_server(data, size, source, session);
            //m_process_uid = 0;
        }
        else if (sub_type == SUBTYPE_PLAIN_TEXT)
        {
            string stream(data, size);
            LOG(INFO) << "SUBTYPE_PLAIN_TEXT stream = " << stream;
            string from;
            tie(from, stream) = divide_string(stream, ' ');
            if (!stream.empty() && from == "php")
            {
                string uid;
                tie(uid, stream) = divide_string(stream, ' ');
                LOG(INFO) << "uid = " << uid;
                if (!stream.empty())
                {
                    m_process_uid = atoll(uid.c_str());
                    LOG(INFO) << "Watchdog::watchdog_poll SUBTYPE_PLAIN_TEXT m_process_uid = " << m_process_uid;
                }
            }

            text_message(data, size, source, session);
            m_process_uid = 0;
        }

        break;
    }
    case PTYPE_CLIENT:
    {
        char* c = (char*)data;
        // 这里的fd是skynet框架加进去的吗???什么时候加进去的??? add by dengkai
        // 解答：fd是在service_gate.c的_forward函数中添加的
        int fd = atoi(strsep(&c, " "));
        dog_message(c, size - (c - (char*)data), fd);
        break;
    }
    case PTYPE_RESPONSE:
    default:
        break;
    }
}

void Watchdog::text_message(const char* msg, size_t sz, uint32_t source, int session)
{
    char* c = (char*)msg;
    const char* from = strsep(&c, " ");
    if (strcmp(from, "gate") == 0)
    {
        const char* cmd = strsep(&c, " ");
        // 接受gate发过来的消息，接受accept, 创建新的连接，以后客户端的协议就会发到agent
        if (strcmp(cmd, "accept") == 0)
        {
            int fd = 0;
            char name[100];
            sscanf(c, "%d %s", &fd, name);
            LOG(INFO) << "Watchdog::text_message new_connection";
            new_connection(fd, name);
        }
        else if (strcmp(cmd, "close") == 0)
        {
            int fd = 0;
            sscanf(c, "%d", &fd);
            LOG(INFO) << "client_disconnect fd = " << fd;
            client_disconnect(fd);
        }
    }
    else if (strcmp(from, "php") == 0)
    {
        const char* uid = strsep(&c, " ");
        (void)uid;
        TextParm parm(c);
        const char* cmd = parm.get("cmd");
        string response = "no cmd";
        if (strcmp(cmd, "stopserver") == 0)
        {
            response = "stop server\n";
            php_stop_server(parm.get_int("type"));
        }
        else if (strcmp(cmd, "listen") == 0)
        {
            response = "listen ok\n";
            LOG(INFO) << "watchdog text_message php_listen";
            php_listen(); // 给gate发送listen, gate就会监听配置地址，并接受客户端的连接
        }

        skynet_send(m_ctx, 0, source, PTYPE_RESPONSE, session, (void*)response.c_str(), response.size());
    }
    else
    {
        LOG(INFO) << "unknow text command msg=" << msg;
    }
}

void Watchdog::dog_send(Message& msg, int fd)
{
    char* data;
    uint32_t size;
    serialize_msg(msg, data, size, 0);
    skynet_socket_send(m_ctx, fd, data, size);
}


void Watchdog::msg_user_login(Message* data, int fd)
{
    pb::UserLoginREQ* user_login = dynamic_cast<pb::UserLoginREQ*>(data);

    int64_t uid = user_login->uid();
    if (uid == 0)
    {
        pb::UserLoginRSP rsp;
        rsp.set_code(-1);
        rsp.set_reason("rdkey error");
        rsp.set_server_time(time(NULL));
        dog_send(rsp, fd);
        dog_disconnect(fd);
        return;
    }

    if (serverstop())
    {
        pb::UserLoginRSP rsp;
        rsp.set_code(-5);
        rsp.set_server_time(time(NULL));
        dog_send(rsp, fd);
        dog_disconnect(fd);
        return;
    }

    pb::iRdkeyCheckREQ req;
    req.set_uid(uid);
    req.set_rdkey(user_login->key());
    rpc_call(m_main, req, boost::bind(&Watchdog::rpcc_user_login, this, _1, uid, user_login->key(), fd), uid);
    return;
}

void Watchdog::rpcc_user_login(Message* data, int64_t uid, const std::string& rdkey, int fd)
{
    pb::iRdkeyCheckRSP* msg = dynamic_cast<pb::iRdkeyCheckRSP*>(data);

    if (m_agent_fd.find(fd) == m_agent_fd.end())
    {
        LOG(INFO) << "fd not exist. login failed. fd:" << fd << " uid:" << uid;
        return;
    }

    if (!msg->ok())
    {
        // 检查rdkey不通过--add by dengkai
        pb::UserLoginRSP rsp;
        rsp.set_code(-1);
        rsp.set_server_time(time(NULL));
        rsp.set_reason("rdkey error");
        dog_send(rsp, fd);
        dog_disconnect(fd);
        return;
    }

    uint32_t handle;
    std::map<int64_t, AgentHandle*>::iterator it = m_agent_uid.find(uid);
    if (it == m_agent_uid.end())
    {
        LOG(INFO) << "rpcc_user_login new_agent fd = " << fd << " uid =  " << uid;
        if ((handle = new_agent(fd, uid)) == 0)
        {
            pb::UserLoginRSP msg;
            msg.set_code(-2);
            msg.set_reason("server error");
            msg.set_server_time(time(NULL));
            dog_send(msg, fd);
            dog_disconnect(fd);
            LOG(ERROR) << "create agent error. uid:" << uid;
            return;
        }
    }
    else //user already in server
    {
        AgentHandle* ah = it->second;
        AgentFd* af = ah->m_fd;
        if (af != NULL) //disconnect old socket
        {
            pb::UserLogoutRSP rsp;
            rsp.set_code(2);
            dog_send(rsp, af->m_fd);
            af->m_handle = NULL;
            dog_disconnect(af->m_fd);
            LOG(INFO) << "kick old socket. uid:" << uid << " fd:" << af->m_fd;
        }
        if ((handle = rebind_agent(fd, ah)) == 0)
        {
            pb::UserLoginRSP msg;
            msg.set_code(-2);
            msg.set_reason("server error");
            msg.set_server_time(time(NULL));
            dog_send(msg, fd);
            dog_disconnect(fd);
            LOG(ERROR) << "rebind agent error. uid:" << uid;
            return;
        }
    }

    LOG(INFO) << "rpcc_user_login agent handle = " << handle;
    char sendline[100];
    sprintf(sendline, "forward %d :%x :0", fd, handle); // 这里是干嘛??? --add by dengkai
    skynet_send(m_ctx, 0, m_gate, PTYPE_TEXT, 0, sendline, strlen(sendline));

    pb::UserLoginRSP rsp;
    rsp.set_code(0);
    rsp.set_server_time(time(NULL));
    dog_send(rsp, fd);
}

ServiceType Watchdog::agent_route_to(const string& proto)
{

}

uint32_t Watchdog::agent_route_dest(ServiceType type)
{
    
}

void Watchdog::php_stop_server(int type)
{

}

void Watchdog::php_listen()
{

}
