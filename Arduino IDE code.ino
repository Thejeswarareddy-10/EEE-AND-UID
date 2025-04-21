#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "noothan";
const char* password = "noothan123";

// DS18B20 on GPIO4
#define ONE_WIRE_BUS 4
#define MOSFET_PIN 14

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WebServer server(80);

bool isPeltierOn = false;
float tempC = 0;
bool loggedIn = false;

void setup() {
  Serial.begin(115200);
  sensors.begin();

  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Routes
  server.on("/", handleLoginPage);
  server.on("/login", handleLogin);
  server.on("/temperature", handleTemperature);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);

  if (tempC > 25 && !isPeltierOn) {
    digitalWrite(MOSFET_PIN, HIGH);
    isPeltierOn = true;
    Serial.println("Peltier ON");
  }

  server.handleClient();
  delay(2000);
}

// Login Page
void handleLoginPage() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Login - ESP32 Temp Monitor</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 100px; background: #f4f4f4; }
    .box { background: white; padding: 40px; border-radius: 10px; display: inline-block; box-shadow: 0 0 10px rgba(0,0,0,0.2); }
    input { padding: 10px; width: 200px; margin: 10px; border: 1px solid #ccc; border-radius: 5px; }
    button { padding: 10px 20px; background: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; }
    button:hover { background: #0056b3; }
    .error { color: red; }
  </style>
</head>
<body>
  <div class="box">
    <h2>Login</h2>
    <form action="/login" method="GET">
      <input type="text" name="username" placeholder="Username"><br>
      <input type="password" name="password" placeholder="Password"><br>
      <button type="submit">Login</button>
    </form>
  </div>
</body>
</html>
)rawliteral");
}

// Handle login credentials
void handleLogin() {
  String username = server.arg("username");
  String password = server.arg("password");

  if (username == "EEE" && password == "UID") {
    loggedIn = true;
    server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Temperature Monitor</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    .temp { font-size: 2.5em; margin-top: 20px; }
  </style>
</head>
<body>
  <h1>WELCOME TO TEMP MONITERING</h1>
  <div class="temp" id="temperature">Loading...</div>

  <script>
    setInterval(() => {
      fetch('/temperature')
        .then(response => response.text())
        .then(data => {
          document.getElementById('temperature').innerText = data + ' °C';
        });
    }, 2000);
  </script>
</body>
</html>
)rawliteral");
  } else {
    server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Login Failed</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 100px; background: #f4f4f4; }
    .box { background: white; padding: 40px; border-radius: 10px; display: inline-block; box-shadow: 0 0 10px rgba(0,0,0,0.2); }
    input { padding: 10px; width: 200px; margin: 10px; border: 1px solid #ccc; border-radius: 5px; }
    button { padding: 10px 20px; background: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; }
    button:hover { background: #0056b3; }
    .error { color: red; }
  </style>
</head>
<body>
  <div class="box">
    <h2>Login Failed</h2>
    <p class="error">Please check your username and password</p>
    <form action="/login" method="GET">
      <input type="text" name="username" placeholder="Username"><br>
      <input type="password" name="password" placeholder="Password"><br>
      <button type="submit">Try Again</button>
    </form>
  </div>
</body>
</html>
)rawliteral");
  }
}

// Serve temperature data
void handleTemperature() {
  if (loggedIn) {
    server.send(200, "text/plain", String(tempC));
  } else {
    server.send(403, "text/plain", "Not authorized");
  }
}
