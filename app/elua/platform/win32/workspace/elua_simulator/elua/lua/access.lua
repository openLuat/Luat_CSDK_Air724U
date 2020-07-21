-- PC编程处理
require"sys"
require"nvm"
require"pinop"
require"led"
module(...,package.seeall)

local PROGRAM_TIMEOUT=120000 -- 编程超时时间2分钟
local mode = 0

function inprogram()
	return mode > 0
end

local function setinstaller(id,s)
	if string.len(s) ~= 6 or string.match(s,"(%d+)") ~= s then
		print("setinstaller:invalid",s)
		return false
	end

	nvm.set("installer",s)
	return true
end

local function setalarmnum(id,s)
	if s ~= "" and (string.len(s) < 8 or string.len(s) > 16 or string.match(s,"(%d+)") ~= s) then
		print("setalarmnum:invalid",s)
		return false
	end

	nvm.set("alarmnum",id,s)
	return true
end

local function setopcode(id,s)
	if string.len(s) ~= 4 or string.match(s,"(%d+)") ~= s then
		print("setopcode:invalid",s)
		return false
	end

	nvm.set("opcode",s)
	return true
end

local function setadmin(id,s)
	if s ~= "" and (string.len(s) < 8 or string.len(s) > 16 or string.match(s,"(%d+)") ~= s) then
		print("setadmin:invalid",s)
		return false
	end

	nvm.set("admin",id-7+1,s)
	return true
end

local function setalarmode(id,s)
	if string.len(s) ~= 5 or string.match(s,"([012]+)") ~= s then
		print("setalarmode:invalid",s)
		return false
	end

	for i=1,string.len(s) do
		nvm.set("alarmode",i,tonumber(string.sub(s,i,i)))
	end

	return true
end

local function setcallrepeat(id,s)
	if string.len(s) ~= 2 or tonumber(s) == nil or tonumber(s) >= 10 then
		return false
	end

	nvm.set("callrepeat",tonumber(s))
	return true
end

local function getwdevnvid(cid)
	if cid < 50 then
		return cid < 24 and (cid-20+1) or (cid-24+1)
	else
		return cid < 74 and (cid-70+1) or (cid-74+1)
	end
end

local currcmd = nil

local function learnctrltimeout()
	currcmd = nil
	rmtctler.exitlearn()
	uart.write(3,"ERROR")
end

function ctrlerlearn(ctrlid)
	if currcmd and currcmd >= 20 and currcmd <= 28 then
		sys.timer_stop(learnctrltimeout)
		nvm.set(currcmd < 24 and "ctrler" or "wdef",getwdevnvid(currcmd),ctrlid)
		currcmd = nil
		uart.write(3,tostring(ctrlid))
	end
end

local function setctrler(id)
	-- 回复当前记录的遥控器身份码
	local code = nvm.get(id < 24 and "ctrler" or "wdef",getwdevnvid(id))
	uart.write(3,string.format("%dA%s",id,code >= 0x10000 and "-" or code))

	--　进入遥控器学习
	currcmd = id
	rmtctler.enterlearn()
	sys.timer_start(learnctrltimeout,15000)
	return "PROCEEDING"
end

local function setalarmtext(id,text)
	if text == "" then -- 不允许清除防区报警内容
		print("setalarmtext:invalid")
		return false
	end

	nvm.set("alarmtext",id-30+1,text)
	return true
end

local function setdef2valid(id,text)
	if text == "开通" or text == "屏蔽" then
		nvm.set("def2valid",text)
		return true
	end

	return false
end

local function setdefstat(id,text)
	if text == "开路" or text == "短路" then
		nvm.set(id == 36 and "def4stat" or "def5stat",text)
		return true
	end

	return false
end

local function getinstaller(id)
	uart.write(3,string.format("%dA%s",id,nvm.get("installer")))
end

local function getalarmnum(id)
	local str = nvm.get("alarmnum",id-51+1)
	uart.write(3,string.format("%dA%s",id,str == "" and "-" or str))
end

local function getopcode(id)
	uart.write(3,string.format("%dA%s",id,nvm.get("opcode")))
end

local function getadmin(id)
	local str = nvm.get("admin",id-57+1)
	uart.write(3,string.format("%dA%s",id,str == "" and "-" or str))
end

local function getalarmode(id)
	uart.write(3,string.format("%dA%s",id,table.concat(nvm.get("alarmode"))))
end

local function getcallrepeat(id)
	uart.write(3,string.format("%dA%s",id,nvm.get("callrepeat")))
end

local function getctrler(id)
	-- 清除遥控器
	nvm.set(id < 74 and "ctrler" or "wdef",getwdevnvid(id),0x10000)
	return true
end

local function getalarmtext(id)
	uart.write(3,string.format("%dA%s",id,nvm.get("alarmtext",id-80+1)))
end

local function getdef2valid(id)
	uart.write(3,string.format("%dA%s",id,nvm.get("def2valid")))
end

local function getdefstat(id,text)
	uart.write(3,string.format("%dA%s",id,nvm.get(id == 86 and "def4stat" or "def5stat")))
end

