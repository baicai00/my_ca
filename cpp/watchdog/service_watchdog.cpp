#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Watchdog.h"

extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
#include "skynet_timer.h"
}

extern "C"
{
    static int watchdog_cb(struct skynet_context* ctx, void* ud, int type, int session, uint32_t source, const void* msg, size_t sz)
    {
        Watchdog* dog = (Watchdog*)ud;
        dog->watchdog_poll((const char*)msg, sz, source, session, type);

        return 0;
    }

    Watchdog* watchdog_create()
    {
        Watchdog* dog = new Watchdog();
        return dog;
    }

    void watchdog_release(Watchdog* dog)
    {
        delete dog;
    }

    // 在watchdog.lua中会调用skynet.launch函数启动watchdog服务,该函数的第二个参数会传递给parm
    int watchdog_init(Watchdog* dog, struct skynet_context* ctx, char* parm)
    {
        uint32_t handle = skynet_context_handle(ctx);
        char sendline[100];
        snprintf(sendline, sizeof(sendline), "%s %u", parm, handle);

        // watchdog新建一个gate服务
        struct skynet_context* gate_ctx = skynet_context_new("gate", sendline);
        if (gate_ctx == NULL)
        {
            return -1;
        }

        uint32_t gate = skynet_context_handle(gate_ctx);
        char temp[100];
        sprintf(temp, "%u", gate);
        if (!dog->service_init(ctx, temp, strlen(temp)))
        {
            return -1;
        }
        skynet_callback(ctx, dog, watchdog_cb);

        // 机器人用到(没看懂是咋用的!!!!)
        skynet_handle_namehandle(gate, "gate");
        skynet_handle_namehandle(handle, "watchdog");

        return 0;
    }
}
