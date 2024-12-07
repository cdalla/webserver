#!/usr/bin/python3
import cgi, os
import sys
import cgitb; cgitb.enable()
import random
import string


form = cgi.FieldStorage()
upload_dir = os.environ.get('UPLOAD_DIR')
upload_dir = os.getenv('UPLOAD_DIR')
if upload_dir == "":
    upload_dir = '/mnt/c/Users/dmonf/OneDrive/Desktop/clo/www/images/'
# Get filename here.
fileitem = form['fileName']
# Test if the file was uploaded
if fileitem.filename:
	# strip leading path from file name to avoid
	# directory traversal attacks
	filename = os.path.basename(fileitem.filename)
	temp_pathname = os.path.join(upload_dir, filename)
	while os.path.exists(temp_pathname):
		name, ext = os.path.splitext(filename)
		temp_pathname = os.path.join(upload_dir, name + random.choice(string.ascii_letters) + ext)
	with open(temp_pathname, 'wb') as f:
		f.write(fileitem.file.read())
		message = 'The file "' + temp_pathname + '" was uploaded successfully'
		print("Content-type:text/html\r\n\r\n")
		html_template = """
		<!DOCTYPE html>

		<head>
		<link rel="stylesheet" type="text/css" href="style.css">
		<title>Fucking dope HTML page</title>
		</head>

		<body>
		<div class="header">
		<h1>This is where you upload stuff</h1>
		</div>
		<div class="menu">
		<ul id="menu"></ul>
		</div>
		<div class="container">
		<div class="image">
		<img src="IllusionsMichael.jpg" width="400" height="400" alt="ILLUSIONS, michael"/>
		</div>
		<div class="text">
		YAY about something
		</div>
		</div>
		<div class="footer">
		<ul id="footer"></ul>
		</div>
		<script src="menu.js" defer></script>
		</body>
		</html>
		"""
		print(html_template)
 
else:
	message = 'No file was uploaded'
	print("Content-type:text/html\r\n\r\n")
	html_template = """
	<!DOCTYPE html>

	<head>
		<link rel="www/stylesheet" type="text/css" href="style.css">
	<title>Fucking dope HTML page</title>
	</head>

	<body>
		<div class="header">
			<h1>This is where you upload stuff</h1>
		</div>
		<div class="menu">
			<ul id="menu"></ul>
		</div>
		<div class="container">
			<div class="image">
				<img src="www/img/IllusionsMichael.jpg" width="400" height="400" alt="ILLUSIONS, michael"/>
			</div>
			<div class="text">
				BOO about something
			</div>
		</div>
		<div class="footer">
			<ul id="footer"></ul>
		</div>
		<script src="www/menu.js" defer></script>
	</body>
	</html>
	"""
	print(html_template.format(message=message))