#!/usr/bin/env python
import cgi
import time

print("Content-type: text/html\r\n\r\n")
print('<html>')
print('<head>')
print('<title>HI</title>')
print('</head>')
print('<body>')

form = cgi.FieldStorage()

print("<b>time : " + str(time.time()) + "</b><br>")

if "data" not in form:
    print("<b>data query is not exist!</b>")
else:
    print("<b>data : " + form["data"].value + "</b>")

print('</body>')
print('</html>')