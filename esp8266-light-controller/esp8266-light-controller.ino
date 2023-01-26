/*
  HelloServerBearSSL - Simple HTTPS server example

  This example demonstrates a basic ESP8266WebServerSecure HTTPS server
  that can serve "/" and "/inline" and generate detailed 404 (not found)
  HTTP respoinses.  Be sure to update the SSID and PASSWORD before running
  to allow connection to your WiFi network.

  Adapted by Earle F. Philhower, III, from the HelloServer.ino example.
  This example is released into the public domain.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServerSecure.h>
#include <umm_malloc/umm_malloc.h>
#include <umm_malloc/umm_heap_select.h>
#include "WebSockets4WebServerSecure.h"
#include <NeoPixelBus.h>

#define NUM_LEDS 50
#define DATA_PIN 2

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(NUM_LEDS, DATA_PIN);

#ifndef STASSID
#define STASSID "Family IPhone"
#define STAPSK  "idontknow"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

BearSSL::ESP8266WebServerSecure server(443);
WebSockets4WebServerSecure webSocket;
BearSSL::ServerSessions serverCache(5);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEZTCCA02gAwIBAgIULus41V/5ZR7Rq9mDSDf5qeiBbhYwDQYJKoZIhvcNAQEL
BQAwgdMxGjAYBgNVBAoMEU9yZ2FuaXphdGlvbiBOYW1lMSEwHwYDVQQLDBhPcmdh
bml6YXRpb25hbCBVbml0IE5hbWUxHDAaBgkqhkiG9w0BCQEWDUVtYWlsIEFkZHJl
c3MxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9ybmlhMRYwFAYDVQQHDA1T
YW4gRnJhbmNpc2NvMRUwEwYDVQQKDAxCZW5lZGljdCBMdGQxIzAhBgNVBAMMGkJl
bmVkaWN0IENlcnRpZmljYXRlcyBSb290MB4XDTIzMDExNDIwMjAwMloXDTI0MDEx
NDIwMjAwMloweDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAU
BgNVBAcMDVNhbiBGcmFuY2lzY28xFTATBgNVBAoMDEJlbmVkaWN0IEx0ZDElMCMG
A1UEAwwcQmVuZWRpY3QgQ2VydGlmaWNhdGVzIFNlcnZlcjCCASIwDQYJKoZIhvcN
AQEBBQADggEPADCCAQoCggEBAPRriDvFfUT6KeuY5/hMk2tsFXPqx7dQFtA/9J+Y
cj+sO1L5NhLbZSEF23rO7S1KWFiby1b+mfBaU29xTtEomYHe/lABkjC2KHnQj3hd
DUAD9omIaqz4D8fkOW9s8WHPku9RT91M8HDcb9rj+UpIRY2omiofDHZS3ykkqtjb
PQ7uKuknQeBcOGE19AJbxAXXdAj4iqyfVixauMM97MAoipu2lLuajhz/LALnR/L7
R2Q8lnG7MuhhT/C9o0ulg5s6VUViWexRdF7Z6U+Y6uG2bomOMMBY7zWhl+NnhfM/
fydMB541NkmI0g666Z06Is4gCHFik20g1nj9sEYG8lAHAoECAwEAAaOBijCBhzAJ
BgNVHRMEAjAAMAsGA1UdDwQEAwIFoDAtBgNVHREEJjAkhwR/AAABhwQKAADIhwQK
AACdhwQKAACUhwSsFAoChwQKFCH2MB0GA1UdDgQWBBRFbYKxmadRli+D8N8QWlJL
Z5yr9TAfBgNVHSMEGDAWgBSi2kgrU+0CIf0r7miyyMWFlFzlXzANBgkqhkiG9w0B
AQsFAAOCAQEAHotKO/FJcxAzgfYNsBa88itoGYbm0Jwq1I/jfjqR6ygPm+pIAJH8
zKZ73sX3ZVaGC78QP3Ml4n3lbT4cxn1cGGt4EtbjcQK8tntCRKNISEoV2Lr0o5VF
cd0lFg/QEWoFQfVC875a0gp1l15AQbRtZEr0467BZ9pvCrdjbLD/dly6iAhtULBB
fK0gzknEAFqTqSk2NjQ/WqK3xkC9/Xu6Sntus29WMagHm/3mSE+MdT4XumTfeb2j
KpAI3a4Y4RjB2YF/tsSGufruJR4/vwy/Kavk1YdrOIly5JfyUqhEUya8civSfieN
7MKe4TF2OTrEAjWiMDELQeA4OrS66eiAJw==
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQD0a4g7xX1E+inr
mOf4TJNrbBVz6se3UBbQP/SfmHI/rDtS+TYS22UhBdt6zu0tSlhYm8tW/pnwWlNv
cU7RKJmB3v5QAZIwtih50I94XQ1AA/aJiGqs+A/H5DlvbPFhz5LvUU/dTPBw3G/a
4/lKSEWNqJoqHwx2Ut8pJKrY2z0O7irpJ0HgXDhhNfQCW8QF13QI+Iqsn1YsWrjD
PezAKIqbtpS7mo4c/ywC50fy+0dkPJZxuzLoYU/wvaNLpYObOlVFYlnsUXRe2elP
mOrhtm6JjjDAWO81oZfjZ4XzP38nTAeeNTZJiNIOuumdOiLOIAhxYpNtINZ4/bBG
BvJQBwKBAgMBAAECggEAGgTScU5yKeQZj09tLjT0ooRUekz9aqm3a/7K6yxeySFs
BUarN3Gsf8nwVc/uVppp8jDPghjDSV9ML3pX4hMY2EblOp4YMmBJycHma/E35l8l
oPlpJbWYdHZ6mL53qapfoWM+BbhZhiIpNBt9n6c25EAU/TPl7zieMfKJz2DzTIHf
wSAEL7sttXBg/5wpW4SXcO6M/Oz0BRSL3YseyS/A138Kmonju8x9DXhKUjhI4k7k
l4l+rPyxwdFM/e8pSeS8V376QZhIEThpYaJYLIpxhHXeY3DofIZEtjKkyKM4F1LF
h+gxx6FySJB+Z6JHNmmLtexfN8y+foVOrBqu6dRy4QKBgQD/+6fPNof6vnsptl8S
/caEYDrafN5jxTmg7CIM/2WHN27/4G7LRho0zTcvSQoYa1QvRw8OhpgoA2RpGR5E
8dep3/lT6mzjw7nFh16xEAnVf+uku3KjApWFBWcaVSb3DAWUviDGKTfF3nA2gOqY
xmD6sKZN9ZZ2y6eTpnrrktLuuQKBgQD0b64vd2pluGBSanl0VP+QEZ1O5EG11k/A
mHQOtfjcj0WKj3OsLHCTZuH9hx1zpJLMUWaBmvXzfXbrlsEH1oAlHz0JJGJnMi4B
sO+5dymh8Io+CB4q+ZL0VqiqdxIs0757b9wDXqu5IUYFr+h4OfQSq8xR79OtxZlK
5fzTLsKOCQKBgQDF694j6RXaWc0Aw3FfTbhgQcc/h6yrl6KnOexaAF6P3dyFGxKP
OPSqPsGcXezgVJwSse6CXK7owTaxzCoLRLJrNjGhIHBw6rSIddJRNQVlfjLEe+p2
/1h8MWE2iwBFo5XtG9RbYyMsaoB4o9S/qyLwsgTj4i8kSvKWxvHjkvJkgQKBgGr5
/ZeUgaInATDQnHsHQBnDO79tMjzsoFa/4fsWNOYCZgLwKUWu5U39duZIHeGgowZ8
kYojUYTQ4hzVC4HspISamVHI9LAJTA09bI0U08mlxjrDvsA8Hc6FpqeEGRY5TWIk
YHuxESKip0yR4gHMCz+opHEvrsqccixPX0jlbAxRAoGARLKmbrLSHxHZtBcl/p03
rdnW4DCFo0RRVqjoj026pXq2PBqvmNdA56Xn1uchcHKFtyfGm1KyXgbRMkFIcGWe
wIStbAdBatuffVhX5EqM7KMJvSM0+EBDWh4llR+aCQ9R8qHb2/R6GfbHkOK1UCxX
DGSiWvF05MlULhskcZO8p0I=
-----END PRIVATE KEY-----
)EOF";

bool isConnected = false;

void webSocketHandleEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_BIN:
            {
            // Serial.printf("[%u] get binary length: %u\n", num, length);
            // hexdump(payload, length);

            // send message to client
            float* data = (float*) payload;
            // float hue = atof(strtok((char *) payload, ":"));
            // float lightness = atof(strtok(NULL, ":"));
            float hue = data[0];
            float lightness = data[1];
            // Serial.print("Hue:");
            // Serial.println(hue);
            // Serial.print("Lightness: ");
            // Serial.println(lightness);

            strip.ClearTo(HslColor(hue, 1.0f, lightness));
            }
            break;
        case WStype_TEXT:
            {
                // Serial.printf("[%u] get Text: %s\n", num, payload);

                // send message to client
                // webSocket.sendTXT(num, payload);
                float hue = atof(strtok((char *) payload, ":"));
                float lightness = atof(strtok(NULL, ":"));
                Serial.println(hue);
                Serial.println(lightness);

                strip.ClearTo(HslColor(hue, 1.0f, lightness));

                // send data to all connected clients
                // webSocket.broadcastTXT("message here");
            }
            break;
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
              isConnected = true;
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            }
            break;
    }

}

void handleRoot() {
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS!");
}

void handleNotFound(){
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
}

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  strip.Begin();
  strip.ClearTo(HslColor(1.0f, 1.0f, 0.2));
  strip.Show();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  // Cache SSL sessions to accelerate the TLS handshake.
  server.getServer().setCache(&serverCache);

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);
  server.addHook(webSocket.hookForWebserver("/wss", webSocketHandleEvent)); // Added
  server.begin();
  strip.ClearTo(HslColor(0, 1.0f, 0));
  strip.Show();
  Serial.println("HTTPS server started");
}

void loop(void){
  server.handleClient();
  webSocket.loop();
  while (isConnected) {
    webSocket.loop();
    strip.Show();
  }
}

// https://github.com/Links2004/arduinoWebSockets/issues/25