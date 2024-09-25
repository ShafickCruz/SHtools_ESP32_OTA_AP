#include <SHtools_ESP32_OTA_AP.h>

// Defina os pinos do LED, do botão e o nome do sketch
const int ledPin = 23;
const int buttonPin = 27;
String nomeSketch = "Projeto teste";

// Crie uma instância da biblioteca
SHtools_ESP32_OTA_AP FirmwareUpdate(ledPin, buttonPin, nomeSketch);

void setup()
{
    // Inicialize a biblioteca
    FirmwareUpdate.begin();
}

void loop()
{
    // Chame a função handle no loop principal
    FirmwareUpdate.handle();
}
