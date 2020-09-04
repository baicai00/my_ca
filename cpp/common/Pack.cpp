#include "Pack.h"


void serialize_msg(const Message& msg, char* &result, uint32_t& size, uint32_t type)
{
}

void serialize_imsg_type(const Message& msg, char* &result, uint32_t& size, uint64_t uid, uint32_t type);  //不加包
{
}

void serialize_imsg(const Message& msg, char* &result, uint32_t& size, uint64_t uid)
{
}


///////////////////////////////////////////////////////////////////////////////////////////////
void ProtobufCodec::onMessage(const char* buffer, uint32_t size)
{
}

void ProtobufCodec::send(skynet_context* ctx, uint32_t fd, const google::protobuf::Message& message)
{
}

static const std::string& ProtobufCodec::errorCodeToString(ErrorCode errCode)
{
}

void ProtobufCodec::fillEmptyBuffer(char*& buffer, uint32_t& size, const goole::protobuf::Message& message)
{
}

google::protobuf::Message* ProtobufCodec::createMessage(const std::string& type_name)
{
}

MessagePtr ProtobufCodec::parse(const char* buffer, int len, ErrorCode* error)
{

}

void ProtobufCodec::defaultErrorCallback(const char* buffer, uint32_t size, ErrorCode errorCode)
{
}

