-- 布防管理
require"sys"
require"misc"
require"pinop"
require"led"
module(...,package.seeall)
local pinset = pinop.pinset
local pinget = pinop.pinget

-- 尾箱开启后自动关闭时间
local AUTO_CLOSE_TRUNK = 5000

-- 布撤防声光信号次数
local ALERT_LS_TIME,DISARM_LS_TIME = 2,1

-- 初始化完成
local ready = false

-- ACC状态
local ACC_ON = pinget(pins.def.ACC_IN_INT)
-- 汽车行驶状态
local carun = false

-- 防区定义
local DEFSIDE = 1 -- 侧门
local DEFTRUNK = 2 -- 后备箱
local DEFSHAKE = 3 -- 振动防区
local DEF4 = 4 -- 油箱
local DEF5 = 5 -- 电池
local DEFUEL = DEF4 -- 油箱
local DEFEPOWER = 99 -- 汽车电瓶

--事件定义 布防 撤防 超时 触发
local EVT_ALERT = 1
local EVT_DISARM = 2
local EVT_TIMEOUT = 3
local EVT_ALARM_STOP = 4
local EVT_ACC = 5 -- ACC信号变化
local EVT_TRIG = 0x100
local EVT_SIDE = EVT_TRIG+DEFSIDE
local EVT_WDEF = EVT_TRIG+0x80

--[=[
状态定义
DISARM:撤防状态
ALERT:布防状态
DISARMING:撤防中
ALERTING:布防中
ALARM:警报中
--]=]
local currstate = "DISARM"

-- 中控锁
local lock = -1
-- 控制中控锁吸合
local function locktimeout()
	pinset(false,pins.def.LOCK1_EN)
	pinset(false,pins.def.LOCK2_EN)
end

local function setlock(val)
	if lock == val then return end

	lock = val

	if lock == true then
		-- 加锁
		pinset(false,pins.def.LOCK2_EN)
		pinset(true,pins.def.LOCK1_EN)
	else
		-- 解锁
		pinset(false,pins.def.LOCK1_EN)
		pinset(true,pins.def.LOCK2_EN)
	end
	-- 产生1.5秒脉冲控制中控锁吸合
	sys.timer_start(locktimeout,1500)
end

-- 闪烁
local times = 0
local total = 0

function blinkon(mode)
	pinset(true,pins.def.LR_LIGHT_EN)
	if mode == "lightsound" then
		pinset(true,pins.def.ALERT_EN)
	end
	sys.timer_start(blinkoff,500,mode)
end

function blinkoff(mode)
	pinset(false,pins.def.LR_LIGHT_EN)
	if mode == "lightsound" then
		pinset(false,pins.def.ALERT_EN)
	end
	times = times+1
	if times >= total then
		times = 0
		total = 0
	else
		sys.timer_start(blinkon,500,mode)
	end
end

local function slstop()
	sys.timer_stop(slstop)
	sys.timer_stop(blinkon)
	sys.timer_stop(blinkoff)
	pinset(false,pins.def.ALERT_EN)
	pinset(false,pins.def.LR_LIGHT_EN)
end

local function slstart(val)
	slstop()
	times = 0
	total = val
	if val > 1000 then
		sys.timer_start(slstop,val)
		pinset(true,pins.def.ALERT_EN)
		blinkon("light")
	else
		blinkon("lightsound")
	end
end

