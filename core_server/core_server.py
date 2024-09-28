import sqlite3
import http.server
import urllib.parse
import secrets
import string
import os
import time

port = 30008

def generate_key(length=32):
    # Combine uppercase letters, lowercase letters, digits, and punctuation characters
    characters = string.ascii_uppercase + string.ascii_lowercase + string.digits

    # Use `secrets.choice` for cryptographically secure random number generation
    key = ''.join(secrets.choice(characters) for _ in range(length))

    return key

def open_db(key):
    fn = f"core_server/databases/{key}.db"

    version = None
    if not os.path.exists(fn):
        version = 0

    connection = sqlite3.connect(fn)
    cursor = connection.cursor()

    if version == 0:
        cursor.execute("CREATE TABLE values_int (key TEXT, value INTEGER, modified TIMESTAMP)")
        cursor.execute("INSERT INTO single_value_int VALUES ('db_version', '1', CURRENT_TIMESTAMP)")
        version = None

    if version == None:
        row = cursor.execute("SELECT key, value FROM values_int WHERE key = 'db_version'").fetchone()
        version = row[1]

    # if version == 1:
    #     cursor.execute("CREATE TABLE heartbeat (id TEXT UNIQUE PRIMARY KEY, ip TEXT, modified TIMESTAMP)")
    #     cursor.execute("CREATE TABLE hour_value_int (key TEXT, hour INTEGER, value INTEGER, modified TIMESTAMP)")
    #     cursor.execute("UPDATE single_value_int SET value = '2' AND modified=CURRENT_TIMESTAMP WHERE key = 'db_version'")
    #     version = 2

    return connection, cursor

class MyHTTPRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        path = urllib.parse.urlparse(self.path).path
        params = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)

        # Handle specific URLs and parameters here
        if path == "/generate_key":
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()

            response = generate_key()

            self.wfile.write(response.encode('utf-8'))
            return

        if path == "/endpoint_heartbeat":
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()

            key = params["key"][0]
            id = params["id"][0]
            ip = params["ip"][0]

            if len(key) != 32:
                return

            connection, cursor = open_db(key)

            cursor.execute("INSERT OR REPLACE INTO endpoints (id, ip, time) VALUES (?, ?, ?)",
                           (id, ip, int(time.time())))

            cursor.close()
            connection.commit()
            connection.close()

            response = ""

            self.wfile.write(response.encode('utf-8'))
            return

        else:
            self.send_error(404)

if __name__ == "__main__":
    server_address = ("", port)
    httpd = http.server.HTTPServer(server_address, MyHTTPRequestHandler)
    print(f"Serving HTTP on port {port}...")
    httpd.serve_forever()