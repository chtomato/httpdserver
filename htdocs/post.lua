#!/usr/local/bin/lua
--如果是在ubuntu上设置为#!/usr/bin/lua
--mac:#!/usr/local/bin/lua
function header()
 	print("Content-Type: text/html");
	print("");
end
function getMethod()
	return os.getenv("REQUEST_METHOD");
end
function getContenLength()
	return os.getenv("CONTENT_LENGTH")
end
local method = getMethod()
local content_length = getContenLength()
header()
body_header=[[
<html>
<head>
<title>test</title>
<body>
]]
body_w = [[
</body>
</head>
</html>
]]
print(body_header.."method:"..method.."len:"..content_length..body_w)

