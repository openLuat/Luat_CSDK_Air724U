
require"pinop"
module(...,package.seeall)

local first = false

local function change()
	if first == false then
		first = true
		pio.pin.setdir(pins.def.WATCHDOG.dir or pio.OUTPUT,pins.def.WATCHDOG.pin)
		pinop.pinset(pins.def.WATCHDOG.defval or false,pins.def.WATCHDOG)
	else
		pinop.pinset(not pins.def.WATCHDOG.val,pins.def.WATCHDOG)
	end
	sys.timer_start(change,900)
end

sys.timer_start(change,900)
