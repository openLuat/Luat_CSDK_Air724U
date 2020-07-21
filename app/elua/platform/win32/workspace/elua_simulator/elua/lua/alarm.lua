-- 警报处理
require"sys"
require"cc"
require"sms"
require"audio"
require"nvm"
module(...,package.seeall)

local MAX_TTS_TIMES = 3
local CALL_INTERVAL = 5000 -- 报警电话增加间隔时间,否则无法来电撤防

local alarms = {}
local state = "IDLE"
local text = ""
local index = 0
local repeats = 0
local callnum = ""
local stopmode = ""
local ttsrepeats = 0
local ttsindex = 0

local function playtts()
	if state ~= "ALARM_CALL" then return end

	local content

	if type(text) == "table" then
		ttsindex = ttsindex+1
		if ttsindex > #text then
			ttsrepeats = ttsrepeats+1
			ttsindex = 1
		end
		content = text[ttsindex]
	else
		ttsrepeats = ttsrepeats+1
		content = text
	end

	if ttsrepeats <= MAX_TTS_TIMES then
		audio.playtts(common.binstohexs(common.gb2312toucs2(content)),"net")
	end
end

local function ttsplayend()
	sys.timer_start(playtts,500) --500ms延时播放tts,避免堵塞系统
end

local function stoptts()
	sys.timer_stop(playtts)
	audio.stoptts()
end

local function dial()
	if state ~= "ALARM_CALL" then return end

	cc.dial(callnum)
end

local function docall()
	sys.timer_start(dial,CALL_INTERVAL)
end

local function makecall()
	repeats = nvm.get("callrepeat")
	local num,mode
	while true do
		index = index + 1
		if index > 5 then
			stop("alarm")
			defense.alarmstop("alarmend")
			return
		end

		num = nvm.get("alarmnum",index)
		mode = nvm.get("alarmode",index)

		if (mode == 0 or mode == 2) and num ~= nil and num ~= "" then
			callnum = num
			docall()
			break
		end
	end
end

local function connected()
	stopmode = ""
	ttsrepeats = 0
	ttsindex = 0
	playtts()
end

local function disconnected()
	stoptts()

	if state == "IDLE" then return end

	if stopmode == "current" then
		stopmode = ""
		makecall()
		return
	elseif stopmode == "all" then
		stop("alarm")
		defense.alarmstop("alarmend")
		return
	end

	stopmode = ""

	if nvm.get("callrepeat") == 0 then
		docall()
	else
		repeats = repeats - 1
		if repeats > 0 then
			docall()
		else
			makecall()
		end
	end
end

local function dtmfdet(str)
	if str == "8" then
		stopmode = "current"
		cc.hangup()
	elseif str == "9" then
		stopmode = "all"
		cc.hangup()
	end
end

local ccapp =
{
	CALL_CONNECTED = connected,
	CALL_DISCONNECTED = disconnected,
	AUDIO_PLAY_END_IND = ttsplayend,
	AUDIO_DTMF_DETECT = dtmfdet,
}

local function gotocall()
	state = "ALARM_CALL"
	index = 0
	audio.dtmfdetect(true)
	sys.regapp(ccapp)
	makecall()
end

local function sendcnf(id,result)
	if state == "IDLE" then return end

	if result == true and state == "ALARM_SMS" then
		local num,mode
		while true do
			index = index + 1
			if index > 5 then
				gotocall()
				break
			end

			num = nvm.get("alarmnum",index)
			mode = nvm.get("alarmode",index)

			if (mode == 0 or mode == 1) and num ~= nil and num ~= "" then
				local outext = type(text) == "table" and table.concat(text) or text

				while true do
					if outext == "" then
						break
					end

					if string.len(outext) > 70*2 then
						sms.send(num,common.binstohexs(common.gb2312toucs2be(string.sub(outext,1,70*2))))
						outext = string.sub(outext,70*2+1,-1)
					else
						sms.send(num,common.binstohexs(common.gb2312toucs2be(outext)))
						outext = ""
					end
				end
				break
			end
		end
	elseif state ~= "ALARM_CALL" then
		gotocall()
	end
end

local function alarm(defnum)
	print("alarm:",state,type(defnum) == "table" and table.concat(defnum) or defnum)

	if state ~= "IDLE" then
		return
	end

	if type(defnum) == "table" then
		text = {}
		for k,v in ipairs(defnum) do
			table.insert(text,nvm.get("alarmtext",v))
		end
	else
		text = nvm.get("alarmtext",defnum)
	end

	if text == nil or text == "" then
		print("nil alarm text",text)
		return
	end

	sys.regapp(sendcnf,"SMS_SEND_CNF")
	state = "ALARM_SMS"
	stopmode = ""
	index = 0
	sendcnf("SMS_SEND_CNF",true)
end

function start(defnum)
	table.insert(alarms,defnum)

	if #alarms == 1 then alarm(defnum) end
end

local function gonext()
	if state ~= "NEXT_ALARM" then print("state not NEXT_ALARM") stop("alarm") return end

	if #alarms == 0 then print("no alarm") stop("alarm") return end

	state = "IDLE"

	alarm(alarms[1])
end

function stop(user)
	if state == "IDLE" then return end
	sys.timer_stop(docall)
	sys.timer_stop(gonext)
	stopmode = ""
	state = "IDLE"
	text = ""
	index = 0
	sys.deregapp(sendcnf)
	sys.deregapp(ccapp)
	if user ~= "alarm" then -- 内部停止,全部结束不需要挂断
		cc.hangup()
		stoptts()
	end

	if user == "disarm" then
		alarms = {}
	else
		table.remove(alarms,1)
		-- 继续下一个缓冲的报警防区
		if #alarms > 0 then
			state = "NEXT_ALARM"
			-- 2秒后开始下一轮报警,以待网络资源释放
			sys.timer_start(gonext,2000)
		end
	end
end

