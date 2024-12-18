#!/usr/bin/env python3
import cgi
import cgitb; cgitb.enable()
import os

print("Content-type:text/html\r\n\r\n")
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
    <title>Delete Image</title>
</head>
<body>
    <div class="cgi-screen">
        <h1 class="cgiTitle">Delete Image</h1>
        {message}
        <h2 class="statusText"><a href="view.py">Back to images</a></h2>
    </div>
</body>
</html>
"""

def delete_file(filename):
    if not filename:
        return 'No filename provided'
    
    file_path = os.path.join(upload_dir, filename)
    try:
        if os.path.exists(file_path):
            os.remove(file_path)
            return f'Successfully deleted {filename}'
        else:
            return 'File does not exist'
    except Exception as e:
        return f'Error deleting file: {str(e)}'

request_method = os.environ.get('REQUEST_METHOD', '')

if request_method == 'POST':
    # Handle POST request (for browser forms)
    form = cgi.FieldStorage()
    filename = form.getvalue('filename')
    result = delete_file(filename)
    print(html_template.format(message=f'<p>{result}</p>'))

else:
    # Handle unsupported methods
    print(html_template.format(message='<p>Please use POST or DELETE method to delete files</p>'))