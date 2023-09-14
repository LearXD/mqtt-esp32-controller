#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

#include <Ticker.h>

Ticker watchdog;

const char *ssid = "esp32";
const char *password = "password123";

const char *mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;

const char *mqtt_user = "myESP32";
const char *mqtt_password = "123456";

WiFiClient wifi;
PubSubClient client(wifi);

bool initialized = false;

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  String channel = String(topic);
  String message;

  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  if (channel == "esp32/led")
  {
    digitalWrite(2, message.toInt());
  }

  if (channel == "esp32/servo")
  {
    Serial.println("Servo: " + message);
  }

  if (channel == "esp32/messages")
  {
    Serial.println("Mensagem recebida: " + message);
  }

  Serial.println(message);
}

void restart()
{
  Serial.println("Reiniciando ESP32-CORE");
  ESP.restart();
}

void setup()
{
  // PREVENT ESP32-CAM CRASH
  watchdog.attach(60 * 3, restart);

  Serial.begin(115200);
  pinMode(2, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Conectando à rede Wi-Fi...");
  }
  Serial.println("Conectado à rede Wi-Fi");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect(mqtt_user, mqtt_user, mqtt_password))
    {
      Serial.println("Conectado ao broker com sucesso!");

      client.subscribe("esp32/led");
      client.subscribe("esp32/servo");
      client.subscribe("esp32/messages");

      return;
    }

    Serial.print("Falha na conexão com o broker MQTT. Tentando novamente em 5 segundos...");
    delay(5000);
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (!initialized)
  {
    initialized = true;
    client.publish("esp32/led", "0");
  }

  client.publish("esp32/cores", String(ESP.getChipCores()).c_str());
  client.publish("esp32/chip-model", ESP.getChipModel());
  client.publish("esp32/cpu-freq", String(ESP.getCpuFreqMHz()).c_str());
  client.publish("esp32/heap", String(ESP.getFreeHeap()).c_str());
  client.publish("esp32/cycle", String(ESP.getCycleCount()).c_str());

  delay(1000);
}