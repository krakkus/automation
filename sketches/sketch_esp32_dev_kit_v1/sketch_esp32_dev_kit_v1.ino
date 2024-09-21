#include <WiFi.h>
#include <EEPROM.h>
#include <WebServer.h>

String id;
String ssid;
String password;

const char* ap_ssid = "MY_ESP32"; // Access point SSID
const char* ap_password = "password"; // Access point password

// Create an instance of the web server
WebServer server(80);

// Pin ownership registration
enum Owner { None, DigitalWrite, DigitalRead, AnalogWrite, AnalogRead };
Owner owners[40] = { None };

void setup() {
  Serial.begin(115200);
  delay(2000);

#define EEPROM_SIZE 1024
  EEPROM.begin(EEPROM_SIZE);

  loadAllConfig();

  Serial.print("Device ID: ");
  Serial.println(id);

  Serial.print("WiFi SSID: ");
  Serial.println(ssid);
  Serial.print("WiFi password: ");
  Serial.println(password);

  WiFi.hostname(id);
  WiFi.mode(WIFI_STA); // Set mode to station (connect to an existing network)
  WiFi.begin(ssid, password);

  int timeout = 10;
  Serial.print("Connecting to WiFi");
  while (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
    delay(1000);
    Serial.print(".");
    timeout -= 1;
    if (timeout == 0) break;
  }
  Serial.println();
  Serial.println("WiFi Status code: " + wl_status_to_string(WiFi.status()));

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to network, SSID: " + String(ssid));
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Could not connect to network");
    Serial.println("setting up access point");
    Serial.print("Access point SSID: ");
    Serial.println(ap_ssid);
    Serial.print("Access point password: ");
    Serial.println(ap_password);

    WiFi.mode(WIFI_AP); // Set mode to access point
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("Access point started");
    Serial.print("Access point IP Address: ");
    Serial.println(WiFi.softAPIP());
  }

  // Start the web server
  server.on("/", handleRoot);
  server.on("/handshake", handleHandshake);
  server.on("/digitalwrite", handleDigitalWrite);
  server.on("/digitalread", handleDigitalRead);
  server.on("/analogwrite", handleAnalogWrite);
  server.on("/analogread", handleAnalogRead);
  server.on("/configwifi", handleConfigWifi);
  server.on("/savewifi", handleSaveWifi);
  server.begin();
}


void loop() {
  server.handleClient();
}

/***********************************
      EEPROM
***********************************/

#define MAX_STRING_LENGTH 255

char buffer[MAX_STRING_LENGTH + 1];
int ptr;

void loadAllConfig() {
  ptr = 0;

  id = loadString();
  ssid = loadString();
  password = loadString();
}

String loadString() {
  int i = 0;
  char c;
  while (i < MAX_STRING_LENGTH) {
    c = EEPROM.read(ptr);
    buffer[i] = c;
    i++;
    ptr++;

    if (c == 0 || c == 255) break;
  }

  return String(buffer);
}

void saveAllConfig() {
  ptr = 0;
  saveString(id);
  saveString(ssid);
  saveString(password);

  EEPROM.commit();
}

void saveString(String str) {
  str.toCharArray(buffer, MAX_STRING_LENGTH);

  Serial.print("saveString: ");
  Serial.println(buffer);

  int i = 0;
  char c;
  while (true) {
    c = buffer[i];
    EEPROM.write(ptr, c);
    i++;
    ptr++;

    if (c == 0) break;
  }
}

/***********************************
      Page templates
***********************************/

#define INDEX_HTML \
  "<!DOCTYPE html>\
<html>\
<head>\
<title>ESP32 DEVKIT V1</title>\
<style>\
body {\
  background-color: #111111;\
  color: #ffffff;\
  font-size: 3em;\
  font-family: Arial, sans-serif;\
}\
button {\
  font-size: 1em;\
  margin-bottom: 8px;\
  border-radius: 12px;\
  background-color: #ADD8E6;\
}\
h1, h2, h3, h4, h5, h6 {\
  color: yellow;\
}\
a {\
  color: #ffffff;\
}\
input {\
  font-size: 1em;\
}\
</style>\
</head>\
<body>\
_CONTENT_\
</body>\
</html>"

