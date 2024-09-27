#ifndef SHTOOLS_ESP32_OTA_AP_H
#define SHTOOLS_ESP32_OTA_AP_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ElegantOTA.h>
#include <WebSerial.h>
#include <pgmspace.h> // para utilizar PROGMEM

class SHtools_ESP32_OTA_AP
{
public:
    SHtools_ESP32_OTA_AP(int ledPin, int buttonPin, String nomeSketch, bool Debuginicial);
    void begin();  // Inicializa a configuração inicial
    void handle(); // Deve ser chamado no loop principal para processar OTA e verificar o botão

    // Método getter para otaMode
    bool get_ServerMode() const;

private:
    bool ServerMode;
    unsigned long buttonPressTime;
    unsigned long lastButtonStateChangeTime;
    unsigned long longPressDuration;
    unsigned long debounceDelay;
    int lastButtonState;
    const char *clientPassword;
    int ledPin;
    int buttonPin;
    String nomeSketch;
    bool Debuginicial;
    unsigned long ota_progress_millis;
    AsyncWebServer server;                 // Declaração do servidor como membro da classe
    static const char IndexHTML[] PROGMEM; // Declaração da página HTML

    void led_handle();
    void bt_handle();
    void onOTAStart();
    void onOTAProgress(size_t current, size_t final);
    void onOTAEnd(bool success);
    void startServerMode(); // Inicia o servidor AP
    void WifiSetup();
    void rotasEcallbacks();
    String generateSSID(); // Gera SSID
};

#endif
