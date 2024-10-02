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

    // Método getter para ServerMode
    bool get_ServerMode() const;

    // Setter para DebugInicial
    void set_DebugInicial(bool valor);

    // Métodos para WebSocket
    void WebSocket_sendMessage(const String &message); // Método para enviar mensagens via WebSocket

    void printMSG(const String &msg);

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
    void WebserialCallback(uint8_t *data, size_t len); // Processa mensagens recebidas via WebSerial
    void startServerMode();                            // Inicia o servidor AP
    bool SerialCMD(String _msg);
    void WifiSetup();
    void rotasEcallbacks();
    String generateSSID(); // Gera SSID
};

#endif