#define NAV_ROOT \
  "<h1>Home</h1>\
<a href=\"/configwifi\">Configure WiFi</a>\
<hr>\
<a href=\"/digitalwrite\">Digital Write</a><br>\
<a href=\"/digitalread\">Digital Read</a><br>\
<a href=\"/analogwrite\">Analog Write</a><br>\
<a href=\"/analogread\">Analog Read</a>"

#define NAV_DIGITALWRITEBUTTON \
  "<h1>Digital Write</h1>\
  <button onclick=\"loadURL('_URI1_')\">ON</button>\
  <button onclick=\"loadURL('_URI2_')\">OFF</button>\
  <div id=\"response\"></div>\
  <script>\
    function loadURL(url) {\
      fetch(url)\
        .then(response => response.text())\
        .then(data => {\
          const responseDiv = document.getElementById(\"response\");\
          responseDiv.innerHTML = data;\
        })\
        .catch(error => {\
          console.error(\"Error loading URL:\", error);\
        });\
    }\
  </script>"

#define NAV_DIGITALREADBUTTON \
  "<h1>Digital Read</h1>\
  <button onclick=\"loadURL('_URI1_')\">READ</button>\
  <div id=\"response\"></div>\
  <script>\
    function loadURL(url) {\
      fetch(url)\
        .then(response => response.text())\
        .then(data => {\
          const responseDiv = document.getElementById(\"response\");\
          responseDiv.innerHTML = data;\
        })\
        .catch(error => {\
          console.error(\"Error loading URL:\", error);\
        });\
    }\
  </script>"

#define NAV_ANALOGWRITEBUTTON \
  "<h1>Analog Write</h1>\
  <input type=\"range\" min=\"0\" max=\"255\" id=\"slider\">\
  <button onclick=\"loadURL('_URI1_')\">WRITE</button>\
  <div id=\"response\"></div>\
  <script>\
    function loadURL(url) {\
      const sliderValue = document.getElementById(\"slider\").value;\
      url = url + \"&value=\" + sliderValue;\
      fetch(url)\
        .then(response => response.text())\
        .then(data => {\
          const responseDiv = document.getElementById(\"response\");\
          responseDiv.innerHTML = data;\
        })\
        .catch(error => {\
          console.error(\"Error loading URL:\", error);\
        });\
    }\
  </script>"

#define NAV_ANALOGREADBUTTON \
  "<h1>Analog Read</h1>\
  <button onclick=\"loadURL('_URI1_')\">READ</button>\
  <div id=\"response\"></div>\
  <script>\
    function loadURL(url) {\
      fetch(url)\
        .then(response => response.text())\
        .then(data => {\
          const responseDiv = document.getElementById(\"response\");\
          responseDiv.innerHTML = data;\
        })\
        .catch(error => {\
          console.error(\"Error loading URL:\", error);\
        });\
    }\
  </script>"

#define NAV_CONFIG_WIFI \
  "<form action=\"/savewifi\" method=\"get\">\
<label for=\"name\">Device ID:</label>\
<input type=\"text\" id=\"id\" name=\"id\" value=\"_ID_\" required><br><br>\
<label for=\"name\">SSID:</label>\
<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"_SSID_\" required><br><br>\
<label for=\"password\">Password:</label>\
<input type=\"password\" id=\"password\" name=\"password\" value=\"_PASSWORD_\" ><br><br>\
<input type=\"submit\" value=\"Save\">\
</form>"

/***********************************
      URL Handlers
***********************************/

bool handleRoot() {
  logRequestLine();

  String html = INDEX_HTML;
  html.replace("_CONTENT_", NAV_ROOT);

  server.send(200, "text/html", html);
  return true;
}

bool handleHandshake() {
  logRequestLine();

  String log = "handshake: " + id;
  String plain = "handshake," + id;;
  Serial.println(log);
  server.send(200, "text/plain", plain);
  return true;
}

