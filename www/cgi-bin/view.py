#!/usr/bin/python3
import os

print("Content-type:text/html\r\n\r\n")
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
    <title>View Images</title>
</head>
<body>
    <div class="cgi-screen">
        <h1 class="cgiTitle">Uploaded Images</h1>
        <div class="uploadedImage">
            {images}
        </div>
        <h2 class="statusText"><a href="upload.py">Upload new image</a></h2>
    </div>
</body>
</html>
"""

def get_images():
    images_html = []
    try:
        files = os.listdir(upload_dir)
        for file in files:
            if file.lower().endswith(('.png', '.jpeg', '.jpg')):
                images_html.append(f'''
                    <div class="uploadedImageItem">
                        <img src="/images/{file}" alt="{file}" class="uploadedImageSrc">
                        <form class="uploadFile" method="post" action="delete.py">
                            <input type="hidden" name="filename" value="{file}">
                            <button class="cgiDeleteButton" type="submit">DELETE</button>
                        </form>
                    </div>
                ''')
    except Exception as e:
        return f'<p>Error loading images: {str(e)}</p>'
    
    return '\n'.join(images_html) if images_html else '<p>No images found</p>'

print(html_template.format(images=get_images()))
