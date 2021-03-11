#include "ValueService.h"


ValueService::ValueService()
{
    msg_service_register_init_func(boost::bind(&ValueService::value_init, this, _1));
}

ValueService::~ValueService()
{
}

bool ValueService::value_init(const string& parm)
{
    service_name("value", true);
    set_msg_service_type(SERVICE_VALUE);

    auto ret = init_value_mgr();
    if (!ret)
    {
        return false;
    }

    register_callback();
    send_callback();

    return true;
}

void ValueService::register_callback()
{
    service_callback(pb::iChangeMultiValue::descriptor()->full_name(),
        [this](Message* data, ValueUser* user)
    {
        // todo
    });

    rpc_register(pb::iChangeMultiValue::descriptor()->full_name(),
        [this](Message* data)
    {
        // todo
    });

    service_callback(pb::iChangeValue::descriptor()->full_name(),
        [this](Message* data, uint32_t source)
    {
        // todo
    });

    rpc_register(pb::iChangeValue::descriptor()->full_name(),
        [this](Message* data)
    {
        // todo
    });
}

void ValueService::text_message(const void* msg, size_t sz, uint32_t source, int session)
{
    char* c = (char*)msg;
    const char* from = strsep(&c, " ");

    if (strcmp(from, "php") == 0)
    {
    }
    else if (strcmp(from, "lua") == 0)
    {
        const char* uid = strsep(&c, " ");
        TextParm parm(c);
        const char* cmd = parm.get("cmd");
        string response;
        if (strcmp(cmd, "changevalueall") == 0)
        {
        }
        else if (strcmp(cmd, "changevalue") == 0)
        {
        }
        else if (strcmp(cmd, "getvalue") == 0)
        {
            ValueType value_type = parm.get_int("value_type");
            auto ret = lua_get_value(value_type, parm.get_int64("uid"));
            if (get<0>(ret) == CHANGE_OK)
            {
            }
            else if (get<0>(ret) == NO_VALUE)
            {
            }
            else
            {
            }
        }

        skynet_send(m_ctx, 0, source, PTYPE_RESPONSE, session, (void*)response.c_str(), response.size());
    }
}

bool ValueService::init_value_mgr()
{
    {
        auto p = new ValueMgr<USER_KKCOIN>;
        p->value_mgr_init(this);
        m_value_mgr[USER_KKCOIN].reset(p);
    }

    {
        auto p = new ValueMgr<USER_DIAMOND>;
        p->value_mgr_init(this);
        m_value_mgr[USER_DIAMOND].reset(p);
    }

    return true;
}

std::shared_ptr<BaseValueMgr> ValueService::get_value_mgr(ValueType value_type)
{
    auto it = m_value_mgr.find(value_type);
    if (it == m_value_mgr.end())
    {
        return nullptr;
    }

    return it->second;
}

tuple<ChangeValueType, int64_t> ValueService::lua_get_value(ValueType value_type, int64_t uid)
{
    auto mgr = get_value_mgr(value_type);
    if (mgr == nullptr)
    {
        return make_tuple(FAILED_VALUE, 0);
    }
    return mgr->get_value(uid);
}

tuple<ChangeValueType, int64_t> ValueService::lua_change_value(ValueType value_type, int64_t uid, int64_t change, const string& type, const string& attach)
{
    auto mgr = get_value_mgr(value_type);
    if (mgr == nullptr)
    {
        return make_tuple(FAILED_VALUE, 0);
    }

    string temp = type;
    if (temp.empty())
    {
        temp = "lua";
    }
    return mgr->change_value(uid, change, temp, attach);
}

