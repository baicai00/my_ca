#ifndef _COMMON_H_
#define _COMMON_H_

// msg service type
enum ServiceType
{
    SERVICE_NULL = -1,  //抛弃消息
    SERVICE_NONE = 0,  //还没找到service处理
    SERVICE_HALL = 1,
    SERVICE_ROOM = 2,
    SERVICE_CLUB = 3,
    SERVICE_MAIL = 4,
    SERVICE_SHOP = 5,
    //SERVICE_LADDER = 6,
    SERVICE_PINE = 7, //未使用
    //SERVICE_LOBBY = 8,
    SERVICE_CAREER = 9, //生涯
    SERVICE_ROUTER = 10,
    SERVICE_VALUE = 11,
    SERVICE_LUAGAME = 12,
    SERVICE_ACTIVITY = 14,
    SERVICE_LOBBY = 15,
    SERVICE_FRIEND = 16,
    SERVICE_ANTI_CHEATING = 17,
    SERVICE_GROUP = 18,
};

enum ServerStopType
{
    NO_STOP = 0, //不停服
    STOP_SERVER = 1, //停服，踢人
    STOP_CREATE_ROOM = 2,  //停止开卓
};

#endif