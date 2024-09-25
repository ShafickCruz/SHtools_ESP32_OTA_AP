#ifndef SHTOOLS_ESP32_OTA_AP_H
#define SHTOOLS_ESP32_OTA_AP_H

#include <Arduino.h>
#include <WiFi.h>
#include <ElegantOTA.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>

class SHtools_ESP32_OTA_AP
{
public:
    SHtools_ESP32_OTA_AP(int ledPin, int buttonPin, String nomeSketch);
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
    unsigned long ota_progress_millis;
    AsyncWebServer server; // Declaração do servidor como membro da classe

    String style;
    String loginIndex;
    String otaIndex;

    void led_handle();
    void bt_handle();
    void onOTAStart();
    void onOTAProgress(size_t current, size_t final);
    void onOTAEnd(bool success);

    void startServerMode(); // Inicia o servidor AP
    void WifiSetup();
    String generateSSID(); // Gera SSID
};

#endif
