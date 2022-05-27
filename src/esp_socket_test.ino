#include <ESP8266WiFi.h>
#include <AsyncTimer.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

AsyncTimer t;
SocketIOclient io;

// WiFi credentials
const char *ssid = "Cimeng";
const char *password = "poponispiyo";

bool ledStatus = false;

void onSocketLoop()
{
  if (!io.isConnected())
    return;

  DynamicJsonDocument json(1024);
  json[0] = "state_update";
  json[1]["led_status"] = ledStatus;

  String payload;
  serializeJson(json, payload);

  io.sendEVENT(payload);
}

void onSocketConnected()
{
  Serial.println("Socket connected!");
  io.sendEVENT("[\"client\", \"esp\"]");
}

void onEventReceived(uint8_t *payload)
{
  DynamicJsonDocument json(1024);
  deserializeJson(json, payload);

  const char *event = json[0];

  if (strcmp(event, "state_update") == 0)
  {
    ledStatus = json[1]["led_status"];
    digitalWrite(LED_BUILTIN, ledStatus ? LOW : HIGH);
  }
}

void wifi_connect()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case sIOtype_CONNECT:
    io.send(sIOtype_CONNECT, "/");
    onSocketConnected();
    break;
  case sIOtype_EVENT:
    onEventReceived(payload);
    break;
  }
}

void socket_connect()
{
  // Initialize socket
  io.begin("192.168.1.55", 8080, "/socket.io/?EIO=4");
  io.onEvent(socketIOEvent);
  t.setInterval(onSocketLoop, 1000);
}

void setup()
{
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  wifi_connect();
  t.setInterval(wifi_connect, 30 * 1000);
  socket_connect();
}

void loop()
{
  t.handle();
  io.loop();
}
