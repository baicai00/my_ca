#ifndef __RPC_H__
#define __RPC_H__

#include <map>
#include <stdint.h>
#include <boost/function.hpp>
#include "Dispatcher.h"


typedef boost::function<void(Message* msg)> RPCCallBack;

typedef boost::function<void(Message* msg)> RSPFunction;


class skynet_context;
class RpcClient
{
public:
    RpcClient();
    void rpc_init_client(skynet_context* ctx);
    int rpc_call(int32_t dest, const google::protobuf::Message& msg, const RPCCallBack& func, int64_t uid);
    void rpc_cancel(int& id);
    void rpc_event_client(const char* data, uint32_t size, uint32_t source, int session);

private:
    skynet_context* m_rpc_ctx;
    std::map<int, RPCCallBack> m_rpc;
};


//////////////////////////////////////////////////////////////////////////////////////
class RpcServer
{
public:
    typedef boost::function<Message*(Message*)> Callback;
    typedef std::map<std::string, Callback> CallbackMap;

    RpcServer();
    void rpc_init_server(skynet_context* ctx);
    void rpc_register(const std::string& name, const Callback& func)
    {
        
    }
    DispatcherStatus rpc_event_server(const char* data, uint32_t size, uint32_t source, int session);

    RSPFunction get_response();

public:
    CallbackMap m_callback;

private:
    void response(Message* msg, uint32_t dest, int session, int64_t uid);

    skynet_context* m_rpc_ctx;
    uint32_t m_source;
    int m_session;
    int64_t m_uid;
};


#endif