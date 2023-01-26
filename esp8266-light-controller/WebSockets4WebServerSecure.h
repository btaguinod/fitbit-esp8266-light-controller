#ifndef __WEBSOCKETS4WEBSERVERSECURE_H
#define __WEBSOCKETS4WEBSERVERSECURE_H

#include <WebSocketsServer.h>
#include <ESP8266WebServerSecure.h>

#if WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266 && WEBSERVER_HAS_HOOK

class WebSockets4WebServerSecure : public WebSocketsServerCore {
    public:
        WebSockets4WebServerSecure(const String &origin = "", const String &protocol = "arduino") : WebSocketsServerCore(origin, protocol) {
            begin();
        }

        // Original: ESP8266WebServer::HookFunction hookForWebserver(const String &wsRootDir, WebSocketServerEvent event) {
        BearSSL::ESP8266WebServerSecure::HookFunction hookForWebserver(const String &wsRootDir, WebSocketServerEvent event) {
            onEvent(event);

            // Original: return [&, wsRootDir](const String &method, const String &url, WiFiClient *tcpClient, ESP8266WebServer::ContentTypeFunction contentType) {
            // Can't change the 'WiFiClient' type here because that is passed in by BearSSL
            return [&, wsRootDir](const String &method, const String &url, WiFiClient *tcpClient, BearSSL::ESP8266WebServerSecure::ContentTypeFunction contentType) {
                (void)contentType;
                WiFiClientSecure *tcpClientSecure = (WiFiClientSecure *)tcpClient; // Added

                if(!(method == "GET" && url.indexOf(wsRootDir) == 0)) {
                    // Original: return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
                    return BearSSL::ESP8266WebServerSecure::CLIENT_REQUEST_CAN_CONTINUE;
                }

                // Allocate a WiFiClient copy (like in WebSocketsServer::handleNewClients())
                // Original: WEBSOCKETS_NETWORK_CLASS *newTcpClient = new WEBSOCKETS_NETWORK_CLASS(*tcpClient);
                WEBSOCKETS_NETWORK_SSL_CLASS *newTcpClient = new WEBSOCKETS_NETWORK_SSL_CLASS(*tcpClientSecure);

                // Then initialize a new WSclient_t (like in WebSocketsServer::handleNewClient())
                WSclient_t *client = handleNewClient(newTcpClient);

                if(client) {
                    // Give "GET <url>"
                    String headerLine;
                    headerLine.reserve(url.length() + 5);
                    headerLine = "GET ";
                    headerLine += url;
                    handleHeader(client, &headerLine);
                }

                // Tell webserver to not close but forget about this client
                // Original: return ESP8266WebServer::CLIENT_IS_GIVEN;
                return BearSSL::ESP8266WebServerSecure::CLIENT_IS_GIVEN;
            };
        }
};

#else // WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266 && WEBSERVER_HAS_HOOK

#ifndef WEBSERVER_HAS_HOOK
#error Your current Framework / Arduino core version does not support Webserver Hook Functions
#else
#error Your Hardware Platform does not support Webserver Hook Functions
#endif

#endif // WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266 && WEBSERVER_HAS_HOOK

#endif // __WEBSOCKETS4WEBSERVERSECURE_H