#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "orangepi";
const char *password = "orangepi";
const char *mqtt_server = "10.29.0.1";
const int mqtt_port = 1803;
const char *mqtt_id = "esp8266";

#define Light_Pin 14
#define Fan_Pin 13 

WiFiClient client;
PubSubClient mqtt_client(client);

bool connectWiFi() {
  Serial.print("Connecting to WiFi... ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    return true;
  } else {
    Serial.println("\nWiFi connection failed");
    return false;
  }
}


bool connectMQTT() {
  Serial.print("Connecting to MQTT... ");
  if (mqtt_client.connect(mqtt_id)) {
    Serial.println("MQTT connected");
    mqtt_client.subscribe("Light");
    mqtt_client.subscribe("Fan");
    mqtt_client.publish("from-esp8266", "Hello Server");
    return true;
  }
  Serial.println("MQTT connection failed");
  return false;
}

void reconnect() {
  if (!mqtt_client.connected()) {
    while (!connectWiFi() || !connectMQTT()) {
      delay(1000); // Chờ 1 giây trước khi thử kết nối lại
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received data from: ");
  Serial.println(topic);
  Serial.print("Message: ");
  
  String message = "";
  for (size_t i = 0; i < length; i++) {
    message += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("------------------------------------------");

  if (String(topic) == "Light") {
    if (message.equals("on")) {
      digitalWrite(Light_Pin, HIGH);
    } else if (message.equals("off")) {
      digitalWrite(Light_Pin, LOW);
    }
  } else if (String(topic) == "Fan") {
    if (message.equals("on")) {
      analogWrite(Fan_Pin, 128);
    } else if (message.equals("off")) {
      analogWrite(Fan_Pin, 0);
    }else if (message.equals("level 1")) {
      analogWrite(Fan_Pin, 64);
    }else if (message.equals("level 2")) {
      analogWrite(Fan_Pin, 128);
    }else if (message.equals("level 3")) {
      analogWrite(Fan_Pin, 192);
    }else if (message.equals("level 4")) {
      analogWrite(Fan_Pin, 255);
    }    
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(Light_Pin, OUTPUT);
  pinMode(Fan_Pin, OUTPUT);
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(callback);
  // Connect to WiFi and MQTT
  reconnect();
}

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
}
