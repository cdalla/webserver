#!/usr/bin/python3

import cgi
import cgitb
import sys

cgitb.enable(display=0, logdir="/Users/naomisterk/codam/webbieservie/www/cgi-logs") # Enable exception handler, if error occurs detailed report will be saved to the directory cgi-logs

# with open(sys.argv[1], 'r') as f:
# 	file = f.read()
# f.close

# print(file)

form = cgi.FieldStorage()

name = form.getvalue("name")

print(name)