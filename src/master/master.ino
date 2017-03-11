#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ota.h>

WiFiEventHandler softAPModeStationConnected, softAPModeStationDisconnected;

ESP8266WebServer server(80);

int EEPROM_CRED = 120;

void setup()
{
    Serial.begin(115200);
    Serial.println("Iniciando Access Point");
    EEPROM.begin(EEPROM_CRED);

    WiFi.mode(WIFI_AP_STA);

    startAccessPoint();
    readCredentials();
    initOTA();
}

void loop() {
    server.handleClient();
    ArduinoOTA.handle();
}

void startAccessPoint(){
    WiFi.softAP("Central", "123456789");

    softAPModeStationConnected = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected& event)
    {
        Serial.println("Nova cliente conectado via WiFi..");
    });

    softAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([](const WiFiEventSoftAPModeStationDisconnected& event)
    {
        Serial.println("Novo cliente desconectado via WiFi..");
    });

    server.on("/networks", networksPage);
    server.on("/connect", connectPage);
    server.on("/credentials", credentialsPage);
    server.on("/cmd", cmdPage);
    server.begin();
}

void cmdPage(){
    String to = server.arg("to");
    String cmd = server.arg("cmd");
    String val = server.arg("value");
    String extra = server.arg("extra");
    String url = "/cmd?" + cmd + "=" + val + extra;

    String res = makeRequest(to.c_str(), url.c_str());

    server.send(200, "application/json", "{'success': 'Command accepted', 'url': '" + url + "', 'response': '" + res + "'}");
}

String makeRequest(const char *host, const char *url){
    WiFiClient client;
    String lines;

    const int httpPort = 80;

    if (!client.connect(host, httpPort)) {
        return "Connection failed!";
    }

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();

    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            client.stop();
            return ">>> Client Timeout !";
        }
    }

    while(client.available()){
        lines += client.readStringUntil('\r');
    }

    return lines;
}

void credentialsPage(){
    readCredentials();

    server.send(200, "application/json", "{'success': 'credentials command run'}");
}

void connectPage(){
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    char ssidChar[120];
    ssid.toCharArray(ssidChar, 120);

    char passwordChar[120];
    password.toCharArray(passwordChar, 120);

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    if(ssid != "" && password != ""){
        connectWifi(ssidChar, passwordChar);
    } else {
        server.send(200, "application/json", "{'error': 'No SSID and Password found'}");
    }
}

void networksPage(){
    int networksFound = WiFi.scanNetworks();

    Serial.printf("%d network(s) found\n", networksFound);
    String htmlResult;

    for (int i = 0; i < networksFound; i++)
    {
        Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");

        if(networksFound > 0){
            htmlResult += "[";
              htmlResult += "{ 'POS': '" + String(i + 1) + "' },";
              htmlResult += "{ 'SSID': '" + String( WiFi.SSID(i).c_str() ) + "'},";
              htmlResult += "{ 'CHANNEL': '" + String( WiFi.channel(i) ) + "'},";
              htmlResult += "{ 'DBM': '" + String( WiFi.RSSI(i) ) + "'},";
              htmlResult += "{ 'ENC': '" + String( WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "close" ) + "'}";

            if((i + 1) == networksFound){
                htmlResult += "]";
            } else {
                htmlResult += "],";
            }
        }
    }

    server.send(200, "application/json", htmlResult);
}


void connectWifi(char *network, char *password){
    int tries = 0;

    WiFi.begin(network, password);

    while (WiFi.status() != WL_CONNECTED && tries < 10)
    {
        delay(1000);
        tries++;
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.println("Connected, IP address: " + getIP());

        saveCredentials(String(network), String(password));
        initOTA();

        server.send(200, "application/json", "{'success': 'Connected!', 'IP:': '" + getIP() + "'}");
    } else {
        WiFi.disconnect();
        server.send(500, "application/json", "{'error': 'I can't connected! :('}");
    }

}

String getIP(){
    return WiFi.localIP().toString();
}

void saveCredentials(String ssid, String password){
    String credentials = ssid + "|" + password;

    int charLength = credentials.length();

    Serial.println("writing eeprom credentials:");

    for (int i = 0; i < EEPROM_CRED; ++i)
    {
        if(i <= charLength){
            EEPROM.write(i, credentials[i]);
            Serial.print("Wrote: ");
            Serial.println(credentials[i]);
        } else {
            EEPROM.write(i, " "[0]);
        }

    }

    EEPROM.commit();
}

String readCredentials(){
    Serial.println("Reading EEPROM credentials");

    String esid;

    for (int i = 0; i < 240; ++i)
    {
        if(String(char(EEPROM.read(i))) != " "){
            esid += char(EEPROM.read(i));
        }
    }

    esid.trim();

    Serial.println(esid.length());
    Serial.print("Credenstials: ");
    Serial.println(esid);

    return esid;
}
