#ifndef __VALUE_MGR_H__
#define __VALUE_MGR_H__

#include "BaseValue.h"
#include "ValueUser.h"
#include <tuple>


//interface class
class BaseValueMgr
{
public:
    virtual ~BaseValueMgr(){}

    virtual tuple<ChangeValueType, int64_t> operate_value(const pb::iChangeValue* msg) = 0;
    virtual tuple<ChangeValueType, int64_t> get_value(int64_t uid) = 0;
    virtual tuple<ChangeValueType, int64_t> delete_value(int64_t uid, const string& type, const string& attach) = 0;
    virtual tuple<ChangeValueType, int64_t> change_value(int64_t uid, int64_t change, const string& type, const string& attach) = 0;
    virtual tuple<ChangeValueType, int64_t> test_operate_value(const pb::iChangeValue* msg) = 0;

    virtual void clear() = 0;
};


template<ValueType L>
class ValueMgr : public BaseValueMgr
{
public:
    using ValueKey = typename ValueKeyTraits<L>::ValueKeyType;
    using Value = BaseValue<ValueKey>;

    ValueMgr()
    {
    }
    void value_mgr_init(MsgService<ValueUser>* service)
    {
    }
    void pending_msg(const ValueKey& k)
    {
    }
    void clean_pending(const ValueKey& k)
    {
    }
    void load_value_from_db(const ValueKey& k)
    {
        pb::iGetDbValue req;
        req.set_type(L);
        m_service->rpc_call(m_service->service_handle("persistence"), req,
            [this, k](Message* data)
        {
            pb::iDbValue* rsp = dynamic_cast<pb::iDbValue*>(data);
            Value v;
            v.key = k;
            v.value = rsp->value();
            if (rsp->code() == 0)
            {
                v.set_init();
            }
            add_new_value(k, v);
        });
    }
    void add_new_value(const ValueKey& k, const Value& v)
    {
        auto it = m_values.lower_bound(k);
        if (it != m_values.end() && it->first == k)
        {
            if (it->second.is_init())
            {
                LOG(WARNING) << "add new value. bug has been exist. " << k;
            }
            else //没init过的，重新加载
            {
                it->second = v;
            }
        }
        else
        {
            m_values.insert(it, make_pair(k, v));
        }

        // 这段代码是干嘛的???--add by dengkai
        m_pending_msg.process_pending(k);
    }
    void response2user(int64_t uid, Value* value)
    {
    }

    virtual void clear()
    {
    }

    virtual tuple<ChangeValueType, int64_t> operate_value(const pb::iChangeValue* msg) override 
    {
    }

    virtual tuple<ChangeValueType, int64_t> get_value(int64_t uid) override
    {
        return _get_value(get_value_key<L>(uid));
    }

    virtual tuple<ChangeValueType, int64_t> delete_value(int64_t uid, const string& type, const string& attach)
    {
    }

    virtual tuple<ChangeValueType, int64_t> change_value(int64_t uid, int64_t change, const string& type, const string& attach) override
    {
        return _change_value(get_value_key<L>(uid), change, type, attach);
    }

    virtual tuple<ChangeValueType, int64_t> test_operate_value(const pb::iChangeValue* msg) override
    {
    }

private:
    Value* get_value(const ValueKey& k)
    {
        auto it = m_values.find(k);
        if (it == m_values.end())
        {
            load_value_from_db(k);
            pending_msg(k);
            return NULL;
        }

        auto& v = it->second;
        v.inc_query();

        //没init完成的value，每请求3次，再load一遍
        if(!v.is_init())
        {
            if (v.get_query_times() % 3 == 0)
            {
                load_value_from_db(k);
            }
        }

        return &v;
    }

    tuple<ChangeValueType, int64_t> _test_operate_value(const ValueKey& k, int64_t change, ValueOP op, string type = "")
    {
    }

    tuple<ChangeValueType, int64_t> _get_value(const ValueKey& k)
    {
        KeyUnStandard ks;
        ks.init(k, L);
        auto uid = ks.get_uid();

        if (is_robot(uid))
        {
            return make_tuple(CHANGE_OK, 90000000);
        }

        Value* value = get_value(k);
        if (value == NULL)
        {
            return make_tuple(NO_VALUE, 0);
        }
        return make_tuple(CHANGE_OK, value->value);
    }

    tuple<ChangeValueType, int64_t> _delete_value(const ValueKey& k, const string& type, const string& attach)
    {
    }

    tuple<ChangeValueType, int64_t> _change_value(const ValueKey& k, int64_t change, const string& type, const string& attach)
    {
        KeyUnStandard ks;
        ks.init(k, L);

        auto uid = ks.get_uid();

        //机器人id直接返回成功
        if (is_robot(uid))
        {
            return make_tuple(CHANGE_OK, 90000000);
        }

        Value* value = get_value(k);
        if (value == NULL)
        {
            return make_tuple(NO_VALUE, 0);
        }

        //没有init完成，不能修改
        if (!value->is_init())
        {
            return make_tuple(CHANGE_FAILED, value->value);
        }

        if (value->value + change < 0)
        {
            return make_tuple(CHANGE_FAILED, value->value);
        }

        value->value += change;

        pb::iSetDbValue req;
        req.set_value_type(L);
        k.to_pb(req.mutable_key());
        req.set_value(value->value);
        req.set_type(type);
        req.set_attach(attach);
        req.set_change(change);
        m_service->service_send(req, m_service->service_handle("persistence"), uid);
        response2user(uid, value);

        return make_tuple(CHANGE_OK, value->value);
    }

    tuple<ChangeValueType, int64_t> _set_cache(const ValueKey& k, int64_t change, const string& type, const string& attach)
    {
    }

protected:
    MsgService<ValueUser>* m_service;

    map<ValueKey, Value> m_values;
    map<int, set<ValueKey>> m_harbor_index;

    PendingProcessor<ValueKey> m_pending_msg;
};


#endif
