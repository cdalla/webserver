#!/usr/bin/python3

import cgi
import cgitb
import sys
import os

# cgitb.enable()
cgitb.enable(display=0, logdir="/Users/naomisterk/codam/webbieservie/www/cgi-logs") # Enable exception handler, if error occurs detailed report will be saved to the directory cgi-logs

form = cgi.FieldStorage()
print(form, file=sys.stderr)

if "uploaded_file" in form:
	fileitem = form["uploaded_file"]
	print("OK")
else:
	print("NO")
	# print(form, file=sys.stderr)

# if not form.has_key('file'):
# 	print("Form doesnt have key file", file=sys.stderr)
# 	exit

# fileitem = form['uploaded_file']

# if not fileitem.file:
# 	print("There is no file data in form_file", file=sys.stderr)
# 	exit

# filedata = fileitem.file.read()

# with open('/Users/naomisterk/codam/webbieservie/www/uploads/test', 'w') as f:
# 	f.write(filedata)

# print("OK")

# if fileitem.filename:
# 	fn = os.path.basename(fileitem.filename)
# 	open('/Users/naomisterk/codam/webbieservie/www/uploads/test', 'w').write(fileitem.file.read())
# 	print("OK")
# else:
# 	print(form, file=sys.stderr)
# 	print("NO")

# if "file" in form:
# 	filedata = form["file"].value
# 	with open('/Users/naomisterk/codam/webbieservie/www/uploads/test', 'b') as f:
# 		f.write(filedata)
# 	print("OK")
# else:
# 	print(form, file=sys.stderr)

print(os.environ, file=sys.stderr)
sys.stdout.flush()
