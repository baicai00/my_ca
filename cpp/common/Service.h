#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <map>
#include <stdint.h>
#include <boost/function.hpp>
#include "Pack.h"
#include "Dispatcher.h"
#include "RPC.h"
#include "Timer.h"


class skynet_context;

typedef boost::function<bool(const std::string&)> InitFunc;


/////////////////////////////////////////////////////////////////////////////////
class RegisterInitFunc
{
public:
    void register_init_func(InitFunc func)
    {
        if (m_func.empty())
        {
            m_func = func;
        }
    }
    bool call(const std::string& parm)
    {
        if (!m_func.empty())
        {
            return m_func(parm);
        }
    }

private:
    InitFunc m_func;
};


/////////////////////////////////////////////////////////////////////////////////
//Service子类需要做的事情：
//1、要调用service_register_init_func，注册自己的初始化函数
//2、service_name注册内部服务名称。
class Service : public Timer, public RpcClient, public RpcServer
{
public:
    Service();
    virtual ~Service();

    virtual void text_message(const void* msg, size_t sz, uint32_t source, int session) {}

    virtual void message(const char* data, int32_t size, uint32_t agent) {}

    // 得到name时回调
    virtual void name_got_event(const std::string& name) {}

    // 服务依赖完成
    virtual void service_start() {}

    // 停服事件
    virtual void service_stop_event() {}

public:
    bool service_init(skynet_context* ctx, const void* parm, int len);
    void service_name(const std::string& name, bool ok);
    void expose_service();

    void service_send(const Message& msg, uint32_t handle, int64_t uid);
    void service_send(const Message& msg, uint32_t handle);
    void service_fsend(const Message& msg, uint32_t handle, uint32_t source, int64_t uid);

    void service_lua_send(const Message& msg, uint32_t handle, int64_t uid);
    void service_lua_send(const Message& msg, uint32_t handle);
    void service_lua_fsend(const Message& msg, uint32_t handle, uint32_t source, int64_t uid);

    inline int rpc_call(int32_t dest, const google::protobuf::Message& msg, const RPCCallBack& func, int64_t uid)
    {
        return RpcClient::rpc_call(dest, msg, func, uid);
    }
    inline int rpc_call(int32_t dest, const google::protobuf::Message& msg, const RPCCallBack& func)
    {
        return RpcClient::rpc_call(dest, msg, func, m_process_uid);
    }

    void service_poll(const char* data, uint32_t size, uint32_t source, int session, int type);
    // 内部消息回调
    void proto(const char* data, uint32_t size, uint32_t source);
    void text_response(const std::string& rsp);
    void service_callback(const std::string& name, const DispatcherT<uint32_t>::Callback& func);

    uint32_t service_handle(const std::string& name);
    const std::string& service_name() { return m_service_name; }

    void service_depend_on(const std::string& name);
    void proto_service_name_list(Message* data, uint32_t handle);
    uint32_t new_skynet_service(const std::string& name, const string& param);

protected:
    DispatcherT<uint32_t> m_dsp;

private:
    std::map<std::string, uint32_t> m_named_service;
    std::string m_service_name;

    //RegisterInitFunc
    
};

#endif