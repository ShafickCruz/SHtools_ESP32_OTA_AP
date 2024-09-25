# SHtools_ESP32_OTA_AP

A biblioteca `SHtools_ESP32_OTA_AP` permite que o ESP32 receba atualizações de firmware via Wi-Fi. Com ela, você pode atualizar o firmware do seu dispositivo de forma remota, sem necessidade de uma rede Wi-Fi intermediária e internet, usando um botão para iniciar o processo de atualização.

**Prática**: Ao pressionar o botão de atualização por 3 segundos, um LED começa a piscar e um servidor web assíncrono em modo access point é criado. Você deve acessar a rede Wi-Fi criada e, no navegador, inserir o IP do ESP32 que está no SSID. Em seguida, selecione o arquivo binário (.bin) e faça o upload para o ESP32.

## Recursos

- **OTA via Wi-Fi**: Faça atualizações de firmware através de uma rede Wi-Fi e da tecnologia OTA.
- **Modo Async**: Utilize esses recursos de forma assíncrona através de um `AsyncWebServer` criado pela biblioteca `ESPAsyncWebServer`.
- **Modo AP**: Elimine a necessidade de uma rede Wi-Fi intermediária através do Access Point criado pelo ESP32.
- **Indicação Visual**: Use um LED para sinalizar o status da atualização e quando o modo OTA está ativo.

## Instalação

### Usando o Gerenciador de Bibliotecas do Arduino

1. Abra o Arduino IDE.
2. Vá para **Sketch** > **Include Library** > **Manage Libraries...**.
3. Na barra de pesquisa, digite `SHtools_ESP32_OTA_AP`.
4. Selecione a biblioteca e clique em **Install**.

### Usando PlatformIO

Adicione a seguinte linha ao seu `platformio.ini`:
lib_deps = ayushsharma82/ElegantOTA@3.1.5

### Dependências

Certifique-se de que as seguintes bibliotecas estão instaladas em seu projeto:

```
<Arduino.h>
<WiFi.h>
<AsyncTCP.h>
<ESPAsyncWebServer.h>
<ElegantOTA.h>
```

## Uso

### Exemplo Básico

Aqui está um exemplo de como usar a biblioteca para configurar o ESP32 para atualizações OTA via Wi-Fi:

```
#include <SHtools_ESP32_OTA_AP.h>

// Defina os pinos do LED, do botão e o nome do sketch
const int ledPin = 23;
const int buttonPin = 27;
String nomeSketch = "Projeto teste";

// Crie uma instância da biblioteca
SHtools_ESP32_OTA_AP FirmwareUpdate(ledPin, buttonPin, nomeSketch);

void setup() {
  // Inicialize a biblioteca
  FirmwareUpdate.begin();
}

void loop() {
  // Chame a função handle no loop principal
  FirmwareUpdate.handle();
}
```