bool handleDigitalWrite() {
  logRequestLine();

  String execute = server.arg("execute");
  String pin = server.arg("pin");
  String value = server.arg("value");

  if (pin == "" && execute == "") {
    String pins = "<h1>Digital Write</h1>";
    for (int i = 1; i <= 40; i++) {
      String pin = "<button onclick=\"location.href='/digitalwrite?pin=" + String(i) + "'\">Pin " + String(i) + "</button>\r\n";
      pins += pin;
    }
    String html = INDEX_HTML;
    html.replace("_CONTENT_", pins);

    server.send(200, "text/html", html);
    return true;
  }

  if (pin != "" && execute == "") {
    String uri1 = "/digitalwrite?pin=" + pin + "&value=1&execute=true";
    String uri2 = "/digitalwrite?pin=" + pin + "&value=0&execute=true";
    
    String html = INDEX_HTML;
    html.replace("_CONTENT_", NAV_DIGITALWRITEBUTTON);
    html.replace("_URI1_" , uri1);
    html.replace("_URI2_" , uri2);

    server.send(200, "text/html", html);
    return true;    
  }

  if (execute != "") {
    if (TakeOwnership(pin.toInt(), DigitalWrite)) pinMode(pin.toInt(), OUTPUT);
    
    digitalWrite(pin.toInt(), value.toInt());
    String log = "digitalWrite: pin " + String(pin) + " set to " + String(value);
    String plain = log;
    Serial.println(log);

    server.send(200, "text/plain", plain);
    return true;
  }

  String plain = "page not here";
  server.send(404, "text/plain", plain);
  return true;
}

bool handleDigitalRead() {
  logRequestLine();

  String execute = server.arg("execute");
  String pin = server.arg("pin");

  if (pin == "" && execute == "") {
    String pins = "<h1>Digital Read</h1>";
    for (int i = 1; i <= 40; i++) {
      String pin = "<button onclick=\"location.href='/digitalread?pin=" + String(i) + "'\">Pin " + String(i) + "</button>\r\n";
      pins += pin;
    }
    String html = INDEX_HTML;
    html.replace("_CONTENT_", pins);

    server.send(200, "text/html", html);
    return true;
  }

  if (pin != "" && execute == "") {
    String uri1 = "/digitalread?pin=" + pin + "&execute=true";
    
    String html = INDEX_HTML;
    html.replace("_CONTENT_", NAV_DIGITALREADBUTTON);
    html.replace("_URI1_" , uri1);

    server.send(200, "text/html", html);
    return true;    
  }

  if (execute != "") {
    if (TakeOwnership(pin.toInt(), DigitalRead)) pinMode(pin.toInt(), INPUT);
    int value = digitalRead(pin.toInt());
    String log = "digitalRead: pin " + String(pin) + " read " + String(value);
    String plain = String(value);
    Serial.println(log);

    server.send(200, "text/plain", plain);
    return true;
  }

  String plain = "page not here";
  server.send(404, "text/plain", plain);
  return true;
}

bool handleAnalogWrite() {
  logRequestLine();

  String execute = server.arg("execute");
  String pin = server.arg("pin");
  String value = server.arg("value");

  if (pin == "" && execute == "") {
    String pins = "<h1>Analog Write</h1>";
    for (int i = 1; i <= 40; i++) {
      String pin = "<button onclick=\"location.href='/analogwrite?pin=" + String(i) + "'\">Pin " + String(i) + "</button>\r\n";
      pins += pin;
    }
    String html = INDEX_HTML;
    html.replace("_CONTENT_", pins);

    server.send(200, "text/html", html);
    return true;
  }

  if (pin != "" && execute == "") {
    String uri1 = "/analogwrite?pin=" + pin + "&execute=true";
  
    String html = INDEX_HTML;
    html.replace("_CONTENT_", NAV_ANALOGWRITEBUTTON);
    html.replace("_URI1_" , uri1);

    server.send(200, "text/html", html);
    return true;    
  }

  if (execute != "") {
    if (TakeOwnership(pin.toInt(), AnalogWrite)) pinMode(pin.toInt(), OUTPUT);
    analogWrite(pin.toInt(), value.toInt());
    String log = "analogWrite: pin " + String(pin) + " set to " + String(value);
    String plain = log;
    Serial.println(log);

    server.send(200, "text/plain", plain);
    return true;
  }

  String plain = "page not here";
  server.send(404, "text/plain", plain);
  return true;
}

