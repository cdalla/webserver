#!/bin/bash

echo "Content-type: text/html"
echo ""

UPLOAD_DIR=${UPLOAD_DIR:-"/tmp/uploads"}

echo "<html>
<head>
    <title>Uploaded Files</title>
</head>
<body>
    <h2>Uploaded Files</h2>
    <ul>"

if [ -d "$UPLOAD_DIR" ]; then
    for file in "$UPLOAD_DIR"/*; do
        if [ -f "$file" ]; then
            filename=$(basename "$file")
            echo "<li>$filename</li>"
        fi
    done
else
    echo "<li>Upload directory not found</li>"
fi

echo "</ul>
</body>
</html>"