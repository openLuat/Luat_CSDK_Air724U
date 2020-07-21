-- gpio操作接口
require"pins"
module(...,package.seeall)

function pinget(p)
	if p.get then return p.get(p) end

	if p.dir == pio.INT then
		return p.val
	else
		return pio.pin.getval(p.pin) == p.valid
	end
end

function pinset(bval,p)
	p.val = bval

	if p.set then p.set(bval,p) return end

	if p.ptype ~= nil and p.ptype ~= "GPIO" then print("unknwon pin type:",p.ptype) return end

	local valid = p.valid == 0 and 0 or 1 -- 默认高有效
	local notvalid = p.valid == 0 and 1 or 0
	local val = bval == true and valid or notvalid

	if p.pin then pio.pin.setval(val,p.pin) end
end

local function init()
	for k,v in pairs(pins.def) do
		if v.init == false then
			-- 不做初始化
		elseif v.ptype == nil or v.ptype == "GPIO" then
			pio.pin.setdir(v.dir or pio.OUTPUT,v.pin)
			if v.dir == nil or v.dir == pio.OUTPUT then
				pinset(v.defval or false,v)
			elseif v.dir == pio.INTPUT or v.dir == pio.INT then
				v.val = pio.pin.getval(v.pin) == v.valid
			end
		elseif v.set then
			pinset(v.defval or false,v)
		end
	end
end

local function intmsg(msg)
	local status = 0

	if msg.int_id == cpu.INT_GPIO_POSEDGE then status = 1 end

	for k,v in pairs(pins.def) do
		if v.dir == pio.INT and msg.int_resnum == v.pin then
			v.val = v.valid == status
			sys.dispatch(string.format("PIN_%s_IND",string.match(k,"(%w+)")),v.valid == status)
			return
		end
	end
end
sys.regmsg(rtos.MSG_INT,intmsg)

init()