bool handleAnalogRead() {
  logRequestLine();

  String execute = server.arg("execute");
  String pin = server.arg("pin");

  if (pin == "" && execute == "") {
    String pins = "<h1>Analog Read</h1>";
    for (int i = 1; i <= 40; i++) {
      String pin = "<button onclick=\"location.href='/analogread?pin=" + String(i) + "'\">Pin " + String(i) + "</button>\r\n";
      pins += pin;
    }
    String html = INDEX_HTML;
    html.replace("_CONTENT_", pins);

    server.send(200, "text/html", html);
    return true;
  }

  if (pin != "" && execute == "") {
    String uri1 = "/analogread?pin=" + pin + "&execute=true";
    
    String html = INDEX_HTML;
    html.replace("_CONTENT_", NAV_ANALOGREADBUTTON);
    html.replace("_URI1_" , uri1);

    server.send(200, "text/html", html);
    return true;    
  }

  if (execute != "") {
    if (TakeOwnership(pin.toInt(), AnalogRead)) pinMode(pin.toInt(), INPUT);
    int value = analogRead(pin.toInt());
    String log = "analogRead: pin " + String(pin) + " read " + String(value);
    String plain = String(value);
    Serial.println(log);

    server.send(200, "text/plain", plain);
    return true;
  }

  String plain = "page not here";
  server.send(404, "text/plain", plain);
  return true;
}


bool handleConfigWifi() {
  logRequestLine();

  String html = INDEX_HTML;
  html.replace("_CONTENT_", NAV_CONFIG_WIFI);
  
  html.replace("_ID_", id);
  html.replace("_SSID_", ssid);
  html.replace("_PASSWORD_", password);


  server.send(200, "text/html", html);
  return true;
}

bool handleSaveWifi() {
  logRequestLine();

  id = urldecode(server.arg("id"));
  ssid = urldecode(server.arg("ssid"));
  password = urldecode(server.arg("password"));

  saveAllConfig();

  String log = "Config written to EEPROM";
  String plain = log;
  Serial.println(log);
  server.send(200, "text/plain", plain);
  return true;
}

/***********************************
      Support functions
***********************************/

bool TakeOwnership(int pin, Owner o) {
  if (owners[pin] == o) return false;

  Serial.println("Ownership changed!");

  owners[pin] = o;
  return true;
}

String wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
}

String urldecode(const String& encoded) {
  String decoded;
  decoded.reserve(encoded.length());  // Reserve space for decoded string

  for (int i = 0; i < encoded.length(); i++) {
    char c = encoded.charAt(i);

    if (c == '+') {
      decoded += ' ';  // Replace '+' with space
    } else if (c == '%') {
      if (i + 2 < encoded.length()) {
        char hex1 = encoded.charAt(i + 1);
        char hex2 = encoded.charAt(i + 2);

        // Convert hexadecimal characters to decimal
        int value = (hex1 >= 'A' ? hex1 - 'A' + 10 : hex1 - '0') * 16 +
                    (hex2 >= 'A' ? hex2 - 'A' + 10 : hex2 - '0');

        // Add decoded character to the string
        decoded += (char)value;
        i += 2;  // Skip the encoded characters
      } else {
        // Invalid encoding: Incomplete escape sequence
        decoded += '%';  // Keep the '%' as it is
      }
    } else {
      decoded += c;  // Add the character as is
    }
  }

  return decoded;
}

void logRequestLine() {
  String fullPath = server.uri();
  String params = "";

  for (int i = 0; i < server.args(); i++) {
    if (i > 0) {
      params += "&";
    }
    params += server.argName(i) + "=" + server.arg(i);
  }

  if (!params.isEmpty()) {
    fullPath += "?" + params;
  }

  Serial.println("Request: " + fullPath);
}
