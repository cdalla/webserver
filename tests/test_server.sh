#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Counter for tests
TESTS_PASSED=0
TESTS_FAILED=0

# Function to run a test
run_test() {
    local test_name=$1
    local command=$2
    local expected_status=$3

    echo "Running test: $test_name"
    
    # Run the command and capture both output and status
    output=$(eval $command 2>&1)
    status=$?

    if [ $status -eq $expected_status ]; then
        echo -e "${GREEN}✓ Test passed: $test_name${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ Test failed: $test_name${NC}"
        echo "Command: $command"
        echo "Expected status: $expected_status"
        echo "Got status: $status"
        echo "Output: $output"
        ((TESTS_FAILED++))
    fi
    echo "----------------------------------------"
}

# Create test files
echo "Creating test files..."
echo "Test content" > testfile.txt
dd if=/dev/zero of=largefile bs=1M count=50 2>/dev/null

# Basic GET Tests
run_test "Basic GET request to root" \
    "curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/" \
    0

run_test "GET request to /test directory" \
    "curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/test/" \
    0

run_test "GET request for index.html" \
    "curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/index.html" \
    0

# POST Tests
run_test "Basic POST request" \
    "curl -s -X POST -d 'test=data' -o /dev/null -w '%{http_code}' http://localhost:8080/test/" \
    0

run_test "File upload POST request" \
    "curl -s -X POST -F 'file=@testfile.txt' -o /dev/null -w '%{http_code}' http://localhost:8080/test/" \
    0

# CGI Tests
run_test "CGI GET request" \
    "curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/cgi-bin/test.py" \
    0

run_test "CGI POST request" \
    "curl -s -X POST -d 'param1=value1' -o /dev/null -w '%{http_code}' http://localhost:8080/cgi-bin/test.py" \
    0

# DELETE Tests
run_test "DELETE request" \
    "curl -s -X DELETE -o /dev/null -w '%{http_code}' http://localhost:8080/test/testfile.txt" \
    0

# Error Cases
run_test "404 Not Found" \
    "[ \$(curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/nonexistent) -eq 404 ]" \
    0

run_test "405 Method Not Allowed" \
    "[ \$(curl -s -X PUT -o /dev/null -w '%{http_code}' http://localhost:8080/) -eq 405 ]" \
    0

# Large File Test
run_test "Large file upload (testing max_body_size)" \
    "curl -s -X POST -F 'file=@largefile' -o /dev/null -w '%{http_code}' http://localhost:8080/test/" \
    0

# Chunked Transfer Test
run_test "Chunked transfer encoding" \
    "curl -s -X POST --header 'Transfer-Encoding: chunked' --data-binary @testfile.txt -o /dev/null -w '%{http_code}' http://localhost:8080/test/" \
    0

# Clean up test files
rm -f testfile.txt largefile

# Print summary
echo "========================================="
echo "Test Summary:"
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"
echo "Total: $((TESTS_PASSED + TESTS_FAILED))"

# Exit with failure if any tests failed
if [ $TESTS_FAILED -gt 0 ]; then
    exit 1
fi

exit 0