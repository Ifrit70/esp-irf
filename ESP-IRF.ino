#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <IRsend.h>   //Lib to send IR code
#include <RCSwitch.h> //Lib for RF control power-plug

// Add this library: https://github.com/markszabo/IRremoteESP8266
#include <IRremoteESP8266.h>

#define IR_SEND_PIN 4 //D2 on nodeMCU
#define RFTX_PIN 12 //D1 on nodeMCU

#define DELAY_BETWEEN_COMMANDS 500

IRsend irsend(IR_SEND_PIN);

RCSwitch mySwitch = RCSwitch();

const char* ssid = "your_wifi_ssid";
const char* password = "your_password";

ESP8266WebServer server(80);

const int led = BUILTIN_LED;

const char* updateIndex = "<form method='POST' action='/updating' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

String rowDiv = "    <div class=\"row\" style=\"padding-bottom:1em\">\n";
String endDiv = "    </div>\n";

// Buttons are using the bootstrap grid for sizing - http://getbootstrap.com/css/#grid
String generateButton(String colSize, String id, String text, String url) {

  return  "<div class=\"" + colSize + "\" style=\"text-align: center\">\n" +
          "    <button id=\"" + id + "\" type=\"button\" class=\"btn btn-default\" style=\"width: 100%\" onclick='makeAjaxCall(\"" + url + "\")'>" + text + "</button>\n" +
          "</div>\n";
}

void handleRoot() {
  digitalWrite(led, 0);
  String website = "<!DOCTYPE html>\n";
  website = website + "<html>\n";
  website = website + "  <head>\n";
  website = website + "    <meta charset=\"utf-8\">\n";
  website = website + "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n";
  website = website + "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  website = website + "    <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">\n";
  website = website + "  </head>\n";
  website = website + "  <body>\n";
  website = website + "    <div class=\"container-fluid\">\n";
  // ------------------------- Power Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-6", "tvpower","TV Power", "tvpower");
  website = website + generateButton("col-xs-6", "rxpower","RX-V477 Power", "rxpower");
  website = website + endDiv;
  // ------------------------- Inpput Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-3", "sschannel1","TV", "sschannel1");
  website = website + generateButton("col-xs-3", "sschannel2","AV1 (Scart)", "sschannel2");
  website = website + generateButton("col-xs-3", "sschannel3","AV2 (Component)", "sschannel3");
  website = website + generateButton("col-xs-3", "sschannel4","RX-V477", "sschannel4");
  website = website + endDiv;
  // ------------------------- TV Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-6", "tvvolup","TV Vol Up", "tvvolup");
  website = website + generateButton("col-xs-6", "tvprogup","TV Prog Up", "tvprogup");
  website = website + endDiv;
  website = website + rowDiv;
  website = website + generateButton("col-xs-6", "tvvoldown","TV Vol Down", "tvvoldown");
  website = website + generateButton("col-xs-6", "tvprogdown","TV Prog Down", "tvprogdown");
  website = website + endDiv;
  website = website + rowDiv;
  website = website + generateButton("col-xs-12", "tvup","^", "tvup");
  website = website + endDiv;
  website = website + rowDiv;
  website = website + generateButton("col-xs-4", "tvleft","<", "tvleft");
  website = website + generateButton("col-xs-4", "tvok","OK", "tvok");
  website = website + generateButton("col-xs-4", "tvright",">", "tvright");
  website = website + endDiv;
  website = website + rowDiv;
  website = website + generateButton("col-xs-12", "tvdown","v", "tvdown");
  website = website + endDiv;
  // ------------------------- RfSwitch Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-3", "rfswitch1on","RF Switch 1 On", "rfswitch1on");
  website = website + generateButton("col-xs-3", "rfswitch1off","RF Switch 1 Off", "rfswitch1off");
  website = website + generateButton("col-xs-3", "rfswitch2on","RF Switch 2 On", "rfswitch2on");
  website = website + generateButton("col-xs-3", "rfswitch2off","RF Switch 2 Off", "rfswitch2off");
  website = website + endDiv;
  // ------------------------- End --------------------------
  website = website + endDiv;
  website = website + "    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\n";
  website = website + "    <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>\n";
  website = website + "    <script> function makeAjaxCall(url){$.ajax({\"url\": url})}</script>\n";
  website = website + "    <a href=\"/update\">Firmware Update</a>\n";
  website = website + "  </body>\n";
  website = website + "</html>\n";

  server.send(200, "text/html", website);
  digitalWrite(led, 1);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 1);
}

