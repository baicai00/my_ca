local skynet = require "skynet"

local CMD = {}
local gtables = {}
local SOCKET = {}
local gate
local agent = {}

skynet.register_protocol {
    name = "client",
    id = skynet.PTYPE_CLIENT,
    pack = function(...) return ... end,
    unpack = skynet.tostring,
}

function SOCKET.open(fd, addr)

end

function SOCKET.close(fd)

end

function SOCKET.error(fd, msg)

end

function SOCKET.warning(fd, size)

end

function SOCKET.data(fd, msg)

end

--===========================================

function CMD.start(conf)

end

function CMD.close(fd)

end

--=============================================

function gtables.skynet_start()
    gate = skynet.newservice("ca_gate")

    skynet.dispatch("client", function(session, source, clientmsg)
        --TODO
    end)

    skynet.dispatch("lua", function(session, source, cmd, subcmd, ...)
        if cmd == "socket" then
            local f = SOCKET[subcmd]
            f(...)
        else
            local f = assert(CMD(cmd))
            skynet.ret(skynet.pack(f(subcmd, ...)))
        end
    end)
end

function gtables.skynet_info()

end

--=============================================

skynet.start(gtables.skynet_start)
skynet.info_func(gtables.skynet_info)