#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# Helper Functions
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

create_test_files() {
    echo "Creating test files..."
    dd if=/dev/zero of=large_file.txt bs=1M count=1 2>/dev/null
    echo "Test content" > small_file.txt
    
    # Create test image using base64
    echo "Creating test image..."
    cat > test_image.png << EOF
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==
EOF
    base64 -d > test_image.png << EOF
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==
EOF
}

test_timeout() {
    echo -e "\n🔍 Testing: Connection timeout behavior"
    start_time=$(date +%s)
    response=$(timeout 11 bash -c '(echo -ne "GET / HTTP/1.1\r\nHost:localhost\r\n"; sleep 100) | nc localhost 8081')
    end_time=$(date +%s)
    elapsed=$((end_time - start_time))
    
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

cleanup() {
    echo -e "\nCleaning up..."
    rm -f large_file.txt small_file.txt test_image.png empty_test.html
    [ -f "../www/bash_test/large_file.txt" ] && rm -f "../www/bash_test/large_file.txt"
}

# Register cleanup
trap cleanup EXIT

# Main Test Suite
main() {
    # Create necessary test files
    create_test_files

    # 1. Basic Server Tests
    echo -e "\n${GREEN}Running Basic Server Tests${NC}"
    test_endpoint "Server 1 basic GET" \
        "curl -s --resolve server_1:8081:127.0.0.1 http://server_1:8081/ | grep 'server 1'"
    test_endpoint "Server 8 basic GET" \
        "curl -s --resolve server_8:8081:127.0.0.1 http://server_8:8081/ | grep 'server 1'"
    test_endpoint "Server 2 basic GET" \
        "curl -s --resolve server_2:8081:127.0.0.1 http://server_2:8081/ | grep 'server 2'"

    # 2. Empty Response Tests
    echo -e "\n${GREEN}Running Empty Response Tests${NC}"
    test_endpoint "Server 3 empty body" \
        "[ -z \"$(curl -s --resolve server_3:8083:127.0.0.1 http://server_3:8083/)\" ]"
    test_endpoint "Server 4 empty body" \
        "[ -z \"$(curl -s --resolve server_4:8084:127.0.0.1 http://server_4:8084/)\" ]"

    # 3. Directory Listing Tests
    echo -e "\n${GREEN}Running Directory Listing Tests${NC}"
    test_endpoint "Directory listing - HTML structure" \
        "curl -s http://localhost:8062/ | grep -q '<title>Directory listing</title>'"
    test_endpoint "Directory listing - Title content" \
        "curl -s http://localhost:8062/ | grep -q '<h1>Directory listing for /</h1>'"

    for file in "server_1.html" "server_2.html" "server_3.html" "server_4.html" "server_5.html"; do
        test_endpoint "Directory listing - Check for $file" \
            "curl -s http://localhost:8062/ | grep -q '<a href=\"/$file\">$file</a>'"
    done

    # 4. HTTP Method Tests
    echo -e "\n${GREEN}Running HTTP Method Tests${NC}"
    for method in PUT PATCH OPTIONS HEAD TRACE CONNECT; do
        test_endpoint "Forbidden $method method" \
            "curl -s -X $method http://localhost:8062/ -w '%{http_code}' | grep -q '501'"
    done

    # 5. CGI Tests
    echo -e "\n${GREEN}Running CGI Tests${NC}"
    test_endpoint "GET CGI infinite loop" \
        "curl -s -X GET http://localhost:8064/infinite.py -w '%{http_code}' | grep -q '504'"
    test_endpoint "GET CGI mistake in the script" \
        "curl -s -X GET http://localhost:8064/mistake.py -w '%{http_code}' | grep -q '502'"

    # 6. File Upload Tests
    echo -e "\n${GREEN}Running Upload Tests${NC}"
    test_endpoint "Small file upload" \
        "curl -s -X POST -F 'file=@small_file.txt' http://localhost:8060/upload.sh"
    test_endpoint "Large file upload (should fail)" \
        "! curl -s -X POST -F 'file=@large_file.txt' http://localhost:8060/upload.sh | grep 'error'"

    # 7. Timeout Tests
    echo -e "\n${GREEN}Running Timeout Tests${NC}"
    test_timeout
    test_endpoint "Server responsiveness after timeout" \
        "curl -s --max-time 5 http://localhost:8062/ | grep -q '<title>Directory listing</title>'"

    # 8. Large File Tests
    echo -e "\n${GREEN}Running Large File Tests${NC}"
    dd if=/dev/zero of=../www/bash_test/large_file.txt bs=1M count=1024 status=progress
    
    if [ -f "../www/bash_test/large_file.txt" ]; then
        size=$(stat -f%z "../www/bash_test/large_file.txt" 2>/dev/null || stat -c%s "../www/bash_test/large_file.txt")
        expected_size=$((1024 * 1024 * 1024))
        
        if [ "$size" -eq "$expected_size" ]; then
            echo -e "${GREEN}Successfully created 1GB file${NC}"
            
            echo -e "\n${GREEN}Testing large file download...${NC}"
            start_time=$(date +%s)
            curl -o /dev/null \
                 -w "Time taken: %{time_total}s\nSpeed: %{speed_download} bytes/sec\nHTTP code: %{http_code}\n" \
                 http://localhost:8062/large_file.txt
            end_time=$(date +%s)
            echo "Total duration: $((end_time - start_time)) seconds"
        else
            echo -e "${RED}File size incorrect: $size bytes (expected $expected_size bytes)${NC}"
        fi
    else
        echo -e "${RED}Failed to create large file${NC}"
    fi

    echo -e "\nTests completed! Check results above."
}

# Run the test suite
main