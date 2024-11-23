#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "leSsid";
const char* password = "leSsidPass";
const char* mqtt_server = "192.168.1.2";
const int mqtt_port = 1883;
const char* mqtt_user = "leMqttUser";
const char* mqtt_password = "leMqttPass";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Send a message every 10 seconds
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    String message = "Hello from ESP32";
    client.publish("esp32/sensor", message.c_str());
    Serial.println(message);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("esp32/status", "connected");
      // Subscribe to the command topic
      client.subscribe("esp32/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, int length) {
  static bool state;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Print the received payload
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Check if the topic matches the expected topic
  if (strcmp(topic, "esp32/command") == 0) {
    // Parse the payload and take action
    String command = "";
    for (int i = 0; i < length; i++) {
      command += (char)payload[i];
    }

    Serial.print("Received command: ");
    Serial.println(command);

    // Take action based on the command
    if (command == "ON") {
      // Turn on the LED or perform other actions
      if (state) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("LED turned on");
        state = 0;  
      }
      else {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("LED turned off");
        state = 1;
      }

    } else if (command == "OFF") {
      // Turn off the LED or perform other actions
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED turned off");
    }
  }
}