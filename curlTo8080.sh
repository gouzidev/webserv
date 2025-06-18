curl -X POST http://127.0.0.1:8080/test \
  -H "X-Huge-Header-1: $(python3 -c 'print("A" * 100)')" \
  -H "Content-Type: application/json" \
  -d '{"test": "data"}'