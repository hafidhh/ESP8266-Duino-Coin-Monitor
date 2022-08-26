#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h> // V6!

const char *ssid = "SSID"; // Change this to your WiFi SSID
const char *password = "Password"; // Change this to your WiFi password
const String ducoUser = "username"; // Change this to your Duino-Coin username

const String ducoReportJsonUrl = "https://server.duinocoin.com/v2/users/" + ducoUser + "?limit=1";

const int run_in_ms = 5000;

float lastAverageHash = 0.0;
float lastTotalHash = 0.0;

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String httpGetString(String URL) {
  String payload = "";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  if (http.begin(client, URL)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return payload;
}

boolean runEvery(unsigned long interval) {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  setupWifi();

}

void loop() {
  if (runEvery(run_in_ms)) {
    float totalHashrate = 0;
    float avgHashrate = 0;
    int total_miner = 0;

    String input = httpGetString(ducoReportJsonUrl);
    DynamicJsonDocument doc (8000);
    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
      }

    JsonObject result = doc["result"];

    JsonObject result_balance = result["balance"];
    double result_balance_balance = result_balance["balance"];
    // const char *result_balance_created = result_balance["created"];
    const char *result_balance_username = result_balance["username"];
    // const char *result_balance_verified = result_balance["verified"];

    for (JsonObject result_miner : result["miners"].as<JsonArray>()) {
      float result_miner_hashrate = result_miner["hashrate"];
      totalHashrate = totalHashrate + result_miner_hashrate;
      total_miner++;
    }
    avgHashrate = totalHashrate / long(total_miner);

    Serial.println();
    Serial.println("result_balance_username : " + String(result_balance_username));
    Serial.println("result_balance_balance : " + String(result_balance_balance) + " DUCO");
    Serial.println("totalHashrate : " + String(totalHashrate) + " H/s");
    Serial.println("avgHashrate H/s : " + String(avgHashrate) + " H/s");
    Serial.println("total_miner : " + String(total_miner));
  }
}