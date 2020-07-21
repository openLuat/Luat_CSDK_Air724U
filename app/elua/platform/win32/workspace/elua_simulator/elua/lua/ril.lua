
-- 定义模块,导入依赖库
local base = _G
local table = require"table"
local string = require"string"
local uart = require"uart"
local rtos = require"rtos"
local sys = require"sys"
module("ril")

--加载常用的全局函数至本地
local setmetatable = base.setmetatable
local print = base.print
local type = base.type
local smatch = string.match
local sfind = string.find
local vwrite = uart.write
local vread = uart.read

-- 常量
local TIMEOUT = 60000 --1分钟无反馈 判定at命令执行失败
-- cmd type: 0:no reuslt 1:number 2:sline 3:mline 4:string 10:spec
local NORESULT = 0
local NUMBERIC = 1
local SLINE = 2
local MLINE = 3
local STRING = 4
local SPECIAL = 10
local RILCMD = {
	["+CSQ"] = 2,
	["+CGSN"] = 1,
	["+WISN"] = 1,
	["+CIMI"] = 1,
	["+CGATT"] = 2,
	["+CCLK"] = 2,
	["+VER"] = 4,
	["+ATWMFT"] = 4,
	["+CMGR"] = 3,
	["+CMGS"] = 2,
	["+CPBF"] = 3,
	["+CPBR"] = 3,
 	["+CIPSEND"] = 10,
	["+CIPCLOSE"] = 10,
	["+CIFSR"] = 10,
}

-- local var
local radioready = false

-- 命令队列
local cmdqueue = {
	"ATE0",
	"AT+CMEE=0",
}
-- 当前正在执行的命令,参数,反馈回调,命令头,类型
local currcmd,currarg,currsp,cmdhead,cmdtype
-- 反馈结果,中间信息,结果信息
local result,interdata,respdata

-- ril会出现三种情况: 命令回复\主动上报\命令超时
-- 超时
local function atimeout()
	rtos.restart() -- 命令响应超时自动重启系统
end

-- 命令回复
local function defrsp(cmd,success,response,intermediate)
	print("default response:",cmd,success,response,intermediate)
end

local rsptable = {}
setmetatable(rsptable,{__index = function() return defrsp end})
function regrsp(head,fnc,typ)
	if typ == nil then
		rsptable[head] = fnc
		return true
	end	
	if typ == 0 or typ == 1 or typ == 2 or typ == 3 or typ == 4 or typ == 10 then
		if RILCMD[head] and RILCMD[head] ~= typ then
			return false
		end
		RILCMD[head] = typ
		rsptable[head] = fnc
		return true
	else
		return false
	end
end

local function rsp()
	sys.timer_stop(atimeout)

	if currsp then
		currsp(currcmd,result,respdata,interdata)
	else
		rsptable[cmdhead](currcmd,result,respdata,interdata)
	end

	currcmd,currarg,currsp,cmdhead,cmdtype = nil
	result,interdata,respdata = nil
end

-- 主动上报提示
local function defurc(data)
	print("defurc:",data)
end

local urctable = {}
setmetatable(urctable,{__index = function() return defurc end})
function regurc(prefix,handler)
	urctable[prefix] = handler
end

function deregurc(prefix)
	urctable[prefix] = nil
end

local urcfilter

local function urc(data)
	if data == "RDY" then
		radioready = true
	else
		local prefix = smatch(data,"(%+*[%u%d ]+)")

		urcfilter = urctable[prefix](data,prefix)
	end
end

