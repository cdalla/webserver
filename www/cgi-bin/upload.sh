#!/bin/bash

# Function to send JSON response with status code
send_json_response() {
    local http_code=$1
    local status=$2
    local message=$3
    echo "Status: $http_code"
    echo "Content-Type: application/json"
    echo
    echo "{"
    echo "  \"status\": \"$status\","
    echo "  \"message\": \"$message\""
    echo "}"
}

# Check if UPLOAD_DIR is set, if not use script's directory
if [ -z "$UPLOAD_DIR" ]; then
    UPLOAD_DIR=$(dirname "$0")
fi

# Create upload directory if it doesn't exist
mkdir -p "$UPLOAD_DIR"

# Check if the directory is writable
if [ ! -w "$UPLOAD_DIR" ]; then
    send_json_response "500 Internal Server Error" "error" "Upload directory is not writable"
    exit 1
fi

# Check if this is a POST request with file upload
if [ "$REQUEST_METHOD" != "POST" ]; then
    send_json_response "405 Method Not Allowed" "error" "Only POST method is allowed"
    exit 1
fi

# Check content type for multipart form data
if [[ "$CONTENT_TYPE" != *"multipart/form-data"* ]]; then
    send_json_response "400 Bad Request" "error" "Content-Type must be multipart/form-data"
    exit 1
fi

# Create a temporary file
TEMP_FILE=$(mktemp)
cat > "$TEMP_FILE"

# Get the boundary from content type
BOUNDARY=$(echo "$CONTENT_TYPE" | sed -n 's/.*boundary=\(.*\)/\1/p')

# Process the uploaded file
if [ -s "$TEMP_FILE" ]; then
    # Extract filename from the form data
    FILENAME=$(grep -a "Content-Disposition: .*filename=" "$TEMP_FILE" | sed -e 's/.*filename="\([^"]*\)".*/\1/' | head -1)
    
    if [ -z "$FILENAME" ]; then
        rm "$TEMP_FILE"
        send_json_response "400 Bad Request" "error" "No file was uploaded"
        exit 1
    fi

    # Clean filename (remove path and special characters)
    FILENAME=$(basename "$FILENAME" | sed 's/[^a-zA-Z0-9._-]/_/g')

    # Extract the file content (skipping headers)
    sed -e "1,/^\r$/d" < "$TEMP_FILE" | sed -e "\$d" | sed -e "\$d" > "$UPLOAD_DIR/$FILENAME"

    # Check if file was saved successfully
    if [ -f "$UPLOAD_DIR/$FILENAME" ]; then
        send_json_response "201 Created" "success" "File $FILENAME uploaded successfully to $UPLOAD_DIR"
    else
        send_json_response "500 Internal Server Error" "error" "Failed to save file"
    fi
else
    send_json_response "400 Bad Request" "error" "No data received"
fi

# Clean up
rm "$TEMP_FILE"