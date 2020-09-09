#include "Watchdog.h"


Watchdog::Watchdog()
{
}

bool Watchdog::dog_init(const std::string& parm)
{
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
}

void Watchdog::watchdog_poll(const char* data, uint32_t size, uint32_t source, int session, int type)
{
    switch (type)
    {
    case PTYPE_TEXT:
    case PTYPE_CLIENT:
    case PTYPE_RESPONSE:
    default:
        break;
    }
}

void Watchdog::text_message(const char* msg, size_t sz, uint32_t source, int session)
{
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


