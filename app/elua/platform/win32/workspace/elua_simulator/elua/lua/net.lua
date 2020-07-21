
local base = _G
local string = require"string"
local sys = require "sys"
local ril = require "ril"
module("net")

local dispatch = sys.dispatch
local req = ril.request
local smatch = string.match
local tonumber = base.tonumber

local state = "INIT"
local lac,ci
local csqqrypriod = 60*1000

local function creg(data)
	local p1,s
	_,_,p1 = string.find(data,"%d,(%d)")
	if p1 == nil then
		_,_,p1 = string.find(data,"(%d)")
		if p1 == nil then
			return
		end
	end

	if p1 == "1" or p1 == "5" then
		s = "REGISTERED"
	else
		s = "UNREGISTER"
	end

	if s ~= state then
		state = s
		dispatch("NET_STATE_CHANGED",s)
	end

	if state == "REGISTERED" then
		p2,p3 = string.match(data,"\"(%x+)\",\"(%x+)\"")
		if lac ~= p2 or ci ~= p3 then
			lac = p2
			ci = p3
		end
	end
end

local function neturc(data,prefix)
	if prefix == "+CREG" then
		creg(data)
	end
end

function getstate()
	return state
end

function getlac()
	return lac
end

function getci()
	return ci
end

local queryfun = function()
	startquerytimer()
end

function startquerytimer()
	req("AT+CREG?")
	if state ~= "REGISTERED" then
		sys.timer_start(queryfun,2000)
	end
end

local function SimInd(id,para)
	if para == "NIST" then
		sys.timer_stop(queryfun)
	end

	return true
end

local csqquerytimerFun = function()
	startcsqtimer()
end

function startcsqtimer()
	req("AT+CSQ")
	sys.timer_start(csqquerytimerFun,csqqrypriod)
end

local function rsp(cmd,success,response,intermediate)
	local prefix = string.match(cmd,"AT(%+%u+)")

	if intermediate ~= nil then
		if prefix == "+CSQ" then
			local rssi = smatch(intermediate,"+CSQ:%s*(%d+)")
			if rssi ~= nil then
				rssi = tonumber(rssi)
				dispatch("GSM_SIGNAL_REPORT_IND",success,rssi)
			end
		end
	end
end

function setcsqqueryperiod(period)
	csqqrypriod = period
	startcsqtimer()
end

sys.regapp(SimInd,"SIM_IND")
ril.regurc("+CREG",neturc)
ril.regrsp("+CSQ",rsp)
req("AT+CREG=2")
startquerytimer()
startcsqtimer()
