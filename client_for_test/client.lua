--[[-------------------------------------------------------------------------
	> File Name: client.lua
	> Author: daixj
	> Mail: chtomato@163.com
	> Created Time: 2016年03月10日 星期四 22时00分32秒
 ------------------------------------------------------------------------]]
local socket = require("socket");
host = "127.0.0.1";
port = "8090";
client = assert(socket.connect(host,port));
client:settimeout(0);
client:send("hello \r\n lua \r\n");
local recvt, sendt, status = socket.select({client},nil,1);
while #recvt > 0 do
	local response, receive_status = client:receive();
	if receive_status ~= "closed" then
		if response then
			print(response);
			recvt,sendt, status = socket.select({client},nil,1);
		end
	else 
		break;
	end
end
