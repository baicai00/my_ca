#include "Main.h"


Main::Main()
{
    service_register_init_func(boost::bind(&Main::main_init, this, _1));
}

bool Main::main_init(const string& parm)
{
    service_name("main", true);

    register_callback();
}

void Main::register_callback()
{

}

Message* Main::rdkey_check(Message* data)
{

}

void Main::proto_register(Message* data, uint32_t handle)
{
    service_callback(pb::iRegisterName::descriptor()->full_name(), boost::bind(&Main::proto_name_register, this, _1, _2));
}

void Main::proto_lua_register(Message* data, uint32_t handle)
{

}

void Main::proto_watchdog_register(Message* data, uint32_t handle)
{

}

void Main::proto_name_register(Message* data, uint32_t handle)
{
    pb::iRegisterName* msg = dynamic_cast<pb::iRegisterName*>(data);

    if (!msg->name().empty())
    {
        // 删除
        for (auto it = m_name_list.begin(); it != m_name_list.end(); ++it)
        {
            if (it->second.name == msg->name())
            {
                m_name_list.earse(it);
                break;
            }
        }

        NameInfo& info = m_name_list[msg->handle()];
        info.name = msg->name();
        info.ok = msg->ok();
        if (info.ok)
        {
            // 将新服务的名字与handle广播到其他服务
            pb::iNameList rsp;
            pb::ServiceName* item = rsp.add_list();
            item->set_handle(msg->handle());
            item->set_name(msg->name());
            name_service_broadcast(rsp);
        }
    }

    // 获取所有其他服务,返回名字列表给新服务
    pb::iNameList rsp;
    for (auto it = m_name_list.begin(); it != m_name_list.end(); ++it)
    {
        if (it->second.ok)
        {
            pb::ServiceName* item = rsp.add_list();
            item->set_handle(it->first);
            item->set_name(it->second.name);
        }
    }
    service_send(rsp, handle);
}

void Main::proto_inner_login(Message* data, uint32_t handle)
{

}

void Main::name_service_broadcast(const Message& msg)
{
    for (auto it = m_name_list.begin(); it != m_name_list.end(); ++it)
    {
        service_send(msg, it->first);
    }
}

