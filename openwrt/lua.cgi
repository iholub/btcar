#!/usr/bin/lua
local function main()
    print("Content-Type: text/json")
    print("")
    qs = os.getenv'QUERY_STRING'
	i, j = string.find(qs, "&")
	cmd = string.sub(qs, 1, j - 1)
	wserial=io.open("/dev/ttyUSB0","w")
  	wserial:write(cmd)
	wserial:flush()
	wserial:close()
	res=io.open("/var/ardres","r")
	while line==nil do
		line=res:read()
	end
	res:flush()
	res:close()
	ok, err = os.remove ("/var/ardres")
	print("{")
        print("qs: " .. qs .. ",")
	print("cmd: " .. cmd .. ",")
	print("line: " .. line)
	print("}")

end

main()