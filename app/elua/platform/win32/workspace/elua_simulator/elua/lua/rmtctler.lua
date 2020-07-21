-- 遥控器

require"sys"
require"common"
require"nvm"
require"defense"
local sbyte = string.byte
module(...,package.seeall)

-- 事件防抖: 间隔时间、次数
local EVT_DEBOUNCE_INTERVAL=150
local EVT_DEBOUNCE_TIMES=3

local EVT_WDEF = 0
local KEYALERT=14
local KEYDISARM=13
local KEYNOSHAKE=11
local KEYONLYFUEL=7
local KEYOPENTRUNK=3
local lastdata

local function normal()
	print("controller normal contact")
end

local function alert()
	print("controller alert")
end

local function lowbat()
	print("controller low battery")
end

local function fangchai()
	print("controller fang chai")
end

local function prockeyalert()
	defense.alert("controller")
end

local function prockeydisarm()
	defense.disarm("controller")
end

local function prockeynoshake()
	defense.noshake()
end

local function prockeyfueltank()
	defense.fueltank()
end

local function prockeyopentrunk()
	defense.opentrunk()
end

local evts =
{
--[=[
	[0x00] = normal,
	[0x01] = alert,
	[0x02] = lowbat,
	[0x03] = fangchai,
--]=]
	[KEYALERT] = prockeyalert,
	[KEYDISARM] = prockeydisarm,
	[KEYNOSHAKE] = prockeynoshake,
	[KEYONLYFUEL] = prockeyfueltank,
	[KEYOPENTRUNK] = prockeyopentrunk,
}
setmetatable(evts,{__index = function(t,n) print("unknown evt:",n) return function() end end})

local function procevt(evt)
	evts[evt]()
end

-- 遥控器学习
local learning = false
function enterlearn()
	learning = true
end

function exitlearn()
	learning = false
end

local lastevt
local debounces

local function evtdebounce()
	lastevt = nil
end

local function parse(s)
	-- print(sbyte(s,1,-1))

	if lastevt == nil then
		lastevt = s
		debounces = 0
		sys.timer_start(evtdebounce,EVT_DEBOUNCE_INTERVAL)
		return
	end

	if lastevt == s then
		sys.timer_start(evtdebounce,EVT_DEBOUNCE_INTERVAL)
		debounces = debounces+1
		if debounces ~= EVT_DEBOUNCE_TIMES then
			return
		else
			-- 只在达到防抖次数时处理一次事件,后面连续的事件判定为连续触发不处理
		end
	else
		lastevt = s
		debounces = 0
		sys.timer_start(evtdebounce,EVT_DEBOUNCE_INTERVAL)
		return
	end

	if learning == true then
		access.ctrlerlearn(sbyte(s,1)*256+sbyte(s,2))
		learning = false
		return
	end

	-- 编程状态下不允许使用遥控器
	if access.inprogram() == true then
		print("programing cannot use controller")
		return
	end

	local devid = sbyte(s,1)*256+sbyte(s,2)
	local evtid = sbyte(s,3)%16 -- event只取低4位

	if nvm.validctrler(devid) == true then
		procevt(evtid)
	else --if evtid == EVT_WDEF then 为兼容新旧接收板,暂时不匹配数据
		local wdefid = nvm.checkwdef(devid)
		if wdefid then
			defense.wdeftrig(wdefid)
		end
	end
end

local function recv()
	local s,l

	repeat
		s = uart.read(1,"*l")

		l = string.len(s)
		while l >= 3 do
			parse(string.sub(s,1,3))
			s = string.sub(s,4,-1)
			l = string.len(s)
		end
	until l < 3
end
sys.reguart(1,recv)

pmd.ldoset(7,pmd.LDO_VMMC)
uart.setup(1,4800,8,uart.PAR_NONE,uart.STOP_1)
