#!/usr/bin/python3

import cgi
import cgitb
import sys

# Had this in here so I could output the environment variables (with os.environ). Not using it
# right now but may be useful for debugging later on so keeping the library included for now
import os

cgitb.enable(display=0, logdir="/Users/naomisterk/codam/webbieservie/www/cgi-logs") # Enable exception handler, if error occurs detailed report will be saved to the directory cgi-logs

form = cgi.FieldStorage()

if "name" in form:
	user_name = form["name"].value
	print(user_name, end='')
else:
	print(form)

# Rght now this is still in there but not sure it's necessary. Put this in there when I wasn't able
# to read the output correctly but this may not have been where the problem was. So gotta test
# that by removing this line below and seeing if it still works.
print(os.environ, file=sys.stderr)
sys.stdout.flush()
