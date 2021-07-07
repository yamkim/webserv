#!/usr/bin/env python
import cgi
import time

print("Content-type: text/html")
print("")
print('<html>')
print('<head>')
print('<title>Python CGI Page</title>')
print('</head>')
print('<body>')

form = cgi.FieldStorage()

print("<b>time : " + str(time.time()) + "</b><br>")

if "data" not in form:
    print("<b>data query is not exist!</b>")
else:
    print("<b>data : " + form["data"].value + "</b>")

for i in range(1, 10):
    print("<b>")
    print(i)
    print("</b>")
    print("<br>")
    time.sleep(0.01)

print('</body>')
print('</html>')