require"pm"
pm.wake()

require"common"
L=common.gb2312toucs2

require"nvm"
require"access"
require"alarm"
require"defense"
require"rmtctler"
require"rmtsms"
require"rmtcall"
require"gps"
require"watchdog"

local declaredNames = {_ = true}
function declare (name, initval)
    --rawset(_G, name, initval)
    declaredNames[name] = true
end

setmetatable(_G, {
    __newindex = function (t, n, v)
    if not declaredNames[n] then
       error("attempt to write to undeclared var. "..n, 2)
    else
       rawset(t, n, v)   -- do the actual set
    end
end,
    __index = function (_, n)
    if not declaredNames[n] then
		error("attempt to read undeclared var. "..n, 2)
    else
       return nil
    end
end,
})

sys.init()
sys.run()
