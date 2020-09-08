#include <glog/logging.h>
#include <stdint.h>

extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
}

class logger
{
public:
};

extern "C"
{
    static int _logger(struct skynet_context* ctx, void *ud, int type, int session, uint32_t source, const void* msg, size_t sz)
    {
        char temp[100];
        if (source != 0)
        {
            sprintf(temp, "[%08x] ", source);
            LOG(INFO) << temp << (const char*)msg;
        }
        else
        {
            LOG(INFO) << (const char*)msg;
        }

        return 0;
    }

    struct logger* logger_create(void)
    {
        struct logger * inst = (logger *)skynet_malloc(sizeof(*inst));
        return inst;
    }

    void logger_release(struct logger* inst)
    {
        skynet_free(inst);
    }

    int logger_init(struct logger* inst, struct skynet_context* ctx, const char * parm)
    {
        const char* dir = skynet_getenv("log_dir");
        const char* name = skynet_getenv("log_name");
        int logbufsecs = atoi(skynet_getenv("logbufsecs"));

        if (strcmp(name, "stdout") == 0)
        {
            FLAGS_logtostderr = true;
        }
        else
        {
            FLAGS_logtostderr = false;
            FLAGS_log_dir = dir;
            FLAGS_max_log_size = 10;
        }
        FLAGS_logbufsecs = logbufsecs;

        google::InitGoogleLogging(name);
        skynet_callback(ctx, inst, _logger);
        skynet_command(ctx, "REG", ".logger");
        return 0;
    }
}