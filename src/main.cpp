#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include <Credentials.h>

const char *wifiSSID = WIFI_SSID;
const char *wifiPASS = WIFI_PASS;

const char *mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;
const char *mqttTopic = MQTT_TOPIC;

const char *mqttClientId = MQTT_ID;
const char *mqttUser = MQTT_USER;
const char *mqttPass = MQTT_PASS;


// Выдан YandexInternalRootCA до 11 февраля 2033 г. 18:51:42
static const char ISRG_Root_x1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

BearSSL::X509List certISRG(ISRG_Root_x1);
BearSSL::WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

bool wifiConnected()
{
  // Если подключение активно, то просто выходим и возвращаем true
  if (WiFi.status() != WL_CONNECTED)
  {
    // ... иначе пробуем подключиться к сети
    Serial.print("Connecting to WiFi AP ");
    Serial.print(wifiSSID);
    Serial.print(" ");

    // Настраиваем объект WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPASS);

    // И ждем подключения 60 циклов по 0,5 сек - это 30 секунд
    int i = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      i++;
      if (i > 60)
      {
        // Если в течение этого времени не удалось подключиться - выходим с false
        Serial.println("");
        Serial.println("Connection failed!");
        return false;
      };
      Serial.print(".");
      delay(500);
    };

    // Подключение успешно установлено
    Serial.println(" ок");
    Serial.print("WiFi connected, obtained IP address: ");
    Serial.println(WiFi.localIP());

    // Для работы TLS-соединения нужны корректные дата и время, получаем их с NTP серверов
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    // Ждем, пока локальное время синхронизируется
    Serial.print("Waiting for NTP time sync: ");
    i = 0;
    time_t now = time(nullptr);
    while (now < 1000000000)
    {
      now = time(nullptr);
      i++;
      if (i > 60)
      {
        // Если в течение этого времени не удалось подключиться - выходим с false
        Serial.println("");
        Serial.println("Time sync failed!");
        return false;
      };
      Serial.print(".");
      delay(500);
    }

    // Время успешно синхронизировано, выводим его в монитор порта
    Serial.println(" ок");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));

    // Теперь можно привязать корневой сертификат к клиенту WiFi
    wifiClient.setTrustAnchors(&certISRG);
  };
  return true;
}

bool mqttConnected()
{
  if (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker: ");
    // Настраиваем MQTT клиент
    mqttClient.setServer(mqttServer, mqttPort);

    if (mqttClient.connect(mqttClientId, mqttUser, mqttPass))
    {
      Serial.println("ok");
    }
    else
    {
      Serial.print("failed, error code: ");
      Serial.print(mqttClient.state());
      Serial.println("!");
    };
    return mqttClient.connected();
  };
  return true;
}

String get_str_from_bssid(uint8_t *bssid)
{
  char buff[17];
  sprintf(
      buff, "%02X:%02X:%02X:%02X:%02X:%02X",
      bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

  return String(buff);
}

String wifiScanResultJSON()
{
  const int scanResult = WiFi.scanNetworks(false, true);

  if (scanResult > 0)
  {
    String ssid;
    int32_t rssi;
    uint8_t encryptionType;
    uint8_t *bssid;
    int32_t channel;
    bool hidden;

    StaticJsonDocument<1024> arr_doc;
    JsonArray array = arr_doc.to<JsonArray>();

    for (int8_t i = 0; i < scanResult; i++)
    {
      DynamicJsonDocument doc(200);

      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);

      doc["ssid"] = ssid;
      doc["bssid"] = get_str_from_bssid(bssid);

      const bss_info *bssInfo = WiFi.getScanInfoByIndex(i);
      String phyMode;
      const char *wps = "";
      if (bssInfo)
      {
        phyMode.reserve(12);
        phyMode = F("802.11");
        String slash;
        if (bssInfo->phy_11b)
        {
          phyMode += 'b';
          slash = '/';
        }
        if (bssInfo->phy_11g)
        {
          phyMode += slash + 'g';
          slash = '/';
        }
        if (bssInfo->phy_11n)
        {
          phyMode += slash + 'n';
        }
        if (bssInfo->wps)
        {
          wps = PSTR("WPS");
        }
      }

      array.add(doc);

      yield();
    }

    String result;

    serializeJson(array, result);
    return result;
  } 

  return "";
}

void setup()
{
  Serial.begin(115200);
  
}

void loop()
{
  String scanResult = wifiScanResultJSON();
  Serial.println();
  Serial.println(scanResult);

  // if (wifiConnected() && mqttConnected()){
  //   if (mqttClient.publish(mqttTopic, scanResult.c_str())){
  //     Serial.println("uploaded");
  //   } else {
  //     Serial.println("not uploaded");
  //   }
  // }

  if (wifiConnected() && mqttConnected()){
    if (mqttClient.publish(mqttTopic, "113")){
      Serial.println("uploaded");
    } else {
      Serial.println("not uploaded");
    }
  }

  delay(10000);
}