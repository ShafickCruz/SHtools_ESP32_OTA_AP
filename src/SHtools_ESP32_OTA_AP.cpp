#include "SHtools_ESP32_OTA_AP.h"

// pagina HTML do index
const char SHtools_ESP32_OTA_AP::IndexHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang='pt'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>ESP32 Control Panel</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; margin-top: 50px; }
    h1 { color: #333; }
    .button { display: inline-block; padding: 15px 25px; font-size: 18px; margin: 10px; cursor: pointer; 
              border: none; border-radius: 5px; background-color: #007bff; color: white; text-decoration: none; 
              min-width: 150px; }
    .button:hover { background-color: #0056b3; }
  </style>
  <script>
    function showAlertAndRedirect() {
      alert('ATENÇÃO!!! Na próxima página, ao selecionar o arquivo de firmware, ele será enviado automaticamente, sem a necessidade de clicar em botão de envio.');
      window.location.href = '/update';
    }
  </script>
</head>
<body>
  <h1>SHtools ESP32 UPDATE</h1>
  <a href='javascript:void(0)' class='button' onclick='showAlertAndRedirect()'>Update OTA</a><br>
  <a href='/webserial' class='button'>Monitor Serial</a><br>
  <button class='button' onclick="alert('TODO: Info page')">Informações</button>
</body>
</html>
)rawliteral";

SHtools_ESP32_OTA_AP::SHtools_ESP32_OTA_AP(int ledPin, int buttonPin, String nomeSketch, bool _debugInicial = false)
    : ServerMode(false), ServerModeInicio(0), buttonPressTime(0), lastButtonStateChangeTime(0), longPressDuration(3000),
      debounceDelay(50), lastButtonState(HIGH), ledPin(ledPin), buttonPin(buttonPin),
      nomeSketch(nomeSketch), Debuginicial(_debugInicial), ota_progress_millis(0), server(80) {}

void SHtools_ESP32_OTA_AP::begin()
{
    // Configura os pinos de I/O
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);

    // se a serial não foi iniciada pelo cliente, inicia a serial
    if (!Serial)
    {
        Serial.begin(115200);
        delay(1000);
    }

    // desconecta WiFi e webserver se estiver conectado,
    // porque WebSerial pode iniciar o wifi/server internamente na criação da instância.
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();
        delay(2000);

        if (WiFi.status() == WL_DISCONNECTED)
            server.end();
    }
}

void SHtools_ESP32_OTA_AP::handle()
{
    /*
    Se estiver no modo Servidor, faz o LED piscar continuamente e processa as requisições. Se não estiver, verifica se debug inicial está habilitado.
    Se debug inicial estiver habilitado, inicia o processo de ServerMode e ignora o botão e se não estiver, keep watching o botão.
    */

    if (ServerMode)
    {
        ServerMode_handle();

        // Se está em modo servidor há mais de 30 minutos, reinicia o esp para sair do modo servidor
        unsigned long cTime = millis();
        if ((cTime - ServerModeInicio) >= 1800000) // 30 minutos
        {
            ESP.restart(); // Reinicia o ESP32
        }
    }
    else
    {
        if (Debuginicial)
        {
            startServerMode();
        }
        else
        {
            bt_handle();
        }
    }
}

void SHtools_ESP32_OTA_AP::ServerMode_handle()
{
    ElegantOTA.loop(); // processa as requisições OTA
    WebSerial.loop();  // processa as requisições Webserial

    unsigned long cTime = millis(); // Captura o tempo atual

    // Faz o LED piscar continuamente
    static unsigned long lastBlinkTime = 0;
    unsigned long blinkInterval = 150; // Piscar a cada 250ms
    cTime = millis();

    if ((cTime - lastBlinkTime) >= blinkInterval)
    {
        digitalWrite(ledPin, !digitalRead(ledPin)); // Inverte o estado do LED
        lastBlinkTime = cTime;
    }
}

