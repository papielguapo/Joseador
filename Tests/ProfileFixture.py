"""Local-only fixture for BridgeProbe --profile-test=write then =read."""
from http.server import BaseHTTPRequestHandler, HTTPServer

class Fixture(BaseHTTPRequestHandler):
    def do_GET(self):
        body = b'<!doctype html><meta name="viewport" content="width=device-width"><body>Isolated persistence test</body>'
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)
    def log_message(self, *args):
        pass

HTTPServer(('127.0.0.1', 18947), Fixture).serve_forever()
