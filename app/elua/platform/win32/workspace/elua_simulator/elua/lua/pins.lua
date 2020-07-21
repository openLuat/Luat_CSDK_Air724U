
-- gpio 配置
require"sys"
module(...,package.seeall)

local charger = false

local function getcharger()
	charger = pmd.charger()
	return charger
end

local function setled2(bval,p)
	local valid = p.valid == 0 and 0 or 1 -- 默认高有效
	local notvalid = p.valid == 0 and 1 or 0
	p.val = bval == true and valid or notvalid

	pmd.ldoset(p.val,pmd.LDO_VASW)
end

local function pmdmsg(msg)
	if msg.charger ~= charger then
		charger = msg.charger
		sys.dispatch("PIN_EPOWER_IND",msg.charger)
	end
end
sys.regmsg(rtos.MSG_PMD,pmdmsg)

def = {
WATCHDOG		= {pin=pio.P0_9,init=false},
LR_LIGHT_EN		= {pin=pio.P0_14   ,},
TRUNK_OUT_EN	= {pin=pio.P0_15   ,},
ALERT_EN		= {pin=pio.P0_12   ,},
LOCK1_EN		= {pin=pio.P0_17   ,},
LOCK2_EN		= {pin=pio.P0_11   ,},
TRUNK_IN_INT	= {pin=pio.P0_6    ,dir=pio.INT,valid=1},
BRAKE_IN_INT	= {pin=pio.P0_3    ,dir=pio.INT,valid=0},
SIDE_IN_INT		= {pin=pio.P0_7    ,dir=pio.INT,valid=1},
ACC_IN_INT		= {pin=pio.P0_5    ,dir=pio.INT,valid=0},
OIL_EN			= {pin=pio.P0_16   ,},
ENGINE_EN		= {pin=pio.P0_8    ,},
PLOUGH_ALARM	= {pin=pio.P0_21   ,dir=pio.INPUT,valid=0},
SHORT_ALARM		= {pin=pio.P0_10   ,dir=pio.INPUT,valid=0},
VOLT_CHOICE		= {pin=pio.P0_20   ,},
SHAKE_IN_INT	= {pin=pio.P0_1    ,dir=pio.INT,valid=1},
LED1_EN			= {pin=pio.P0_24   ,},
LED2_EN			= {pin=pmd.LDO_VASW,ptype="LDO",set=setled2,},
EPOWER			= {ptype="CHARGER",val=false,get=getcharger,},
}