void SHtools_ESP32_OTA_AP::bt_handle()
{
    int currentButtonState = digitalRead(buttonPin); // Lê o estado atual do botão
    unsigned long currentTime = millis();            // Captura o tempo atual

    // Verifica se o estado do botão mudou e se passou o tempo suficiente para o debounce
    if (currentButtonState != lastButtonState)
    {
        if ((currentTime - lastButtonStateChangeTime) > debounceDelay)
        {
            if (currentButtonState == LOW)
                buttonPressTime = currentTime; // Registra o momento em que o botão foi pressionado

            lastButtonStateChangeTime = currentTime;
        }
    }

    // Verifica se o botão está pressionado e se o tempo de pressionamento excede longPressDuration
    if (currentButtonState == LOW && (currentTime - buttonPressTime) >= longPressDuration)
    {
        // aguarda o botão ser liberado (de LOW para HIGH)
        digitalWrite(ledPin, !digitalRead(ledPin)); // apaga ou acende o led para indicar que o longpress foi reconhecido
        while (digitalRead(buttonPin) == LOW)
        {
            delay(100);
        }

        startServerMode(); // Chama a função startOTA para iniciar o processo
    }

    // Atualiza o estado anterior do botão
    lastButtonState = currentButtonState;
}

void SHtools_ESP32_OTA_AP::startServerMode()
{
    Serial.println("Entrando em modo Servidor...");

    WifiSetup();               // configura e inicializa o servidor wifi
    rotasEcallbacks();         // define rotas e callbacks
    ElegantOTA.begin(&server); // Start ElegantOTA
    WebSerial.begin(&server);  // Start Webserial
    server.begin();            // Start webserver

    Serial.println("Servidor iniciado");
    ServerMode = true;
    ServerModeInicio = millis();
}

void SHtools_ESP32_OTA_AP::rotasEcallbacks()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", IndexHTML); });

    // ElegantOTA callbacks
    ElegantOTA.onStart([this]()
                       { this->onOTAStart(); });

    ElegantOTA.onProgress([this](size_t current, size_t final)
                          { this->onOTAProgress(current, final); });

    ElegantOTA.onEnd([this](bool success)
                     { this->onOTAEnd(success); });

    // Callback para incoming messages do webserial
    WebSerial.onMessage([](uint8_t *data, size_t len)
                        {
    Serial.printf("Received %u bytes from WebSerial: ", len);
    Serial.write(data, len);
    Serial.println();
    WebSerial.println("Received Data...");
    String d = "";
    for(size_t i = 0; i < len; i++){
      d += char(data[i]);
    }
    WebSerial.println(d); });
}

void SHtools_ESP32_OTA_AP::WifiSetup()
{
    // desconecta WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();
        delay(2000);
    }

    // Configura o servidor AP
    IPAddress local_ip(192, 168, 100, 100);
    IPAddress local_mask(255, 255, 255, 0);
    IPAddress gateway(192, 168, 100, 1);
    String ssid = "192.168.100.100 -> " + generateSSID(); // Gera o SSID com identificador único

    // Aguarda a inicialização do servidor wifi
    Serial.println("inicializando webserver.");
    while (!WiFi.softAP(ssid.c_str()))
    {
        Serial.print(".");
        delay(100);
    }

    // aplica as configurações
    WiFi.softAPConfig(local_ip, gateway, local_mask);

    Serial.println("Access Point criado com sucesso.");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.softAPIP());
}

void SHtools_ESP32_OTA_AP::onOTAStart()
{
    // Log when OTA has started
    Serial.println("Update OTA iniciado!");
    // <Add your own code here>
}

void SHtools_ESP32_OTA_AP::onOTAProgress(size_t current, size_t final)
{
    // Log every 1 second
    unsigned long cTime = millis();
    if ((cTime - ota_progress_millis) > 1000)
    {
        ota_progress_millis = millis();
        Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    }
}

void SHtools_ESP32_OTA_AP::onOTAEnd(bool success)
{
    // Log when OTA has finished
    if (success)
    {
        Serial.println("Update OTA concluído com sucesso!");
    }
    else
    {
        Serial.println("Houve um erro durante o update OTA!");
    }
    // <Add your own code here>
}

String SHtools_ESP32_OTA_AP::generateSSID()
{
    uint64_t mac = ESP.getEfuseMac();   // Obtém o identificador único do ESP32
    String uniqueID = String(mac, HEX); // Converte para uma string hexadecimal
    uniqueID.toUpperCase();             // Opcional: converte para maiúsculas
    return uniqueID;
}

// Implementação do método getter para otaMode
bool SHtools_ESP32_OTA_AP::get_ServerMode() const
{
    return ServerMode;
}