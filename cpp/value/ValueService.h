#ifndef __VALUE_SERVICE_H__
#define __VALUE_SERVICE_H__

#include "MsgService.h"
#include <stdint.h>
#include <memory>


class ValueService : public MsgService<ValueUser>
{
public:
    ValueService();
    virtual ~ValueService();

    bool value_init(const string& parm);
    void register_callback();

    virtual void text_message(const void* msg, size_t sz, uint32_t source, int session) override;

    bool init_value_mgr();
    std::shared_ptr<BaseValueMgr> get_value_mgr(ValueType value_type);

    tuple<ChangeValueType, int64_t> lua_get_value(ValueType value_type, int64_t uid);
    tuple<ChangeValueType, int64_t> lua_change_value(ValueType value_type, int64_t uid, int64_t change, const string& type, const string& attach);

private:
    std::map<ValueType, std::shared_ptr<BaseValueMgr>> m_value_mgr;
};

#endif