-- 检查防区
-- 保存防区状态
local function getdefstatus()
	local tstatus = {}

	tstatus[DEFEPOWER] = not pinget(pins.def.EPOWER)

	tstatus[DEFSHAKE] = pinget(pins.def.SHAKE_IN_INT)
	tstatus[DEFSIDE] = pinget(pins.def.SIDE_IN_INT)
	-- 尾箱防区设置为屏蔽
	if nvm.get("def2valid") == "屏蔽" then
		tstatus[DEFTRUNK] = false
	else
		tstatus[DEFTRUNK] = pinget(pins.def.TRUNK_IN_INT)
	end

	pinset(false,pins.def.VOLT_CHOICE)
	rtos.sleep(10)
	if nvm.get("def4stat") == "短路" then
		tstatus[DEF4] = pinget(pins.def.SHORT_ALARM)
	else
		tstatus[DEF4] = pinget(pins.def.PLOUGH_ALARM)
	end

	pinset(true,pins.def.VOLT_CHOICE)
	rtos.sleep(10)
	if nvm.get("def5stat") == "短路" then
		tstatus[DEF5] = pinget(pins.def.SHORT_ALARM)
	else
		tstatus[DEF5] = pinget(pins.def.PLOUGH_ALARM)
	end

	return tstatus
end

local defstatus = {}

local alertnoshake = false
local onlyfuel = false

local function checkstatus(t)
	local talarm = {}

	-- 检查汽车电瓶
	if t[DEFEPOWER] == true then print("EPOWER not ready") return DEFEPOWER end --主电异常只报主电

	-- 检查油箱防区
	if onlyfuel == true then print("DEFUEL not ready") return t[DEFUEL] == true and DEFUEL or true end

	-- 检查振动防区
	if alertnoshake == false and t[DEFSHAKE] == true then print("DEFSHAKE not ready") table.insert(talarm,DEFSHAKE) end

	if onlyfuel ~= true then
		-- 检查其他防区
		for k,v in pairs(t) do
			if k ~= DEFSHAKE and k ~= DEFEPOWER and k ~= DEFUEL and v == true then
				if nvm.get("alarmtext",k) ~= "" then -- 只处理有报警信息的防区,无报警信息防区认为无效防区
					print("DEF not ready:",k)
					table.insert(talarm,k)
				end
			end
		end
	end

	if #talarm == 0 then
		-- 无防区异常
		return true
	elseif #talarm == 1 then
		-- 只有一个防区异常,直接提示异常防区
		return talarm[1]
	else
		-- 有多个防区异常,返回所有异常防区
		return talarm
	end
end

local function check()
	local t = getdefstatus()

	for k,v in pairs(t) do
		if defstatus[k] ~= v then
			defstatus[k] = v
			-- 防区正常 清除
			if v == false then
				t[k] = nil
			end
		else
			t[k] = nil
		end
	end

	return checkstatus(t)
end

local function fsmtimeout()
	procevt(EVT_TIMEOUT)
end

-- 布防成功
local function alertsucc()
	sys.timer_stop(fsmtimeout)
	-- 布防成功声光信号提示
	slstart(ALERT_LS_TIME)
	currstate = "ALERT"
	alertnoshake = false
	led.work("alertind")
end

-- 撤防
local function procdisarm(user,mode)
	-- 撤防时清除防区状态
	defstatus = {}

	-- 只撤防振动
	if mode == "ALERT_NO_SHAKE" then
		alertnoshake = true
		slstart(ALERT_LS_TIME)
		return
	else
		alertnoshake = false
	end

	led.work("idle")
	-- 撤防时中控锁解锁
	setlock(false)
	-- 关闭断电断油
	pinset(true,pins.def.OIL_EN)
	pinset(true,pins.def.ENGINE_EN)
	alarm.stop("disarm")
	slstart(DISARM_LS_TIME) -- 撤防成功声光信号

	-- 任何方式的撤防都要做防误解除的处理
	if currstate == "ALERT" or currstate == "DISARMING" then -- 只在布防或者待撤防状态下才作防误解除的处理
		-- 防误解除处理
		currstate = "DISARMING"
		sys.timer_start(fsmtimeout,nvm.get("delayautoalert"))
	else
		currstate = "DISARM"
	end

	return true
end

