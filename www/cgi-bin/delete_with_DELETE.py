#!/usr/bin/env python3
import json
import os
from pathlib import Path

def is_valid_image(filename):
    """Check if the file has a valid image extension"""
    valid_extensions = {'.jpg', '.jpeg', '.png', '.gif', '.bmp', '.webp'}
    return Path(filename).suffix.lower() in valid_extensions

def sanitize_filename(filename):
    """Basic filename sanitization"""
    return Path(filename).name

def get_upload_dir():
    """Get upload directory from environment or use script's directory"""
    upload_dir = os.environ.get('UPLOAD_DIR')
    if not upload_dir:
        # Get the directory where the script is located
        upload_dir = os.path.dirname(os.path.abspath(__file__))
    return upload_dir

def send_response(status_code, response_data):
    """Send HTTP response with headers and JSON data"""
    print("Status: " + status_code)
    print("Content-Type: application/json")
    print()  # Empty line after headers
    print(json.dumps(response_data))

def main():
    # Get upload directory or use current directory
    upload_dir = get_upload_dir()
    
    # Check if the request method is DELETE
    if os.environ.get('REQUEST_METHOD') != 'DELETE':
        send_response("405 Method Not Allowed", {
            'status': 'error',
            'message': 'Only DELETE method is allowed'
        })
        return
    
    # Parse query string
    query_string = os.environ.get('QUERY_STRING', '')
    if not query_string:
        send_response("400 Bad Request", {
            'status': 'error',
            'message': 'No parameters provided'
        })
        return
        
    # Parse parameters
    try:
        parameters = dict(pair.split('=') for pair in query_string.split('&') if pair)
    except:
        send_response("400 Bad Request", {
            'status': 'error',
            'message': 'Invalid query string format'
        })
        return
    
    # Get image name
    image_name = parameters.get('image')
    if not image_name:
        send_response("400 Bad Request", {
            'status': 'error',
            'message': 'No image name provided'
        })
        return
    
    # Sanitize and validate filename
    image_name = sanitize_filename(image_name)
    if not is_valid_image(image_name):
        send_response("400 Bad Request", {
            'status': 'error',
            'message': 'Invalid image file type'
        })
        return
    
    # Construct full path
    image_path = os.path.join(upload_dir, image_name)
    
    try:
        if not os.path.exists(image_path):
            send_response("404 Not Found", {
                'status': 'error',
                'message': f'Image {image_name} not found in {upload_dir}'
            })
        else:
            os.remove(image_path)
            send_response("200 OK", {
                'status': 'success',
                'message': f'Image {image_name} deleted successfully from {upload_dir}'
            })
            
    except Exception as e:
        send_response("500 Internal Server Error", {
            'status': 'error',
            'message': f'Failed to delete image: {str(e)}'
        })

if __name__ == "__main__":
    main()