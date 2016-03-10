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
client:send("hello lua")
