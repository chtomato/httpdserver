#!/usr/local/bin/lua
function header()
	print("Content-Type: text/html");
	print("");
end
header()
print(os.date("%c"))
