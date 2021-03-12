local skynet = require "skynet"
local gateserver = require "snax.gateserver"


local handler = {}
local CMD = {}

function handler.connect(fd, ipaddr)

end

function handler.disconnect(fd)

end

function handler.error(fd, msg)

end

function handler.command(cmd, source, ...)

end

gateserver.start(handler)