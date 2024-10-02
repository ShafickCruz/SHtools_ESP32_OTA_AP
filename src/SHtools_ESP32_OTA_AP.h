#ifndef SHTOOLS_ESP32_OTA_AP_H
#define SHTOOLS_ESP32_OTA_AP_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncWebSocket.h>
#include <ElegantOTA.h>
#include <pgmspace.h>    // para utilizar PROGMEM - memoria flash
#include <Preferences.h> // memoria flash

class SHtools_ESP32_OTA_AP
{
public:
    SHtools_ESP32_OTA_AP(int ledPin, int buttonPin, String nomeSketch);
    void begin();  // like setup
    void handle(); // like loop

    bool get_ServerMode() const;                            // Método getter para ServerMode
    void set_DebugInicial(bool valor);                      // Setter para DebugInicial
    void printMSG(const String &msg, bool newline = false); // Serial Monitor personalizado

private:
    bool ServerMode;
    unsigned long ServerModeInicio;
    unsigned long buttonPressTime;
    unsigned long lastButtonStateChangeTime;
    unsigned long longPressDuration;
    unsigned long debounceDelay;
    int lastButtonState;
    const char *clientPassword;
    int ledPin;
    int buttonPin;
    String nomeSketch;
    bool DebugInicial;
    unsigned long ota_progress_millis;
    AsyncWebServer server; // Declaração do servidor como membro da classe
    AsyncWebSocket ws;     // Declaração do WebSocket como membro da classe

    Preferences config;                    // instancia para configrações usando preferences
    static const char IndexHTML[] PROGMEM; // Declaração da página HTML

    void ServerMode_handle();
    void bt_handle();
    void onOTAStart();
    void onOTAProgress(size_t current, size_t final);
    void onOTAEnd(bool success);
    void startServerMode(); // Inicia o servidor AP
    bool SerialCMD(String _msg);
    void WifiSetup();
    void rotasEcallbacks();
    String generateSSID(); // Gera SSID
    // void WebSocket_sendMessage(const String &message); // Método para enviar mensagens via WebSocket
};

#endif