-- 布防
local function procalert(user,mode)
	if mode == "ONLY_FUEL" then
		--只布防油箱
		onlyfuel = true
	else
		onlyfuel = false
	end

	setlock(true)

	-- 油箱单独布防时 不影响中控锁跟断电断油控制
	if onlyfuel == false then -- 不论是否布防成功都要控制中控锁跟电油
		-- 断电断油
		pinset(false,pins.def.OIL_EN)
		pinset(false,pins.def.ENGINE_EN)
	end

	if check() == true then
		-- 全防区检查通过
		alertsucc()
		return true
	end

	-- 声光信号20次提示车主防区未准备好
	slstart(20)
	sys.timer_start(fsmtimeout,nvm.get("alertdelaytime"))

	currstate = "ALERTING"
	return false
end

local function dodisarm(evt,user,mode)
	if evt == EVT_ALERT then
		return procalert(user,mode)
	elseif evt == EVT_DISARM then
		-- 遥控器重复布撤防要作出对应声光提示
		if user == "controller" then
			return procdisarm(user,mode)
		else
			return true
		end
	else
		return false
	end
end

local function doalerting(evt,user,mode)
	if evt == EVT_ALERT then -- 延时检查防区状态的情况下重复布防的处理
		return procalert()
	elseif evt == EVT_TIMEOUT then
		-- 超时检查, 布防成功或者失败
		local result = checkstatus(defstatus)
		if result == true then
			alertsucc()
		else
			-- 进入布防状态
			currstate = "ALERT"
			led.work("alertind")
			alertnoshake = false
			-- 发出报警
			alarm.start(result)
		end
	elseif evt > EVT_WDEF then
		-- 无线防区触发
	elseif evt > EVT_TRIG then
		-- 防区状态变化触发
		check() -- 更新防区状态
		if checkstatus(defstatus) == true then
			alertsucc()
		end
	elseif evt == EVT_DISARM then
		sys.timer_stop(fsmtimeout)
		procdisarm(user,mode)
	end
	return true
end

local function gotoalarm1(defid)
	currstate = "ALARM1"
	slstart(20000) -- 20秒声光信号
	sys.timer_start(fsmtimeout,15000) -- 15秒继续监测
	-- 防区警报触发
	alarm.start(defid)
end

local function doalert(evt,user,mode)
	if evt == EVT_ALERT then
		-- 重复布撤防要作出对应声光提示
		return procalert(user,mode)
	elseif evt > EVT_WDEF then
		-- 无线防区触发
		gotoalarm1(evt-EVT_WDEF)
	elseif evt > EVT_TRIG then
		local result = check()
		if result ~= true then
			gotoalarm1(result)
		end
	elseif evt == EVT_DISARM then
		procdisarm(user,mode)
	end
	return true
end

local function gotoalarm2(defid)
	-- 15秒内再次触发防区
	currstate = "ALARM2"
	-- 此时触发声光信号持续40秒
	slstart(40000)
	sys.timer_start(fsmtimeout,40000)
	alarm.start(defid)
end

local function doalarm1(evt,user,mode)
	if evt > EVT_WDEF then
		-- 无线防区触发
		gotoalarm2(evt-EVT_WDEF)
	elseif evt > EVT_TRIG then
		local result = check()
		if result ~= true then
			gotoalarm2(result)
		end
	elseif evt == EVT_TIMEOUT then
		-- 超时返回布防状态
		currstate = "ALERT"
	elseif evt == EVT_DISARM then
		procdisarm(user,mode)
	end
	return true
end

local function alarmdef(defid)
	-- 此时任何触发都是声光信号持续40秒
	slstart(40000)
	sys.timer_start(fsmtimeout,40000)
	alarm.start(defid)
end

local function doalarm2(evt,user,mode)
	if evt > EVT_WDEF then
		-- 无线防区触发
		alarmdef(evt-EVT_WDEF)
	elseif evt > EVT_TRIG then
		local result = check()
		if result ~= true then
			alarmdef(result)
		end
	elseif evt == EVT_TIMEOUT then
		-- 超时返回布防状态
		currstate = "ALERT"
	elseif evt == EVT_DISARM then
		procdisarm(user,mode)
	end
	return true
