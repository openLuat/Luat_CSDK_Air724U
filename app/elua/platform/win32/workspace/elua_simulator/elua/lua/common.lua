-- 通用处理库
module(...,package.seeall)

local tonumber = tonumber
local tinsert = table.insert
local ssub = string.sub
local sbyte = string.byte
local schar = string.char
local sformat = string.format

-- "0031003200330034" -> "1234"
function ucs2toascii(inum)
	local tonum = {}
	for i=1,string.len(inum),4 do
		table.insert(tonum,tonumber(string.sub(inum,i,i+3),16)%256)
	end

	return string.char(unpack(tonum))
end

-- "+1234" -> "002B0031003200330034"
function nstrToUcs2Hex(inum)
	local hexs = ""
	local elem = ""

	for i=1,string.len(inum) do
		elem = string.sub(inum,i,i)
		if elem == "+" then
			hexs = hexs .. "002B"
		else
			hexs = hexs .. "003" .. elem
		end
	end

	return hexs
end

function binstohexs(bins)
	local hexs = ""

	if bins == nil or type(bins) ~= "string" then return nil,"nil input string" end

	for i=1,string.len(bins) do
		hexs = hexs .. sformat("%02X",sbyte(bins,i))
	end
	hexs = string.upper(hexs)
	return hexs
end

function hexstobins(hexs)
	local tbins = {}
	local num

	if hexs == nil or type(hexs) ~= "string" then return nil,"nil input string" end

	for i=1,string.len(hexs),2 do
		num = tonumber(ssub(hexs,i,i+1),16)
		if num == nil then
			return nil,"error num index:" .. i .. ssub(hexs,i,i+1)
		end
		tinsert(tbins,num)
	end

	return schar(unpack(tbins))
end

function ucs2togb2312(ucs2s)
	local cd = iconv.open("gb2312","ucs2")
	return cd:iconv(ucs2s)
end

function gb2312toucs2(gb2312s)
	local cd = iconv.open("ucs2","gb2312")
	return cd:iconv(gb2312s)
end

function ucs2betogb2312(ucs2s)
	local cd = iconv.open("gb2312","ucs2be")
	return cd:iconv(ucs2s)
end

function gb2312toucs2be(gb2312s)
	local cd = iconv.open("ucs2be","gb2312")
	return cd:iconv(gb2312s)
end
