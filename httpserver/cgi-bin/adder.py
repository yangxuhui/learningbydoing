#!/usr/bin/python

import os

n1, n2 = 0, 0

if "QUERY_STRING" in os.environ:
    query_string = os.environ["QUERY_STRING"]
    args = query_string.split('&')
    n1, n2 = int(args[0]), int(args[1])

content = """
<html>
<head><title>Young Web Server</title></head>
<body>
<h1>Welcom to Young add application: </h1>
<p>The answer is : {a} + {b} = {c}</p>
<p>Thanks for visiting!</p>
</body>
</html>""".format(a = n1, b = n2, c = n1 + n2)

print "Connection: close"
print "Content-length: {0}".format(len(content))
print "Content-type: text/html\r\n"
print content
