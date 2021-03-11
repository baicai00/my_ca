#include <glog/logging.h>
#include "Main.h"


extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
}

using namespace std;

extern "C"
{
    static int main_cb(struct skynet_context * ctx, void * ud, int type, int session, uint32_t source, const void * msg, size_t sz)
    {
        Main* main = (Main*)ud;
        //log_debug("Main get a msg type:%d size:%d msg:%s", type, sz, msg);
        main->service_poll((const char*)msg, sz, source, session, type);

        return 0; //0表示成功
    }

    Main* main_create()
    {
        Main* main = new Main();
        return main;
    }

    void main_release(Main* main)
    {
        delete main;
    }

    int main_init(Main* main, struct skynet_context* ctx, char* parm)
    {
        if (!main->service_init(ctx, parm, strlen_1(parm)))
        {
            return -1;
        }
        skynet_callback(ctx, main, main_cb);
        return 0;
    }
}