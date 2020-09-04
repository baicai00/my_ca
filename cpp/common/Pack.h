#ifndef __PACK_H__
#define __PACK_H__

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

inline uint64_t ntoh64(uint64_t* input)
{
}

inline uint64_t hton64(uint64_t* input)
{
}

inline uint64_t get_uid_from_stream(const void* data)
{
}

// 外部的协议，需要和客户端通信
void serialize_msg(const Message& msg, char* &result, uint32_t& size, uint32_t type);
// 内部的协议，不和客户端通信
void serialize_imsg_type(const Message& msg, char* &result, uint32_t& size, uint64_t uid, uint32_t type);  //不加包头

void serialize_imsg(const Message& msg, char* &result, uint32_t& size, uint64_t uid);


///////////////////////////////////////////////////////////////////////////////////////////////
/*
wire format
消息格式: 与客户端通信
uint32_t dataLen
uint16_t nameLen
char typeName[namelen]
char protobufData[dataLen - namelen]

消息格式: 与其他服务进行通信
uint64_t uid
uint16_t namelen
char typeName[namelen]
char protobufData[len - uid - namelen]
*/


typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufCodec : boost::noncopyable
{
public:
    enum ErrorCode
    {
        kNoError = 0,
        kInvalidLength,
        kCheckSumError,
        kInvalidNameLen,
        kUnknowMessageType,
        kParseError,
    };

    typedef boost::function<void(const MessagePtr&)> ProtobufMsgCallback;

    typedef boost::function<void(const char* buffer, uint32_t size, ErrorCode)> ErrorCallback;

    explicit ProtobufCodec(const ProtobufMsgCallback& msgCb)
        : msgCallback_(msgCb), errorCallback_(defaultErrorCallback)
    {
    }

    ProtobufCodec(const ProtobufMsgCallback& msgCb, const ErrorCallback& errorCb)
        : msgCallback_(msgCb), errorCallback_(errorCb)
    {
    }

    void onMessage(const char* buffer, uint32_t size);

    void send(skynet_context* ctx, uint32_t fd, const google::protobuf::Message& message);

    static const std::string& errorCodeToString(ErrorCode errCode);

    void fillEmptyBuffer(char*& buffer, uint32_t& size, const goole::protobuf::Message& message);

    google::protobuf::Message* createMessage(const std::string& type_name);

    MessagePtr parse(const char* buffer, int len, ErrorCode* error);

private:
    static void defaultErrorCallback(const char* buffer, uint32_t size, ErrorCode errorCode);

    ProtobufMsgCallback msgCallback_;
    ErrorCallback errorCallback_;

    const static int kMessageLen = 4; // 消息的总长度 用4个byte来存储消息的总长度 PACK_HEAD
    const static int kTypeNameLen = 2; // 协议名称的长度 TYPE_HEAD
    const static int kMaxTypeNameLen = 50; // 协议名称的最大长度 MAX_TYPE_SIZE
    const static int kMaxMessageLen = 0x1000000; // 64 * 1024 * 1024 MAX_PACK_SIZE
};


///////////////////////////////////////////////////////////////////////////////////////////////
//char* to message
// 解码器
// 把从网络端接受的数据反序列化成protobuf消息
class InPack
{
public:
    InPack();
    bool reset(const char* data, uint32_t size = 0);
    bool inner_reset(const char* cdata, uint32_t size = 0);

    Message* create_message();

public:
    std::string m_type_name;    // 协议名称
    int64_t m_uid;              // 发送协议的uid

private:
    InPack(const char* data, uint32_t size = 0);
    bool decode_stream(const char* data, uint32_t size);

    const char* m_data;
    uint32_t    m_data_len;
    const char* m_pb_data;
    uint32_t    m_pb_data_len;

    const static int kMessageLen = 4; // 消息的总长度 用4个byte来存储消息的总长度 PACK_HEAD
    const static int kTypeNameLen = 2; // 协议名称的长度 TYPE_HEAD
    const static int kMaxTypeNameLen = 50; // 协议名称的最大长度 MAX_TYPE_SIZE
    const static int kMaxMessageLen = 0x1000000; // 64 * 1024 * 1024 MAX_PACK_SIZE
};

///////////////////////////////////////////////////////////////////////////////////////////////
//message to char*
// 把protobuf消息序列化成char*用于发送给网络端(需要添加包头)
// 发送给内部的服务 uid + 
// 发送给外部的服务 总长度 +
class OutPack
{
public:
    OutPack() {}
    OutPack(const Message& msg)
    {
        
    }
    bool reset(const Message& msg);
    void new_outpack(char*& result, uint32_t& size, uint32_t sub_type);
    // 用于RPC
    void new_innerpack_type(char*& result, uint32_t& size, uint64_t uid, uint32_t type);
    // 用于RPC
    void new_innerpack(char*& result, uint32_t& size, uint64_t uid);

private:
    std::string m_pb_data;      // pb数据的字符串
    std::string m_type_name;    // pb协议的名称

    const static int kMessageLen = 4; // 消息的总长度 用4个byte来存储消息的总长度 PACK_HEAD
    const static int kTypeNameLen = 2; // 协议名称的长度 TYPE_HEAD
    const static int kMaxTypeNameLen = 50; // 协议名称的最大长度 MAX_TYPE_SIZE
    const static int kMaxMessageLen = 0x1000000; // 64 * 1024 * 1024 MAX_PACK_SIZE
};

///////////////////////////////////////////////////////////////////////////////////////////////
//key=value key=value格式的字符串
class TextParm
{
public:
    TextParm(const char* stream);
    TextParm(const char* stream, int len);
    TextParm() {}

    void deserilize(const char* stream, size_t len);
    const char* serilize();

    size_t serilize_size()
    {
        return m_stream.size();
    }
    void earse(const std::string& key)
    {
        m_data.erase(key);
    }
    void insert(const std::string& key, const std::string& value)
    {
        m_data[key] = value;
    }
    void insert_int(const std::string& key, int value)
    {
        char v[10];
        sprintf(v, "%d", value);
        m_data[key] = v;
    }
    void insert_uint(const std::string& key, uint32_t value)
    {
        char v[10];
        sprintf(v, "%u", value);
        m_data[key] = v;
    }

    const std::string& get_string(const std::string& key) const
    {
        std::map<std::string, std::string>::iterator it = m_data.find(key);
        if (it == m_data.end())
        {
            return nullstring;
        }
        return it->second;
    }
    const char* get(const std::string& key) const
    {
        return get_string(key).c_str();
    }
    int get_int(const std::string& key) const
    {
        return atoi(get(key));
    }
    int64_t get_int64(const std::string& key) const
    {
        return atoll(get(key));
    }
    uint32_t get_uint(const std::string& key) const
    {
    	return strtoul(get(key), NULL, 10);
    }
    void print() const
    {
        std::map<std::string, std::string>::const_iterator it = m_data.begin();
        while (it != m_data.end())
        {
            printf("%s=%s\n", it->first.c_str(), it->second.c_str());
            ++it;
        }
    }

    bool has(const std::string& key)
    {
        std::map<std::string, std::string>::const_iterator it = m_data.find(key);
        if (it == m_data.end())
            return false;
        return true;
    }
    

private:
    std::map<std::string, std::string> m_data;
    static const std::string nullstring;
    std::string m_stream;
};

#endif