void setup(void){
  irsend.begin();
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
  
  Serial.begin(115200);

  mySwitch.enableTransmit(RFTX_PIN);
  // Optional set number of transmission repetitions.
  //mySwitch.setRepeatTransmit(2);
  // Optional set pulse length.
  //mySwitch.setPulseLength(320);
  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS Responder Started");
  }

  server.on("/", handleRoot);

  server.on("/tvvoldown", [](){
    Serial.println("TV Sound Down");
    irsend.sendNEC(0x20DFC03F, 32);
    server.send(200, "text/plain", "TV Volume Down");
  });

  server.on("/tvvolup", [](){
    Serial.println("TV Sound Up");
    irsend.sendNEC(0x20DF40BF, 32);
    server.send(200, "text/plain", "TV Volume Up");
  });
  
  server.on("/tvprogdown", [](){
    Serial.println("Program Down");
    irsend.sendNEC(0x20DF807F, 32);
    server.send(200, "text/plain", "TV Program Down");
  });

  server.on("/tvprogup", [](){
    Serial.println("Program Up");
    irsend.sendNEC(0x20DF00FF, 32);
    server.send(200, "text/plain", "TV Program Up");
  });

  server.on("/tvdown", [](){
    Serial.println("TV Down");
    irsend.sendNEC(0x20DF827D, 32);
    server.send(200, "text/plain", "TV Down");
  });

  server.on("/tvup", [](){
    Serial.println("TV Up");
    irsend.sendNEC(0x20DF02FD, 32);
    server.send(200, "text/plain", "TV Up");
  });

  server.on("/tvleft", [](){
    Serial.println("TV Left");
    irsend.sendNEC(0x20DFE01F, 32);
    server.send(200, "text/plain", "TV Left");
  });

  server.on("/tvright", [](){
    Serial.println("TV Right");
    irsend.sendNEC(0x20DF609F, 32);
    server.send(200, "text/plain", "TV Right");
  });

  server.on("/tvok", [](){
    Serial.println("TV OK");
    irsend.sendNEC(0x20DF22DD, 32);
    server.send(200, "text/plain", "TV OK");
  });

  server.on("/sschannel1", [](){
    Serial.println("TV Antenna");
    irsend.sendNEC(0x20DF0FF0, 32);
    server.send(200, "text/plain", "TV Antenna");
  });

  server.on("/sschannel2", [](){
    Serial.println("TV AV1 (SCART)");
    irsend.sendNEC(0x20DF0FF0, 32);//back to antenna input
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);//open input menu
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);//go to 2nd input
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DF22DD, 32);//Valid input selection
    server.send(200, "text/plain", "AV1 (Scart)");
  });

  server.on("/sschannel3", [](){
    Serial.println("TV AV2 (Component)");
    irsend.sendNEC(0x20DF0FF0, 32);//back to antenna input
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);//open input menu
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);//go to 2nd input
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DF22DD, 32);//Valid input selection
    server.send(200, "text/plain", "AV2 (Component)");
  });

  server.on("/sschannel4", [](){
    Serial.println("TV RX-V477 Input");
    irsend.sendNEC(0x20DF0FF0, 32);//back to antenna input
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);//open input menu
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);//go to 2nd input
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DFD02F, 32);
    delay(DELAY_BETWEEN_COMMANDS);
    irsend.sendNEC(0x20DF22DD, 32);//Valid input selection
    server.send(200, "text/plain", "RX-V477");
  });

  server.on("/tvpower", [](){
    Serial.println("TV power");
    irsend.sendNEC(0x20DF10EF, 32);
    server.send(200, "text/plain", "TV Power");
  });

  server.on("/togglesource", [](){
    Serial.println("TV/Rad");
    irsend.sendNEC(0x20DF0FF0, 32);
    server.send(200, "text/plain", "TV/Rad");
  });

  server.on("/rxpower", [](){
    Serial.println("RX-V477 Power");
    irsend.sendNEC(0x7E8154AB, 32);
    server.send(200, "text/plain", "RX-V477 Power");
  });

  server.on("/rfswitch1on", [](){
    Serial.println("RF Switch 1 On");
    mySwitch.switchOn("00000", "10000");
    server.send(200, "text/plain", "RF Switch 1 On");
  });
  
  server.on("/rfswitch1off", [](){
    Serial.println("RF Switch 1 Off");
    mySwitch.switchOff("00000", "10000");
    server.send(200, "text/plain", "RF Switch 1 Off");
  });
  
  server.on("/rfswitch2on", [](){
    Serial.println("RF Switch 2 On");
    mySwitch.switchOn("00000", "01000");
    server.send(200, "text/plain", "RF Switch 2 On");
  });
  
  server.on("/rfswitch2off", [](){
    Serial.println("RF Switch 2 Off");
    mySwitch.switchOff("00000", "01000");
    server.send(200, "text/plain", "RF Switch 2 Off");
  });


  server.on("/update", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/html", updateIndex);
  });
  server.on("/updating", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
    HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop(void){
  server.handleClient();
}

