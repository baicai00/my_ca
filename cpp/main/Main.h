#ifndef __MAIN_H__
#define __MAIN_H__

#include <map>
#include <string>
#include <set>
#include <stdint.h>
#include <glog/logging.h>
#include "Service.h"
#include "common.h"

using namespace std;


struct ServiceInfo
{
    ServiceType type;
    uint32_t    handle;
    set<string> proto;
};

struct NameInfo
{
    string  name;   // 服务的名字
    bool    ok;     // 为true表示该服务注册时会将自己的名字与handle广播给其他服务,当有新服务注册时该服务也会收到新服务的名字与handle
};

class Main : public Service
{
public:
    Main();
    bool main_init(const string& parm);

    void register_callback();

    Message* rdkey_check(Message* data);

    void proto_register(Message* data, uint32_t handle);

    void proto_lua_register(Message* data, uint32_t handle);

    void proto_watchdog_register(Message* data, uint32_t handle);

    void proto_name_register(Message* data, uint32_t handle);

    void proto_inner_login(Message* data, uint32_t handle);


    void name_service_broadcast(const Message& msg);

private:
    map<ServiceType, ServiceInfo>   m_service;
    map<uint32_t, NameInfo>         m_name_list; // 保存服务的名字信息 key=>服务的handle
};

#endif