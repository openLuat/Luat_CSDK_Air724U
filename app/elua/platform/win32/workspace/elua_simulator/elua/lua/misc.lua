-- ∆‰À˚≈‰÷√
local string = require"string"
local ril = require"ril"
local sys = require"sys"
local base = _G
local os = require"os"
module(...)

local tonumber = base.tonumber
local tostring = base.tostring
local req = ril.request
local sn
local ver

local CCLK_QUERY_TIMER_PERIOD = 60*1000
local clk = {}

function setclock(t,rsp)
	req(string.format("AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+32\"",string.sub(t.year,3,4),t.month,t.day,t.hour,t.min,t.sec),nil,rsp)
	startclktimer()
end

function getclockstr()
	clk = os.date("*t")
	clk.year = string.sub(clk.year,3,4)
	return string.format("%02d%02d%02d%02d%02d%02d",clk.year,clk.month,clk.day,clk.hour,clk.min,clk.sec)	
end

function getclock()
	return os.date("*t")
end

local CclkQueryTimerFun = function()		
	startclktimer()
end

function startclktimer()
	sys.dispatch("CLOCK_IND")
	sys.timer_start(CclkQueryTimerFun,CCLK_QUERY_TIMER_PERIOD)
end

function getsn()
	return sn
end

function getbasever()
	if ver ~= nil and base._INTERNAL_VERSION ~= nil then
		local d1,d2,bver,bprj,lver
		d1,d2,bver,bprj = string.find(ver,"_V(%d+)_(.+)")
		d1,d2,lver = string.find(base._INTERNAL_VERSION,"_V(%d+)")
		
		if bver ~= nil and bprj ~= nil and lver ~= nil then
			return "SW_V" .. lver .. "_" .. bprj .. "_B" .. bver
		end
	end
end

function rsp(cmd,success,response,intermediate)
	if cmd == "AT+WISN?" then
		sn = intermediate
	elseif cmd == "AT+VER" then
		ver = intermediate
	end
end

local function urc(data,prefix)
	if prefix == "+CPIN" then
		if data == "+CPIN: READY" then
			sys.dispatch("SIM_IND","RDY")
		elseif data == "+CPIN: NOT INSERTED" then
			sys.dispatch("SIM_IND","NIST")
		end
	end
end

ril.regrsp("+WISN",rsp)
ril.regrsp("+VER",rsp)

ril.regurc("+CPIN",urc)

req("AT+WISN?")
req("AT+VER")
startclktimer()