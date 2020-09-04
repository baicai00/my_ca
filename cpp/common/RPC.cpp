#include "RPC.h"

RpcClient::RpcClient()
{
}

void RpcClient::rpc_init_client(skynet_context* ctx)
{
}

int RpcClient::rpc_call(int32_t dest, const google::protobuf::Message& msg, const RPCCallBack& func, int64_t uid)
{
}

void RpcClient::rpc_cancel(int& id)
{
}

void RpcClient::rpc_event_client(const char* data, uint32_t size, uint32_t source, int session)
{
}


//////////////////////////////////////////////////////////////////////////////////////

RpcServer::RpcServer()
{
}

void RpcServer::rpc_init_server(skynet_context* ctx)
{
}

DispatcherStatus RpcServer::rpc_event_server(const char* data, uint32_t size, uint32_t source, int session)
{
}

RSPFunction RpcServer::get_response()
{
}

void RpcServer::response(Message* msg, uint32_t dest, int session, int64_t uid)
{
}



