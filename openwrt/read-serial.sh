#!/usr/bin/lua

local function main()
	rserial=io.open("/dev/ttyUSB0","r")
	while true do
		line = nil
		while line==nil do
			line=rserial:read()
			rserial:flush()
			res=io.open("/var/ardres","w")
			res:write(line .. "\n")
			res:flush()
			res:close()
			line = nil
		end
	end
end

main()
