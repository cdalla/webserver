#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# Test function with status code
test_endpoint() {
    local description=$1
    local command=$2
    echo -e "\n🔍 Testing: $description"
    eval $command
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Pass${NC}"
    else
        echo -e "${RED}✗ Failed${NC}"
    fi
}

# Create test files
echo "Creating test files..."
dd if=/dev/zero of=large_file.txt bs=1M count=1 2>/dev/null
echo "Test content" > small_file.txt
convert -size 100x100 xc:white test_image.png 2>/dev/null || echo "ImageMagick not installed - skipping image creation"

# Basic server tests
test_endpoint "Server 1 basic GET" \
    "curl -s --resolve server_1:8081:127.0.0.1 http://server_1:8081/ | grep 'server 1'"

test_endpoint "Server 8 basic GET" \
    "curl -s --resolve server_8:8081:127.0.0.1 http://server_8:8081/ | grep 'server 1'"

test_endpoint "Server 2 basic GET" \
    "curl -s --resolve server_2:8081:127.0.0.1 http://server_2:8081/ | grep 'server 2'"


# Verify empty response body
test_endpoint "Server 3 empty body" \
    "[ -z \"$(curl -s --resolve server_3:8083:127.0.0.1 http://server_3:8083/)\" ]"

test_endpoint "Server 4 empty body" \
    "[ -z \"$(curl -s --resolve server_4:8084:127.0.0.1 http://server_4:8084/)\" ]"
# Directory listing tests
test_endpoint "Directory listing - HTML structure" \
    "curl -s http://localhost:8062/ | grep -q '<title>Directory listing</title>'"

test_endpoint "Directory listing - Title content" \
    "curl -s http://localhost:8062/ | grep -q '<h1>Directory listing for /</h1>'"

for file in "server_1.html" "server_2.html" "server_3.html" "server_4.html" "server_5.html"; do
    test_endpoint "Directory listing - Check for $file" \
        "curl -s http://localhost:8062/ | grep -q '<a href=\"/$file\">$file</a>'"
done

# Test forbidden methods (should return 405 Method Not Allowed)
for method in PUT PATCH OPTIONS HEAD TRACE CONNECT; do
    test_endpoint "Forbidden $method method" \
        "curl -s -X $method http://localhost:8062/ -w '%{http_code}' | grep -q '501'"
done

# Upload tests
test_endpoint "Small file upload" \
    "curl -s -X POST -F 'file=@small_file.txt' http://localhost:8060/upload.sh"

test_endpoint "Large file upload (should fail)" \
    "! curl -s -X POST -F 'file=@large_file.txt' http://localhost:8060/upload.sh | grep 'error'"

# Timeout test function
test_timeout() {
    echo -e "\n🔍 Testing: Connection timeout behavior"
    
    start_time=$(date +%s)
    
    # Run netcat with timeout and capture response
    response=$(timeout 11 bash -c '(echo -ne "GET / HTTP/1.1\r\nHost:localhost\r\n"; sleep 100) | nc localhost 8081')
    
    end_time=$(date +%s)
    elapsed=$((end_time - start_time))
    
    # Check if response contains 408 status code and proper content
    if echo "$response" | grep -q "HTTP/1.1 408 Request Timeout" && \
       echo "$response" | grep -q "Content-Type: text/html" && \
       echo "$response" | grep -q "<title>408 Request Timeout</title>" && \
       [ $elapsed -ge 9 ] && [ $elapsed -le 11 ]; then
        echo -e "${GREEN}✓ Pass - Connection timed out after $elapsed seconds with proper 408 response${NC}"
        return 0
    else
        echo -e "${RED}✗ Failed - Either timeout duration ($elapsed seconds) was wrong or incorrect response${NC}"
        return 1
    fi
}

# Run timeout test
test_timeout

# Test server responsiveness after timeout
test_endpoint "Server responsiveness after timeout" \
    "curl -s --max-time 5 http://localhost:8062/ | grep -q '<title>Directory listing</title>'"

# Test POST with wrong content type
test_endpoint "POST with invalid content type" \
    "curl -s -X POST -H 'Content-Type: text/plain' --data 'test' http://localhost:8060/upload.sh -w '%{http_code}' | grep -q '502'"

# Cleanup function
cleanup() {
    echo -e "\nCleaning up..."
    rm -f large_file.txt small_file.txt test_image.png empty_test.html
}

# Register cleanup
trap cleanup EXIT

echo -e "\nTests completed! Check results above."