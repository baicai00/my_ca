#include "common.h"
#include "ValueService.h"
#include <stdint.h>

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
    static int value_cb(struct skynet_context * ctx, void * ud, int type, int session, uint32_t source, const void * msg, size_t sz)
    {
        
    }

    ValueService* value_create()
    {
    }

    void value_release(ValueService* value)
    {
    }

    int value_init(ValueService* value, struct skynet_context* ctx, char* parm)
    {
    }
}