local function procatc(data)
	print("atc:",data)

	if interdata and cmdtype == MLINE then -- 继续接收多行反馈直至出现OK为止
		-- 多行反馈的命令如果接收到中间数据说明执行成功了,判定之后的数据结束就是OK
		if data ~= "OK\r\n" then
			if sfind(data,"\r\n",-2) then -- 去掉最后的换行符
				data = string.sub(data,1,-3)
			end
			interdata = interdata .. "\r\n" .. data
			return
		end
	end

	if urcfilter then
		data,urcfilter = urcfilter(data)
	end

	if sfind(data,"\r\n",-2) then -- 若最后两个字节是\r\n则删掉
		data = string.sub(data,1,-3)
	end

	if data == "" then
		return
	end

	if currcmd == nil then -- 当前无命令在执行则判定为urc
		urc(data)
		return
	end

	local isurc = false
	
	if string.find(data,"^%+CMS ERROR:") or string.find(data,"^%+CME ERROR:") then
		data = "ERROR"
	end

	if data == "OK" or data == "SHUT OK" then
		result = true
		respdata = data
	elseif data == "ERROR" or data == "NO ANSWER" or data == "NO DIALTONE" then
		result = false
		respdata = data
	elseif data == "> " then
		if cmdhead == "+CMGS" then -- 根据提示符发送短信或者数据
			print("send:",currarg)
			vwrite(uart.ATC,currarg,"\026")
		elseif cmdhead == "+CIPSEND" then
			print("send:",currarg)
			vwrite(uart.ATC,currarg)
		else
			print("error promot cmd:",currcmd)
		end
	else
		--根据命令类型来判断收到的数据是urc或者反馈数据
		if cmdtype == NORESULT then -- 无结果命令 此时收到的数据只有URC
			isurc = true
		elseif cmdtype == NUMBERIC then -- 全数字
			local numstr = smatch(data,"(%d+)")
			if numstr == data then
				interdata = data
			else
				isurc = true
			end
		elseif cmdtype == STRING then -- 字符串
			local str = smatch(data,"(.+)")
			interdata = data
		elseif cmdtype == SLINE or cmdtype == MLINE then
			if interdata == nil and sfind(data, cmdhead) == 1 then
				interdata = data
			else
				isurc = true
			end
		elseif cmdhead == "+CIFSR" then
			local s = smatch(data,"%d+%.%d+%.%d+%.%d+")
			if s ~= nil then
				interdata = s
				result = true
			else
				isurc = true
			end
		elseif cmdhead == "+CIPSEND" or cmdhead == "+CIPCLOSE" then
			local keystr = cmdhead == "+CIPSEND" and "SEND" or "CLOSE"
			local lid,res = smatch(data,"(%d), *([%u%d :]+)")

			if lid and res then
				if sfind(res,keystr) == 1 or sfind(res,"TCP ERROR") == 1 or sfind(res,"UDP ERROR") == 1 then
					result = true
					respdata = data
				else
					isurc = true
				end
			else
				isurc = true
			end
		else
			isurc = true
		end
	end

	if isurc then
		urc(data)
	elseif result ~= nil then
		rsp()
	end
end

local readat = false

local function getcmd(item)
	local cmd,arg,rsp

	if type(item) == "string" then
		cmd = item
	elseif type(item) == "table" then
		cmd = item.cmd
		arg = item.arg
		rsp = item.rsp
	else
		print("getpack unknown item")
		return
	end

	head = smatch(cmd,"AT([%+%*]*%u+)")

	if head == nil then
		print("request error cmd:",cmd)
		return
	end

	if head == "+CMGS" or head == "+CIPSEND" then -- 必须有参数
		if arg == nil or arg == "" then
			print("request error no arg",head)
			return
		end
	end

	currcmd = cmd
	currarg = arg
	currsp = rsp
	cmdhead = head
	cmdtype = RILCMD[head] or NORESULT

	return currcmd
end

local function sendat()
	if not radioready or readat or currcmd ~= nil then
		-- 未初始化/正在读取atc数据、有命令在执行、队列无命令 直接退出
		return
	end

	local item

	while true do
		if #cmdqueue == 0 then
			return
		end

		item = table.remove(cmdqueue,1)

		getcmd(item)

		if currcmd ~= nil then
			break
		end
	end

	sys.timer_start(atimeout,TIMEOUT)

	print("sendat:",currcmd)

	vwrite(uart.ATC,currcmd .. "\r")
end

local function atcreader()
	local s

	readat = true
	while true do
		s = vread(uart.ATC,"*l",0)

		if string.len(s) ~= 0 then
			procatc(s)
		else
			break
		end
	end
	readat = false
	sendat() -- atc上报数据处理完以后才执行发送AT命令
end

sys.regmsg("atc",atcreader)

function request(cmd,arg,onrsp)
	--插入缓冲队列
	if not arg and not onrsp then
		table.insert(cmdqueue,cmd)
	else
		table.insert(cmdqueue,{cmd = cmd,arg = arg,rsp = onrsp})
	end

	sendat()
end
