#!/usr/bin/python3
import cgi
import cgitb; cgitb.enable()
import os
import random
import string
import errno
import sys

print("Content-type:text/html\r\n\r\n")
form = cgi.FieldStorage()
files = os.listdir("/home/cdalla/webserver/www")
dir_path = "/home/cdalla/webserver/www"

base_begin = """
<!DOCTYPE html>
<html lang="en">
<head>
	<link rel="stylesheet" href="/index.css">
	<meta charset="UTF-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>WebServ</title>
</head>
<body>
"""
base_middle = """
	<div class="cgi-screen">
		<div>
			<h1 class="cgiTitle">Selct a File to Upload</h1>
			<form class="uploadFile" method="post" action="/cgi-bin/upload.py" >
			  <input class="chooseFile" type="file" name="fileName">
			  <button class="uploadButton" id="uploadButton" type="submit">UPLOAD</button>
			</form>
		</div>
"""
base_end = """
<h2 class="statusText">Return <a href="../../index.html">home?</a></h2>
	  </div>
</body>
</html>
"""
def base_page(message="none"):
    print (base_begin)
    if message != "none":
        print ('<script>')
        print ("const alertHTML = '<div class=\"alert\">" + message + "</div>'; ")
        print ("document.body.insertAdjacentHTML('beforeend', alertHTML);")
        print("setTimeout(() => document.querySelector('.alert').classList.add('hide'), 3000);")
        print("</script>")
    print (base_middle)
    files = os.listdir(dir_path)
    if files:
        print ('<h1 class="cgiTitle">Uploaded Images</h1>')
        print ('<div class="uploadedImage">')
        for file in files:
            print ('<div class="uploadedImageItem">')
            print ('<form class="uploadFile" method="DELETE"')
            print ('<img src="'+ "/images/" + file + '" alt="' + file + '" class="uploadedImageSrc">')
            print ('<button class="cgiDeleteButton" id="cgiDeleteButton">DELETE</button>')
            print ('</div>')
        print ('</div>')
    print (base_end)

def upload_image():
    if "fileName" not in form:
        base_page("You need to provide a file to upload a file")
        return 
    filename = str(form["fileName"].value) 
    if '.png' == filename[:-4] or '.jpeg' == filename[:-5]:
        temp_pathname = dir_path + filename
        while True:
            if os.path.exists(temp_pathname):
                file_dtls = temp_pathname.split('.')
                temp_pathname = file_dtls[0] + random.choice(string.ascii_letters) + file_dtls[1]
            else:
                break
        open(temp_pathname, "w").write(open(0).read())
        base_page("File was uploaded successfully")
    else:
        base_page("Wrong format")

def delete_image():
    filename= form.getvalue('fileName')
    if not filename:
        base_page("You need to provide a file to delete")
    else:
        try:
            temp_pathname = dir_path + filename.value
            os.symlink(target, temp_pathname)
            os.remove( temp_pathname)
            base_page(filename + " was correctly deleted")
        except FileExistsError:
          base_page("The file you want to delete does not exist")
          
method_map = {"GET": base_page,
"POST": upload_image,
"DELETE": delete_image
}
method_map[os.environ['REQUEST_METHOD']]()
