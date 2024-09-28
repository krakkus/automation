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
                       "type TEXT,"
                       "last_updated INTEGER,"
                       "key_txt TEXT,"
                       "key_int INTEGER,"
                       "value_txt TEXT,"
                       "value_int INTEGER,"
                       "extra_txt TEXT,"
                       "extra_int INTEGER"
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

        if path == "/table_1_write":
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()

            api_key = params["api_key"][0]
            type = params["type"][0]
            last_updated = int(time.time())
            key_txt = param_or_empty("key_txt", params)
            key_int = param_or_zero("key_int", params)
            value_txt = param_or_empty("value_txt", params)
            value_int = param_or_zero("value_int", params)
            extra_txt = param_or_empty("extra_txt", params)
            extra_int = param_or_zero("extra_int", params)

            if len(api_key) != 32:
                return

            connection, cursor = open_db(api_key)

            cursor.execute("INSERT OR REPLACE INTO table_1 (type, last_updated, key_txt, key_int,"
                           "value_txt, value_int, extra_txt, extra_int) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                           (type, last_updated, key_txt, key_int, value_txt, value_int, extra_txt, extra_int))

            cursor.close()
            connection.commit()
            connection.close()

            response = ""

            self.wfile.write(response.encode('utf-8'))
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