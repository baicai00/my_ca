#ifndef __MSG_SERVICE_H__
#define __MSG_SERVICE_H__

#include "Service.h"


//能处理客户端消息的service,需要set_msg_service_name
template<typename User>
class MsgService : public Service
{
public:
    MsgService()
    {
    }

    virtual ~MsgService() {}

    bool msg_service_init(const std::string& parm);

    void set_msg_service_type(ServiceType type);

    void msg_callback(const std::string& name, const typename DispatcherT<User*>::Callback& func);

    User* get_user(int64_t uid);

    void reinit_agent(uint32_t handle, int64_t uid);

    void send_callback();
    void send_callback(uint32_t msg_service);

    virtual void message(const char* data, int32_t size, uint32_t agent);

    void __message(const char* data, int32_t size, uint32_t agent);

    void msg_broadcast(Message& msg, User* user = NULL);

    void msg_user_release(Message* data, uint32_t handle);

    void agent_release(Message* data, uint32_t handle);
    void agent_disconnect(Message* data, uint32_t handle);
    void agent_rebind(Message* data, uint32_t handle);
    void agent_init(Message* data, uint32_t handle);

    virtual void agent_release_event(User* user);
    //生成user，绑定handle domain到user
    void agent_init_event(uint32_t agent, int64_t uid, int domain);
    virtual void agent_disconnect_event(User* user) {}
    virtual void agent_rebind_event(User* user) {}

protected:
    DispatcherT<User*> m_msg_dsp;

private:
    RegisterInitFunc m_init_func;
};


#endif
