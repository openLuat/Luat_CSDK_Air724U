local base = _G
local string = require"string"
local io = require"io"
local rtos = require"rtos"
local audio = require"audiocore"
local sys = require"sys"
local ril = require"ril"
module("audio")

local smatch = string.match
local print = base.print
local dispatch = sys.dispatch
local req = ril.request
local tonumber = base.tonumber

local speakervol,audiochannel,microphonemode,microphonevol
local tmp1,tmp2
local tmpchannel,tmpspeakervol

-- dtmf检测 参数: 使能,[灵敏度 默认2 最灵敏为1]
function dtmfdetect(enable,sens)
	if enable == true then
		if sens == 1 then
			req("AT+DTMFDET=2,1,1")
		else
			req("AT+DTMFDET=2,1,2")
		end
	end

	req("AT+DTMFDET="..(enable and 1 or 0))
end

function senddtmf(str,playtime,intvl)
	if string.match(str,"([%dABCD%*#]+)") ~= str then
		print("senddtmf: illegal string "..str)
		return false
	end

	playtime = playtime and playtime or 100
	intvl = intvl and intvl or 100

	req("AT+SENDSOUND="..string.format("\"%s\",%d,%d",str,playtime,intvl))
end

-- text = "123" = 310032003300
-- path = "net" 发给网络  "speaker" 本地播放
function playtts(text,path)
	local action = path == "net" and 4 or 2

	req("AT+QTTS=1")
	req(string.format("AT+QTTS=%d,\"%s\"",action,text))
end

function stoptts()
	req("AT+QTTS=3")
end

-- 通话中发送声音到对端,必须是12.2K AMR格式
function transvoice(data,loop)
	local f = io.open("/RecDir/rec000","wb")

	if f == nil then
		print("transvoice:open file error")
		return false
	end

	-- 有文件头并且是12.2K帧
	if string.sub(data,1,7) == "#!AMR\010\060" then
	-- 无文件头且是12.2K帧
	elseif string.byte(data,1) == 0x3C then
		f:write("#!AMR\010")
	else
		print("transvoice:must be 12.2K AMR")
		return false
	end

	f:write(data)
	f:close()

	req(string.format("AT+AUDREC=0,%d,2,0,50000",loop == true and 1 or 0))
	return true
end

-- 音频播放接口
function play(name)
	return audio.play(name)
end

function stop()
	return audio.stop()
end

local dtmfnum = {[71] = "Hz1000",[69] = "Hz1400",[70] = "Hz2300"}

local function parsedtmfnum(data)
	local n = base.tonumber(string.match(data,"(%d+)"))
	local dtmf

	if (n >= 48 and n <= 57) or (n >=65 and n <= 68) or n == 42 or n == 35 then
		dtmf = string.char(n)
	else
		dtmf = dtmfnum[n]
	end

	if dtmf then
		dispatch("AUDIO_DTMF_DETECT",dtmf)
	end
end

local function audiourc(data,prefix)
	if prefix == "+DTMFDET" then
		parsedtmfnum(data)
	elseif prefix == "+AUDREC" then
		local duration = string.match(data,": *%d,(%d+)",string.len(prefix)+1)

		if base.tonumber(duration) > 0 then
			dispatch("AUDIO_PLAY_END_IND")
		else
			dispatch("AUDIO_PLAY_ERROR_IND")
		end
	elseif prefix == "+QTTS" then
		local flag = string.match(data,": *(%d)",string.len(prefix)+1)
		if flag == "0" then
			dispatch("AUDIO_PLAY_END_IND")
		end
	end
end

local function audiorsp(cmd,success,response,intermediate)
	local prefix = smatch(cmd,"AT(%+%u+%?*)")

	if prefix == "+CLVL?" then
		if intermediate then
			local cvol = smatch(intermediate,"+CLVL%s*:%s*(%d+)")
			if cvol ~= nil and cvol ~= "" and success then
				speakervol = tonumber(cvol)
			end
		end				
	elseif prefix == "+CLVL" then
		if success then
			speakervol = tonumber(tmpspeakervol)
		end	    
		dispatch("SPEAKER_VOLUME_SET_CNF",success)
	elseif prefix == "+CHFA?" then
		if intermediate then
			local channel = smatch(intermediate,"+CHFA%s*:%s*(%d+)")
			if channel ~= nil and channel ~= "" and success then
				audiochannel = tonumber(channel)
			end
		end		
	elseif prefix == "+CHFA" then
		if success then
			audiochannel = tonumber(tmpchannel)
		end
		dispatch("AUDIO_CHANNEL_SET_CNF",success)
	elseif prefix == "+CMIC?" then
		if intermediate then
			local mode,vol = smatch(intermediate,"+CMIC%s*:%s*(%d),(%d)")
			if mode ~= "" and vol ~= "" and mode ~= nil and vol ~= nil and success then
				microphonemode = tonumber(mode)
				microphonevol = tonumber(vol)
			end
		end		
	elseif prefix == "+CMIC" then
		if success then
			microphonemode = tonumber(tmp1)
			microphonevol = tonumber(tmp2)
		end
		dispatch("MICROPHONE_GAIN_SET_CNF",success)
	end
end

ril.regurc("+DTMFDET",audiourc)
ril.regurc("+AUDREC",audiourc)
ril.regurc("+QTTS",audiourc)
ril.regrsp("+CLVL?",audiorsp,2)
ril.regrsp("+CLVL",audiorsp,0)
ril.regrsp("+CHFA?",audiorsp,2)
ril.regrsp("+CHFA",audiorsp,0)
ril.regrsp("+CMIC?",audiorsp,2)
ril.regrsp("+CMIC",audiorsp,0)

function setspeakervol(vol)
    req("AT+CLVL=" .. vol)
	tmpspeakervol = vol
end

function getspeakervol()
	return speakervol
end

function setaudiochannel(channel)
    req("AT+CHFA=" .. channel)
	tmpchannel = channel
end

function getaudiochannel()
	return audiochannel
end

function setmicrophonegain(mode,vol)
    req("AT+CMIC=" .. mode .. "," .. vol)
end

function getmicrophonegain()
	return microphonemode,microphonevol
end

local function audiomsg(msg)
	if msg.play_end_ind == true then
		dispatch("AUDIO_PLAY_END_IND")
	elseif msg.play_error_ind == true then
		dispatch("AUDIO_PLAY_ERROR_IND")
	end
end

sys.regmsg(rtos.MSG_AUDIO,audiomsg)
req("AT+CLVL?")
req("AT+CHFA?")
req("AT+CMIC?")
