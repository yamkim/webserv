#!/usr/bin/env python
import cgi
import time

print("Content-type: text/html\r\n\r\n")
print('<html>')
print('<head>')
print('<title>HI</title>')
print('</head>')
print('<body>')

print("<b>time : " + str(time.time()) + "</b><br>")

print('</body>')
print('</html>')