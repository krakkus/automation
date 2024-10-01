import sqlite3
import http.server
import urllib.parse
import secrets
import string
import os
import json
import time

port = 30008

def generate_key(length=32):
    # Combine uppercase letters, lowercase letters, digits, and punctuation characters
    characters = string.ascii_uppercase + string.ascii_lowercase + string.digits

    # Use `secrets.choice` for cryptographically secure random number generation
    key = ''.join(secrets.choice(characters) for _ in range(length))

    return key

def open_db(api_key):
    fn = os.path.join(os.path.expanduser('~'), "databases")
    if not os.path.exists(fn):
        os.makedirs(fn)

    fn = os.path.join(fn, f"{api_key}.db")

    print(fn)

    version = None
    if not os.path.exists(fn):
        version = 0

    connection = sqlite3.connect(fn)
    cursor = connection.cursor()

    if version == 0:
        cursor.execute("CREATE TABLE table_1 ("
                       "type TEXT DEFAULT '',"
                       "last_updated INTEGER DEFAULT CURRENT_TIMESTAMP,"
                       "key_txt TEXT DEFAULT '',"
                       "key_int INTEGER DEFAULT 0,"
                       "value_txt TEXT DEFAULT '',"
                       "value_int INTEGER DEFAULT 0,"
                       "extra_txt TEXT DEFAULT '',"
                       "extra_int INTEGER DEFAULT 0"
                       ")")
        cursor.execute("CREATE UNIQUE INDEX table_1_index ON table_1 (type, key_txt, key_int)")
        cursor.execute("INSERT INTO table_1 (type, key_txt, key_int, value_int) VALUES ('SYSTEM', 'db_version', 0, 1)")
        version = None

    if version == None:
        row = cursor.execute("SELECT value_int FROM table_1 WHERE type='SYSTEM' AND key_txt='db_version' AND key_int=0").fetchone()
        version = row[0]

    # if version == 1:
    #     cursor.execute("CREATE TABLE heartbeat (id TEXT UNIQUE PRIMARY KEY, ip TEXT, modified TIMESTAMP)")
    #     cursor.execute("CREATE TABLE hour_value_int (key TEXT, hour INTEGER, value INTEGER, modified TIMESTAMP)")
    #     cursor.execute("UPDATE single_value_int SET value = '2' AND modified=CURRENT_TIMESTAMP WHERE key = 'db_version'")
    #     version = 2

    return connection, cursor

def param_or_zero(key, params):
    if key in params:
        return params[key][0]
    else:
        return 0

def param_or_empty(key, params):
    if key in params:
        return params[key][0]
    else:
        return ""

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

        if path == "/query":
            self.send_response(200)
            self.send_header("Content-type", "text/json")
            self.end_headers()

            if len(params["apikey"][0]) != 32:
                return
            connection, cursor = open_db(params["apikey"][0])

            cursor.execute(params["query"][0])
            rows = cursor.fetchall()
            cursor.close()
            connection.commit()
            connection.close()

            json_string = json.dumps(rows)
            self.wfile.write(json_string.encode('utf-8'))

            return

        else:
            self.send_error(404)

def main():
    server_address = ("", port)
    httpd = http.server.HTTPServer(server_address, MyHTTPRequestHandler)
    print(f"Serving HTTP on port {port}...")
    httpd.serve_forever()

if __name__ == "__main__":
    main()