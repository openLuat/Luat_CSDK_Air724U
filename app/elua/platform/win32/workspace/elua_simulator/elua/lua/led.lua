
require"sys"
require"pinop"
module(...,package.seeall)

local mode="blink"

local cfg={
	initing = {ONTIME=500,OFFTIME=500},
	readyind = {ONTIME=500,OFFTIME=500},
	blink = {ONTIME=100,OFFTIME=2900},
	alertind = {ONTIME=100,OFFTIME=2900},
}

local ONTIME = 100
local OFFTIME = 2900
local blinks = 0
local blinktimes

local function blinkoff()
	if not cfg[mode] then return end

	pinop.pinset(false,pins.def.LED1_EN)
	pinop.pinset(false,pins.def.LED2_EN)
	sys.timer_start(blinkon,cfg[mode].OFFTIME)
end

function blinkon()
	if not cfg[mode] then return end

	blinks = blinks + 1

	if blinktimes and blinks > blinktimes then
		return
	end

	if mode == "initing" then
		-- 开机初始化红灯闪烁
		pinop.pinset(true,pins.def.LED2_EN)
	elseif mode == "alertind" or mode == "readyind" then
		-- 布防状态或者初始化完成绿灯闪烁
		pinop.pinset(true,pins.def.LED1_EN)
	elseif mode == "blink" then
		-- 编程状态红绿灯一起闪烁
		pinop.pinset(true,pins.def.LED1_EN)
		pinop.pinset(true,pins.def.LED2_EN)
	end
	sys.timer_start(blinkoff,cfg[mode].ONTIME)
end

function work(mval,val)
	sys.timer_stop(blinkon)
	sys.timer_stop(blinkoff)
	if mval == "idle" then
		mode = mval
		pinop.pinset(false,pins.def.LED1_EN)
		pinop.pinset(false,pins.def.LED2_EN)
	elseif mval == "programend" then
		mode = mval
		pinop.pinset(true,pins.def.LED1_EN)
		pinop.pinset(false,pins.def.LED2_EN)
	else
		pinop.pinset(false,pins.def.LED1_EN)
		pinop.pinset(false,pins.def.LED2_EN)
		mode = mval

		blinks = 0
		blinktimes = val
		blinkon()
	end
end
