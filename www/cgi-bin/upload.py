#!/usr/bin/python3
import cgi
import cgitb; cgitb.enable()
import os
import random
import string

print("Content-type:text/html\r\n\r\n")
form = cgi.FieldStorage()
upload_dir = os.environ.get('UPLOAD_DIR')
upload_dir = os.getenv('UPLOAD_DIR')
if upload_dir == "":
    upload_dir = '/mnt/c/Users/dmonf/OneDrive/Desktop/clo/www/images/'

html_template = """
<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="stylesheet" href="/index.css">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Upload Image</title>
</head>
<body>
    <div class="cgi-screen">
        <h1 class="cgiTitle">Select a File to Upload</h1>
        <form class="uploadFile" method="post" action="/cgi-bin/upload.py" enctype="multipart/form-data">
            <input class="chooseFile" type="file" name="fileName">
            <button class="uploadButton" type="submit">UPLOAD</button>
        </form>
        <h2 class="statusText">View <a href="view.py">uploaded images</a></h2>
    </div>
    {message}
</body>
</html>
"""

def show_message(message):
    if message:
        return f'<div class="alert">{message}</div>'
    return ''

if os.environ.get('REQUEST_METHOD') == 'POST':
    if "fileName" not in form:
        print(html_template.format(message=show_message("You need to provide a file to upload")))
    else:
        fileitem = form["fileName"]
        if not fileitem.filename:
            print(html_template.format(message=show_message("No file was uploaded")))
        else:
            filename = os.path.basename(fileitem.filename)
            if filename.lower().endswith(('.png', '.jpeg', '.jpg')):
                temp_pathname = os.path.join(upload_dir, filename)
                while os.path.exists(temp_pathname):
                    name, ext = os.path.splitext(filename)
                    temp_pathname = os.path.join(upload_dir, name + random.choice(string.ascii_letters) + ext)
                
                with open(temp_pathname, 'wb') as f:
                    f.write(fileitem.file.read())
                print(html_template.format(message=show_message("File was uploaded successfully")))
            else:
                print(html_template.format(message=show_message("Wrong format - only PNG and JPEG files are allowed")))
else:
    print(html_template.format(message=''))