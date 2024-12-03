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
dir_path = "/home/cdalla/webserver/www/images"  # Added trailing slash

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
            <h1 class="cgiTitle">Select a File to Upload</h1>
            <form class="uploadFile" method="post" action="/cgi-bin/upload.py" enctype="multipart/form-data">
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
    print(base_begin)
    if message != "none":
        print('<script>')
        print("const alertHTML = '<div class=\"alert\">" + message + "</div>'; ")
        print("document.body.insertAdjacentHTML('beforeend', alertHTML);")
        print("setTimeout(() => document.querySelector('.alert').classList.add('hide'), 3000);")
        print("</script>")
    print(base_middle)
    
    files = os.listdir(dir_path)
    if files:
        print('<h1 class="cgiTitle">Uploaded Images</h1>')
        print('<div class="uploadedImage">')
        for file in files:
            print('<div class="uploadedImageItem">')
            # Changed to point to the same script with DELETE method
            print(f'<form class="uploadFile" method="DELETE" action="/cgi-bin/upload.py">')
            print(f'<img src="/images/{file}" alt="{file}" class="uploadedImageSrc">')
            print(f'<input type="hidden" name="filename" value="{file}">')
            print('<button class="cgiDeleteButton" type="submit">DELETE</button>')
            print('</form>')
            print('</div>')
        print('</div>')
    print(base_end)

def upload_image():
    if "fileName" not in form:
        base_page("You need to provide a file to upload a file")
        return
    
    fileitem = form["fileName"]
    if not fileitem.filename:
        base_page("No file was uploaded")
        return
        
    filename = os.path.basename(fileitem.filename)
    if filename.lower().endswith(('.png', '.jpeg', '.jpg')):
        temp_pathname = os.path.join(dir_path, filename)
        while os.path.exists(temp_pathname):
            name, ext = os.path.splitext(filename)
            temp_pathname = os.path.join(dir_path, name + random.choice(string.ascii_letters) + ext)
        
        with open(temp_pathname, 'wb') as f:
            f.write(fileitem.file.read())
        base_page("File was uploaded successfully")
    else:
        base_page("Wrong format - only PNG and JPEG files are allowed")

def delete_image():
    filename = form.getvalue('filename')
    if not filename:
        base_page("You need to provide a file to delete")
        return
    
    file_path = os.path.join(dir_path, filename)
    try:
        if os.path.exists(file_path):
            os.remove(file_path)
            base_page(f"{filename} was correctly deleted")
        else:
            base_page("The file you want to delete does not exist")
    except Exception as e:
        base_page(f"Error deleting file: {str(e)}")

# Map HTTP methods to functions
method_map = {
    "GET": base_page,
    "POST": upload_image,
    "DELETE": delete_image
}

# Get the request method and call the appropriate function
request_method = os.environ.get('REQUEST_METHOD', 'GET')
method_map.get(request_method, base_page)()