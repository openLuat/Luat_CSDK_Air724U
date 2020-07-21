
local base = _G
local string = require "string"
local table = require "table"
local sys = require "sys"
local ril = require "ril"
local common = require "common"
module("sms")

local print = base.print
local tonumber = base.tonumber
local dispatch = sys.dispatch
local req = ril.request

local ready = false

function send(num,data)
	if not ready then return false end
	req(string.format("%s\"%s\"","AT+CMGS=",common.nstrToUcs2Hex(num)),data)
	return true
end

function read(pos)
	if not ready then return false end
	req("AT+CMGR="..pos)
	return true
end

function delete(pos)
	if not ready then return false end
	req("AT+CMGD="..pos)
	return true
end

local function rsp(cmd,success,response,intermediate)
	local prefix = string.match(cmd,"AT(%+%u+)")

	if prefix == "+CMGR" then
		local num,name,t,data
		if intermediate then
			num,name,t,data = string.match(intermediate,"+CMGR: \"[%u ]+\",\"([B%d]+)\",\"?(%x*)\"?,\"([^\"]+)\"\r\n(%x+)")
		end
		local pos = string.match(cmd,"AT%+CMGR=(%d+)")
		num = num and common.ucs2toascii(num) or ""
		data = data or ""
		t = t or ""
		name = name or ""
		dispatch("SMS_READ_CNF",success,num,data,pos,t,name)
	elseif prefix == "+CMGD" then
		dispatch("SMS_DELETE_CNF",success)
	elseif prefix == "+CMGS" then
		dispatch("SMS_SEND_CNF",success)
	end
end

local function urc(data,prefix)
	if data == "SMS READY" then
		ready = true
		req("AT+CSMP=17,167,0,8")
		req("AT+CSCS=\"UCS2\"")
		dispatch("SMS_READY")
	elseif prefix == "+CMTI" then
		local pos = string.match(data,"(%d+)",string.len(prefix)+1)
		dispatch("SMS_NEW_MSG_IND",pos)
	end
end

function getsmsstate()
	return ready
end

ril.regurc("SMS READY",urc)
ril.regurc("+CMT",urc)
ril.regurc("+CMTI",urc)

ril.regrsp("+CMGR",rsp)
ril.regrsp("+CMGD",rsp)
ril.regrsp("+CMGS",rsp)

--默认上报新短信存储位置
--req("AT+CNMI=2,1")
--使用text模式发送
req("AT+CMGF=1")
