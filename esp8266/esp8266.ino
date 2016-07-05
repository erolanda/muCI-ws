#include <FS.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <DNSServer.h>  
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include <SoftwareSerial.h>

SoftwareSerial swSer(5, 4, false, 256); //rx tx

String inputString = "";         // a string to hold incoming data

WebSocketsClient webSocket;
// Update these with values suitable for your network.
//const char* ssid = "BrokenBrains";
//const char* password = "1234567278";
//const char* ws = "192.168.2.116"; // ip addres of nodejs server

//const char* ssid = "hackmeifucan";
//const char* password = "Bpy()[wmmt]w#j$Q}X8S58$yH^:Hx5t)cTmSAad~SR*9z";
//const char* ws = "192.168.0.2"; // ip addres of nodejs server

unsigned long lastMsg = 0;
int cont = 0;
const long interval = 10;

bool ban = false;

#define packetSize (20)

//define your default values here, if there are different values in config.json, they are overwritten.
char ws_server[40];
char ws_port[6] = "8080";
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    case WStype_DISCONNECTED:
        Serial.println("Desconectado");
      break;
    case WStype_CONNECTED:{
        Serial.println("Conectado");
      }
      break;
    case WStype_TEXT:
        //Serial.printf("Recibi: %s\n", payload);
        ban = !ban;
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  swSer.begin(115200);
  //read configuration from FS json
  Serial.println("mounting FS...");
  if(SPIFFS.begin()){
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(ws_server, json["ws_server"]);
          strcpy(ws_port, json["ws_port"]);
        }else{
          Serial.println("failed to load json config");
        }
      }
    }
  }else{
    Serial.println("failed to mount FS");
  }

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_ws_server("server", "ws server", ws_server, 40);
  WiFiManagerParameter custom_ws_port("port", "ws port", ws_port, 5);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_ws_server);
  wifiManager.addParameter(&custom_ws_port);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(20);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(ws_server, custom_ws_server.getValue());
  strcpy(ws_port, custom_ws_port.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["ws_server"] = ws_server;
    json["ws_port"] = ws_port;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  //Configure connection to ws server
  webSocket.begin(ws_server, atoi(ws_port));
  webSocket.onEvent(webSocketEvent);
  wifi_set_sleep_type(NONE_SLEEP_T);
}

void loop() {
  while (swSer.available() > 0) {
    char inChar = (char)swSer.read();
    if (inChar == '\n') {
      webSocket.sendTXT(inputString);
      //Serial.print(inputString);
      inputString = "";
    }else{
      inputString += inChar;
    }
  }
}