end

local function dodisarming(evt,user,mode)
	if evt == EVT_TIMEOUT or evt == EVT_ALERT then
		-- 超时,车门未打开,误解除
		return procalert(user,mode)
	elseif evt > EVT_TRIG then
		if pinget(pins.def.SIDE_IN_INT) == true then
			-- 车门打开, 撤防
			currstate = "DISARM"
			sys.timer_stop(fsmtimeout)
		end
	elseif evt == EVT_DISARM then
		-- 遥控器重复布撤防要作出对应声光提示
		return procdisarm(user,mode)
	elseif evt == EVT_ACC then
		-- 待撤防, ACC点火进入撤防状态
		if ACC_ON == true then
			currstate = "DISARM"
			sys.timer_stop(fsmtimeout)
		end
	end

	return true
end

local fsm =
{
	{state="DISARM",		evt=EVT_ALERT,			action=dodisarm},
	{state="DISARM",		evt=EVT_DISARM,			action=dodisarm},

	{state="ALERTING",		evt=EVT_TRIG,			action=doalerting},
	{state="ALERTING",		evt=EVT_TIMEOUT,		action=doalerting},
	{state="ALERTING",		evt=EVT_DISARM,			action=doalerting},

	{state="ALERT",			evt=EVT_ALERT,			action=doalert},
	{state="ALERT",			evt=EVT_TRIG,			action=doalert},
	{state="ALERT",			evt=EVT_DISARM,			action=doalert},

	{state="ALARM1",		evt=EVT_TRIG,			action=doalarm1},
	{state="ALARM1",		evt=EVT_TIMEOUT,		action=doalarm1},
	{state="ALARM1",		evt=EVT_DISARM,			action=doalarm1},

	{state="ALARM2",		evt=EVT_TRIG,			action=doalarm2},
	{state="ALARM2",		evt=EVT_TIMEOUT,		action=doalarm2},
	{state="ALARM2",		evt=EVT_DISARM,			action=doalarm2},

	{state="DISARMING",		evt=EVT_ALERT,			action=dodisarming},
	{state="DISARMING",		evt=EVT_TIMEOUT,		action=dodisarming},
	{state="DISARMING",		evt=EVT_TRIG,			action=dodisarming},
	{state="DISARMING",		evt=EVT_DISARM,			action=dodisarming},
	{state="DISARMING",		evt=EVT_ACC,			action=dodisarming},
}

function procevt(evt,user,mode)
	--  未初始化完成不允许控制
	if ready == false then return false end

	-- ACC开启时遥控器控制处理
	if ACC_ON == true and user == "controller" then
		if evt == EVT_DISARM then -- 遥控器可以执行任何撤防动作
		elseif evt == EVT_ALERT and mode == "ONLY_FUEL" then -- 遥控器可以单独布防油箱
		else
			print("acc is on, cannot control")
			return false
		end
	end

	print("FSM:",currstate,evt,user,mode)

	for k,v in pairs(fsm) do
		if currstate == v.state then
			if v.evt == evt then
				return v.action(evt,user,mode)
			elseif v.evt == EVT_TRIG and evt > v.evt then
				return v.action(evt,user,mode)
			end
		end
	end
end

function getstate()
	return currstate
end

function alert(user)
	if user == "controller" and ACC_ON == true then setlock(true) return end -- 行驶中按防盗键，中控锁加锁

	return procevt(EVT_ALERT,user)
end

function disarm(user)
	if user == "controller" and carun == true then setlock(false) return end -- 行驶中按解除键，中控锁解锁

	return procevt(EVT_DISARM,user)
end

function alarmstop(user)
	return procevt(EVT_ALARM_STOP,user)
end

function noshake()
	procevt(EVT_DISARM,"controller","ALERT_NO_SHAKE")
end

function fueltank()
	procevt(EVT_ALERT,"controller","ONLY_FUEL")