local cmds =
{
	[00] = setinstaller,
	[01] = setalarmnum,
	[02] = setalarmnum,
	[03] = setalarmnum,
	[04] = setalarmnum,
	[05] = setalarmnum,
	[06] = setopcode,
	[07] = setadmin,
	[08] = setadmin,
	[09] = setadmin,
	[10] = setalarmode,
	[11] = setcallrepeat,
	[20] = setctrler,
	[21] = setctrler,
	[22] = setctrler,
	[23] = setctrler,
	[24] = setctrler,
	[25] = setctrler,
	[26] = setctrler,
	[27] = setctrler,
	[28] = setctrler,
	[30] = setalarmtext,
	[31] = setalarmtext,
	[32] = setalarmtext,
	[33] = setalarmtext,
	[34] = setalarmtext,
	[35] = setdef2valid,
	[36] = setdefstat,
	[37] = setdefstat,

	[50] = getinstaller,
	[51] = getalarmnum,
	[52] = getalarmnum,
	[53] = getalarmnum,
	[54] = getalarmnum,
	[55] = getalarmnum,
	[56] = getopcode,
	[57] = getadmin,
	[58] = getadmin,
	[59] = getadmin,
	[60] = getalarmode,
	[61] = getcallrepeat,
	[70] = getctrler,
	[71] = getctrler,
	[72] = getctrler,
	[73] = getctrler,
	[74] = getctrler,
	[75] = getctrler,
	[76] = getctrler,
	[77] = getctrler,
	[78] = getctrler,
	[80] = getalarmtext,
	[81] = getalarmtext,
	[82] = getalarmtext,
	[83] = getalarmtext,
	[84] = getalarmtext,
	[85] = getdef2valid,
	[86] = getdefstat,
	[87] = getdefstat,
}

local function setalertdelay(id,s)
	if string.len(s) ~= 2 or string.match(s,"(%d+)") ~= s or tonumber(s) == 0 then
		print("setalertdelay:invalid",s)
		return false
	end

	nvm.set("alertdelaytime",tonumber(s)*1000)
	return true
end

local function setrmtcall(id,s)
	if string.len(s) ~= 3 or string.match(s,"(%d+)") ~= s then
		print("setrmtcall:invalid",s)
		return false
	end

	local callrepeat = tonumber(string.sub(s,1,1))
	local callinterval = tonumber(string.sub(s,2,3))

	if not (callrepeat >= 1 and callrepeat <= 9 and callinterval >= 10 and callinterval <= 50) then
		print("setrmtcall:invalid",s)
		return false
	end

	nvm.set("incalltimes",callrepeat)
	nvm.set("incallinterval",callinterval*1000)
	return true
end

local function setautoalertdelay(id,s)
	if string.len(s) ~= 3 or string.match(s,"(%d+)") ~= s or tonumber(s) < 20 then -- 至少要超过20秒
		print("setautoalertdelay:invalid",s)
		return false
	end

	nvm.set("delayautoalert",tonumber(s)*1000)
	return true
end

local function getalertdelay(id)
	local delay = nvm.get("alertdelaytime")/1000
	uart.write(3,string.format("%d%02d",id,delay))
end

local function getrmtcall(id)
	uart.write(3,string.format("%d%d%02d",id,nvm.get("incalltimes"),nvm.get("incallinterval")/1000))
end

local function getautoalertdelay(id)
	local delay = nvm.get("delayautoalert")/1000
	uart.write(3,string.format("%d%03d",id,delay))
end

local hidecmds =
{
	[17] = setautoalertdelay,
	[18] = setalertdelay,
	[19] = setrmtcall,
	[67] = getautoalertdelay,
	[68] = getalertdelay,
	[69] = getrmtcall,
}

local function timeout()
	mode = 0
	uart.write(3,"EXIT")
	led.work("programend")
end

local function parse(s)
	if s == "99" then
		uart.write(3,"OK")
		nvm.restore()
		return
	end

	if mode == 0 then
		if s == nvm.get("installer") or s == "737737" then
			mode = s == "737737" and 2 or 1
			uart.write(3,"OK")
			led.work("blink")
			sys.timer_start(timeout,PROGRAM_TIMEOUT)
		else
			uart.write(3,"ERROR")
		end
		return
	end

	if mode > 0 then
		if s == "*#" then
			mode = 0
			if currcmd then
				if currcmd >= 20 and currcmd <= 23 then
					sys.timer_stop(learnctrltimeout)
					rmtctler.exitlearn()
				end
				currcmd = nil
			end
			uart.write(3,"OK")
			sys.timer_stop(timeout)
			led.work("programend")
			return
		end
	end

	sys.timer_start(timeout,PROGRAM_TIMEOUT)

	if currcmd ~= nil then
		uart.write(3,"ERROR") -- 有其他命令在处理,不允许处理下一条命令
		return
	end

	local id = tonumber(string.sub(s,1,2))

	if id == nil then
		print("unknwon cmd:",s)
		return
	end

	local proc = cmds[id]

	if proc == nil and mode == 2 then
		proc = hidecmds[id]
	end

	if proc ~= nil then
		local result = proc(id,string.sub(s,3,-1))

		if result == "PROCEEDING" then
			currcmd = id
		elseif result == true then
			uart.write(3,"OK")
		elseif result == false then
			uart.write(3,"ERROR")
		end
	else
		uart.write(3,"ERROR")
	end
end

local function recv()
	local s

	while true do
		s = uart.read(3,"*l")

		if string.len(s) == 0 then
			break
		end

		parse(s)
	end
end
sys.reguart(3,recv)
uart.setup(3,921600,8,uart.PAR_NONE,uart.STOP_1,2)
