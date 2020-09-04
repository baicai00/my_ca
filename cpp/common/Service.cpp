#include "Service.h"


Service::Service()
{
}

Service::~Service()
{
}

bool Service::service_init(skynet_context* ctx, const void* parm, int len)
{
}

void Service::service_name(const std::string& name, bool ok)
{
}

void Service::expose_service()
{
}

void Service::service_send(const Message& msg, uint32_t handle, int64_t uid)
{
}

void Service::service_send(const Message& msg, uint32_t handle)
{
}

void Service::service_fsend(const Message& msg, uint32_t handle, uint32_t source, int64_t uid)
{
}

void Service::service_lua_send(const Message& msg, uint32_t handle, int64_t uid)
{
}

void Service::service_lua_send(const Message& msg, uint32_t handle)
{
}

void Service::service_lua_fsend(const Message& msg, uint32_t handle, uint32_t source, int64_t uid)
{
}

void Service::service_poll(const char* data, uint32_t size, uint32_t source, int session, int type)
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

void Service::proto(const char* data, uint32_t size, uint32_t source)
{
}

void Service::text_response(const std::string& rsp)
{
}

void Service::service_callback(const std::string& name, const DispatcherT<uint32_t>::Callback& func)
{
    m_dsp.register_callback(name, func);
}

uint32_t Service::service_handle(const std::string& name)
{
}

void Service::service_depend_on(const std::string& name)
{
}

void Service::proto_service_name_list(Message* data, uint32_t handle)
{
}

uint32_t Service::new_skynet_service(const std::string& name, const string& param)
{
}