end

local function closetrunk()
	pinset(false,pins.def.TRUNK_OUT_EN)
end

function opentrunk()
	pinset(true,pins.def.TRUNK_OUT_EN)
	sys.timer_start(closetrunk,AUTO_CLOSE_TRUNK)
end

local function delaytrig(evt)
	procevt(evt)
end

local function trigevt(evt)
	local t = getdefstatus()

	-- 检查如果突然超过两个防区发生异常 那么延时检测是否EPOWER断电
	for k,v in pairs(t) do
		if defstatus[k] == nil then t = {} break end

		if v == false or defstatus[k] == v then
			t[k] = nil
		end
	end

	t = checkstatus(t)

	if type(t) == "table" and #t > 1 then
		sys.timer_start(delaytrig,2000,evt)
		return
	end

	procevt(evt)
end

local DELAY_TRIG = 800 -- 延时800ms后检查防区状态

local function trunkind()
	sys.timer_start(trigevt,DELAY_TRIG,EVT_TRIG+DEFTRUNK)
end

local function sideind()
	sys.timer_start(trigevt,DELAY_TRIG,EVT_TRIG+DEFSIDE)
end

local function shakeind()
	sys.timer_start(trigevt,DELAY_TRIG,EVT_TRIG+DEFSHAKE)
end

-- 汽车电瓶状态
local function epowerind(status)
	sys.timer_start(trigevt,DELAY_TRIG,EVT_TRIG+DEFEPOWER)
end

-- 中控锁控制
local acctimeout = false

local function brakeind(status)
	if ACC_ON == true and status == true then
		-- 15秒后踩了刹车自动加锁
		if acctimeout == true then
			acctimeout = false
			carun = true
			setlock(true) -- 汽车行驶自动加锁
		end
	end
end

local function acctimer()
	acctimeout = true
end

local function accind(status)
	ACC_ON = status

	if status == true then
		acctimeout = false
		sys.timer_start(acctimer,15000)
	else
		sys.timer_stop(acctimer)
		acctimeout = false
		carun = false
		setlock(false) -- 汽车熄火自动解锁
	end
	procevt(EVT_ACC)
end

-- 防区4 5 轮流检测控制
local function def45timer()
	sys.timer_start(def45timer,2000)

	if currstate == "DISARM" then return end

	local def,status

	def = pins.def.VOLT_CHOICE.val == true and DEF5 or DEF4

	local trigstat = nvm.get(def == DEF4 and "def4stat" or "def5stat")
	status = pinget(trigstat == "短路" and pins.def.SHORT_ALARM or pins.def.PLOUGH_ALARM)

	pinset(not pins.def.VOLT_CHOICE.val,pins.def.VOLT_CHOICE)

	if defstatus[def] ~= status then -- 状态发生变化才触发
		sys.timer_start(trigevt,DELAY_TRIG,EVT_TRIG+def)
	end
end

sys.timer_start(def45timer,2000)

local defind = {
	PIN_TRUNK_IND = trunkind,
	PIN_BRAKE_IND = brakeind,
	PIN_SIDE_IND = sideind,
	PIN_ACC_IND = accind,
	PIN_SHAKE_IND = shakeind,
	PIN_EPOWER_IND = epowerind,
}

sys.regapp(defind)

function wdeftrig(id)
	procevt(EVT_WDEF+id)
end

local function devready()
	ready = true
	led.work("readyind",3)
end

-- 有卡时以短信初始化完成为初始化结束标志
local function smsready()
	devready()
	sys.deregapp(smsready)
end
sys.regapp(smsready,"SMS_READY")

-- 无卡时等待2分钟为初始化结束标志
local function simind(msgid,simstat)
	if simstat == "NIST" then
		sys.deregapp(smsready)
		sys.timer_start(devready,60000)
	end
	sys.deregapp(simind)
	return true
end
sys.regapp(simind,"SIM_IND")

led.work("initing")
