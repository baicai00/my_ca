#include "Watchdog.h"


Watchdog::Watchdog()
{
}

bool Watchdog::dog_init(const std::string& parm)
{
}

uint32_t Watchdog::new_agent(int fd, int64_t uid)
{
}

uint32_t Watchdog::rebind_agent(int fd, AgentHandle* ah)
{
}

void Watchdog::destroy_agent(uint32_t handle)
{
}

void Watchdog::destroy_fd_agent(int fd)
{
}

void Watchdog::destroy_fd_agent(uint32_t handle)
{
}

void Watchdog::new_connection(int fd, const std::string& name)
{
}

void Watchdog::client_disconnect(int fd)
{
}

void Watchdog::dog_disconnect(int fd)
{
}

void Watchdog::dog_message(char* data, uint32_t size, int fd)
{
}

void Watchdog::watchdog_poll(const char* data, uint32_t size, uint32_t source, int session, int type)
{
    switch (type)
    {
    case PTYPE_TEXT:
    case PTYPE_CLIENT:
    case PTYPE_RESPONSE:
    default:
        break;
    }
}

void Watchdog::text_message(const char* msg, size_t sz, uint32_t source, int session)
{
}

