#ifndef __BASE_VALUE_H__
#define __BASE_VALUE_H__

#include <stdint.h>
#include "common.h"


enum ChangeValueType
{
    CHANGE_FAILED = -1,  //修改失败
    FAILED_VALUE = -2,  //失败
    CHANGE_OK = 0,  //修改成功
    NO_VALUE = 1,  //没有数据，消息pending并去load数据
};


///////////////////////////////////////////////////////////////////////////
template<int L>
class ValueKey
{
public:
    enum
    {
        KEY_LEN = L,
    };

    ValueKey()
    {
        for (auto i = 0; i < L; ++i)
        {
            key[i] = 0;
        }
    }

    bool operator==(const ValueKey& o) const
    {
        for (int i = 0; i < KEY_LEN; ++i)
        {
            if (key[i] != o.key[i])
            {
                return false;
            }
        }
        return true;
    }

    bool operator<(const ValueKey& o) const
    {
        for (int i = 0; i < KEY_LEN; ++i)
        {
            if (key[i] < o.key[i])
            {
                return true;
            }
            else if (key[i] > o.key[i])
            {
                return false;
            }
        }
        return false;
    }

    int64_t get_key(int idx) const
    {
    }

    void to_pb(pb::ValueKey* pb) const
    {
    }

    void from_pb(const pb::ValueKey* pb)
    {
    }

    int64_t key[L];
};


///////////////////////////////////////////////////////////////////////////
class KeyStandard
{
public:
    KeyStandard(ValueType type)
    {
        m_len = 0;
        m_type = type;
    }
    template<typename T>
    void to_value_key(T& t)
    {
        for (auto i = 0; i < T::KEY_LEN; ++i)
        {
            t.key[i] = m_key[i];
        }
    }
    void set_uid(int64_t uid)
    {
        switch (m_type)
        {
        case INVAILID:
            break;
        case USER_KKCOIN:
            m_key[0] = uid;
            break;
        case USER_DIAMOND:
            m_key[0] = uid;
            break;
        }
    }

private:
    int64_t m_key[100];
    int m_len;
    ValueType m_type;
};


class KeyUnStandard
{
public:
    KeyUnStandard()
    {
        m_len = 0;
        m_type = INVAILID;
        memset(m_key, 0, sizeof(m_key));
    }

    template<int L>
    void init(const ValueKey<L>& k, ValueType type)
    {
        init(k.key, ValueKey<L>::KEY_LEN, type);
    }

    void init(const int64_t* key, int len, ValueType type)
    {
        if (len >= 100)
        {
            return;
        }
        m_len = len;
        m_type = type;
        for (int i = 0; i < len; ++i)
        {
            m_key[i] = key[i];
        }
    }

    void init(const pb::ValueKey* pb, ValueType type)
    {
    }

public:
    int64_t get_uid()
    {
        int64_t uid = 0;
        switch(m_type)
        {
        case INVAILID:
            break;
        case USER_KKCOIN:
            uid = m_key[0];
            break;
        case USER_DIAMOND:
            uid = m_key[0];
            break;
        }

        return uid;
    }

private:
    int64_t m_key[100];
    int m_len;
    ValueType m_type;
};


///////////////////////////////////////////////////////////////////////////
template<typename K>
class BaseValue
{
public:
    BaseValue()
    {
        value = 0;
        m_query_times = 0;
        m_is_init = false;
    }

    void set_init() { m_is_init = true; }

    bool is_init() { return m_is_init; }

    void inc_query() { ++m_query_times; }

    uint32_t get_query_times() { return m_query_times; }

public:
    int64_t value;
    K       key;

private:
    uint32_t    m_query_times; // 请求次数
    bool        m_is_init;
};


///////////////////////////////////////////////////////////////////////////
typedef ValueKey<1> ValueKey1;


///////////////////////////////////////////////////////////////////////////
template<ValueType L>
class ValueKeyTraits
{
public:
    // 这里用到了using的别名指定的功能,相当于typedef void ValueMgrType;
    using ValueMgrType = void;
};

template<>
class ValueKeyTraits<USER_KKCOIN>
{
public:
    using ValueKeyType = ValueKey1;
};

template<>
class ValueKeyTraits<USER_DIAMOND>
{
public:
    using ValueKeyType = ValueKey1;
};

///////////////////////////////////////////////////////////////////////////
template<ValueType L>
typename ValueKeyTraits<L>::ValueKeyType get_value_key(int64_t uid)
{
    typename ValueKeyTraits<L>::ValueKeyType key;
    KeyStandard ks(L);
    ks.set_uid(uid);
    ks.to_value_key(key);

    return key;
}



#endif
