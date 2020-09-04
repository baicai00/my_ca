#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "Pack.h"
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>


typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;


class Callback : boost::noncopyable
{
public:
    virtual ~Callback() {}
    virtual void onMessage(const MessagePtr& message) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class CallbackT : public Callback
{
#ifndef NDEBUG
    BOOST_STATIC_ASSERT((boost::is_base_of<google::protobuf::Message, T>::value));
#endif

public:
    typedef boost::function<void(const boost::shared_ptr<T>& message)> ProtobufMessageTCallback;

    CallbackT(const ProtobufMessageTCallback& callback)
        : callback_(callback)
    {
    }

    virtual void onMessage(const MessagePtr& message) const
    {
        boost::shared_ptr<T> concrete = down_pointer_cast<T>(message);
        assert(concrete != NULL);
        callback_(concrete);
    }

private:
    ProtobufMessageTCallback callback_;
};


//////////////////////////////////////////////////////////////////////////////////////
class ProtobufDispatcher
{
public:
    typedef boost::function<void(const MessagePtr& message)> ProtobufMessageCallback;

    explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCb)
        : defaultCallback_(defaultCb)
    {
    }

    void onProtobufMessage(const MessagePtr& message) const
    {
        CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
        if (it != callbacks_.end())
        {
            it->second->onMessage(message);
        }
        else
        {
            defaultCallback_(message);
        }
    }

    template<typename T>
    void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback)
    {
        boost::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
        callbacks_[T::descriptor()] = pd;
    }

private:
    typedef std::map<const google::protobuf::Descriptor*, boost::shared_ptr<Callback> > CallbackMap;

    CallbackMap callbacks_;
    ProtobufMessageCallback defaultCallback_;
};


//////////////////////////////////////////////////////////////////////////////////////
enum DispatcherStatus
{
    DISPATCHER_SUCCUSS = 0,
    DISPATCHER_PACK_ERROR = 1,
    DISPATCHER_PB_ERROR = 2,
    DISPATCHER_CALLBACK_ERROR = 3,
};


template<typename T>
class DispatcherT
{
public:
    typedef boost::function<void(Message*, T)> Callback;
    typedef std::map<std::string, Callback> CallbackMap;

    void register_callback(const string& name, const Callback& func)
    {
        m_callback[name] = func;
    }

    DispatcherStatus dispatch_message(const char* data, uint32_t size, T user)
    {
        return dispatch_(data, size, user, true);
    }

    DispatcherStatus dispatch_client_message(const char* data, uint32_t size, T user)
    {
        return dispatch_(data, size, user, false);
    }

    static void onMessage(const MessagePtr& message)
    {
    }

public:
    CallbackMap m_callback;

private:
    DispatcherStatus dispatch_(const char* data, uint32_t size, T user, bool inner)
    {
        InPack pack;
        bool b = false;
        if (inner)
        {
            b = pack.inner_reset(data, size);
        }
        else
        {
            b = pack.reset(data, size);
        }
        if(!b)
        {
            return DISPATCHER_PACK_ERROR;
        }

        typename CallbackMap::iterator it = m_callback.find(pack.m_type_name);
        if (it != m_callback.end())
        {
            Message* msg = pack.create_message();
            if (!msg)
            {
                return DISPATCHER_PB_ERROR;
            }
            it->second(msg, user);
            delete msg;
            return DISPATCHER_SUCCUSS;
        }
        return DISPATCHER_CALLBACK_ERROR;
    }
};


#endif
