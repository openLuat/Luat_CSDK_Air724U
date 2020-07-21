-- 电源管理 power manage
local base = _G
local pmd = require"pmd"
local pairs = base.pairs
module("pm")

local tags = {}
local flag = true

function wake(tag)
	id = tag or "default"

	tags[id] = tags[id] and tags[id]+1 or 1

	if flag == true then
		flag = false
		pmd.sleep(0)
	end
end

function sleep(tag)
	id = tag or "default"

	tags[id] = tags[id] and tags[id]-1 or 0

	if tags[id] < 0 then
		base.print("pm.sleep:error",tag)
		tags[id] = 0
	end

	-- 只要存在任何一个模块唤醒,则不睡眠
	for k,v in pairs(tags) do
		if v > 0 then
			return
		end
	end

	flag = true
	pmd.sleep(1)
end
