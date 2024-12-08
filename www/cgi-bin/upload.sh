#!/bin/bash

echo "Content-type: text/html"
echo ""

UPLOAD_DIR=${UPLOAD_DIR:-"/tmp/uploads"}
mkdir -p "$UPLOAD_DIR" 2>/dev/null

while read -r line; do
    if [[ "$line" == *"filename="* ]]; then
        filename=$(echo "$line" | sed -n 's/.*filename="\([^"]*\)".*/\1/p')
        # Skip the headers until empty line
        while read -r line && [ -n "$line" ]; do continue; done

        cat > "$UPLOAD_DIR/$filename"
        break
    fi
done

echo "<html><head><title>Upload Result</title></head><body>"
if [ -f "$UPLOAD_DIR/$filename" ]; then
    echo "<p>File uploaded successfully as: $filename</p>"
else
    echo "<p>Error: Upload failed</p>"
fi
echo "</body></html>"