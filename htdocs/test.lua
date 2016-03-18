#!/usr/bin/lua
--如果是在ubuntu上设置为#!/usr/bin/lua
function header( ... )
 	print("Content-Type: text/html")
	print("")
end 
header()
print ("<html>")
print ("<head>")
print ("<title>test</title>")
print ("</head>")
print ("<body>")
print ("hello lua")
print ("</body>")
print ("</html>")
