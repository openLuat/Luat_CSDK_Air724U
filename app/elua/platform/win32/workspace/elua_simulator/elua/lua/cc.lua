local base = _G
local string = require"string"
local sys = require"sys"
local ril = require"ril"
local net = require"net"
local pm = require"pm"
local aud = require"audio"

module("cc")

local ipairs = base.ipairs
local dispatch = sys.dispatch
local req = ril.request

--local
local ccready = false

local emergency_num = {"112", "911", "000", "08", "110", "119", "118", "999"}

function isemergencynum(num)
	for k,v in ipairs(emergency_num) do
		if v == num then
			return true
		end
	end
	return false
end

local function discevt(reason)
	pm.sleep("cc")
	dispatch("CALL_DISCONNECTED",reason)
end

function dial(number)
	if number == "" or number == nil then
		return false
	end

	if ccready == false and not isemergencynum(number) then
		return false
	end

	pm.wake("cc")
	req(string.format("%s%s;","ATD",number))

	return true
end

function hangup()
	aud.stop()
	req("AT+CHUP")
end

function accept()
	aud.stop()
	req("ATA")
	pm.wake("cc")
end

local function ccurc(data,prefix)
	if data == "CALL READY" then
		ccready = true
		dispatch("CALL_READY")
	elseif data == "CONNECT" then
		dispatch("CALL_CONNECTED")
	elseif data == "NO CARRIER" or data == "BUSY" or data == "NO ANSWER" then
		discevt(data)
	elseif prefix == "+CLIP" then
		local number = string.match(data,"\"(%d*)\"",string.len(prefix)+1)
		dispatch("CALL_INCOMING",number)
	end
end

local function ccrsp(cmd,success,response,intermediate)
	local prefix = string.match(cmd,"AT(%+*%u+)")
	if prefix == "D" then
		if not success then
			discevt("CALL_FAILED")
		end
	elseif prefix == "+CHUP" then
		discevt("LOCAL_HANG_UP")
	elseif prefix == "A" then
		dispatch("CALL_CONNECTED")
	end
end

-- urc
ril.regurc("CALL READY",ccurc)
ril.regurc("CONNECT",ccurc)
ril.regurc("NO CARRIER",ccurc)
ril.regurc("NO ANSWER",ccurc)
ril.regurc("BUSY",ccurc)
ril.regurc("+CLIP",ccurc)
-- rsp
ril.regrsp("D",ccrsp)
ril.regrsp("A",ccrsp)
ril.regrsp("+CHUP",ccrsp)

--cc config
req("ATX4") --ø™∆Ù≤¶∫≈“Ù,√¶“ÙºÏ≤‚
req("AT+CLIP=1")
