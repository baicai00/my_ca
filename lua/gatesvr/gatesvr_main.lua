local skynet = require "skynet"

local max_client = 64

skynet.start(function()
    skynet.error("gatesvr start")
    local watchdog = skynet.newservice("ca_watchdog")
    skynet.call(watchdog, "lua", "start", {
        port = 8888,
        maxclient = max_client,
        nodelay = true,
    })
    skynet.error("Watchdog listen on", 8888)
    skynet.exit()
end)