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
    footer { margin-top: 30px; color: #555; font-size: 14px; }
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
  <a href='/html/webserial' class='button'>Monitor Serial</a><br>
  <button class='button' onclick="alert('TODO: Info page')">Informações</button>

  <footer>
    <p><strong>Comandos Disponíveis:</strong></p>
    <p><code>cmd:DebugInicial</code> - Define se o sistema iniciará em modo debug ou se dependerá de pressionar o botão para entrar no modo debug. O modo debug permanece ativo por até 30 minutos.</p>
  </footer>
</body>
</html>
)rawliteral";

SHtools_ESP32_OTA_AP::SHtools_ESP32_OTA_AP(int ledPin, int buttonPin, String nomeSketch)
    : ServerMode(false), ServerModeInicio(0), buttonPressTime(0), lastButtonStateChangeTime(0), longPressDuration(3000),
      debounceDelay(50), lastButtonState(HIGH), ledPin(ledPin), buttonPin(buttonPin),
      nomeSketch(nomeSketch), DebugInicial(0), ota_progress_millis(0), server(80), ws("/sw") {}

void SHtools_ESP32_OTA_AP::begin()
{
    // Configura os pinos de I/O
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);

    Serial.begin(115200);
    delay(1000);

    // Iniciar Preferences com o namespace "_config"
    config.begin("_config", false);

    // Verificar se a chave "DebugInicial" existe
    if (!config.isKey("DebugInicial"))
    {
        // Se não existe, cria e define o valor como false (0)
        config.putBool("DebugInicial", false);
        DebugInicial = false;
    }
    else
    {
        // Se existe, obtém o valor e atribui à variável
        DebugInicial = config.getBool("DebugInicial");
    }

    config.end(); // Fecha preferences para liberar recursos
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
    }
    else
    {
        if (DebugInicial)
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
    ElegantOTA.loop();   // processa as requisições Webserial e OTA
    ws.cleanupClients(); // Processa eventos do WebSocket e mantém o WebSocket ativo e limpa conexões inativas

    unsigned long cTime = millis();

    /*
    Se está em modo servidor há mais de 30 minutos,
    desativa o modo DebugInicial e reinicia o esp para sair do modo servidor
    */
    if ((cTime - ServerModeInicio) >= 1800000) // 30 minutos
    {
        set_DebugInicial(false); // Desativa DebugInicial
        ESP.restart();           // Reinicia o ESP32
    }

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
    printMSG("Entrando em modo Servidor...", true);

    WifiSetup();               // configura e inicializa o servidor wifi
    ElegantOTA.begin(&server); // Start ElegantOTA
    server.addHandler(&ws);    // Inicializa o WebSocket
    rotasEcallbacks();         // define rotas e callbacks
    server.begin();            // Start webserver

    printMSG("Servidor iniciado", true);
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

    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
               {
        switch (type) {
            case WS_EVT_CONNECT:
                printMSG("Novo cliente conectado", true);
                break;
            case WS_EVT_DISCONNECT:
                printMSG("Cliente desconectado", true);
                break;
            case WS_EVT_DATA:
                // Aqui você pode lidar com dados recebidos do cliente, se necessário
                break;
            default:
                break;
        } });
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
    printMSG("inicializando webserver.", true);
    while (!WiFi.softAP(ssid.c_str()))
    {
        printMSG(".");
        delay(100);
    }

    // aplica as configurações
    WiFi.softAPConfig(local_ip, gateway, local_mask);

    printMSG("Access Point criado com sucesso.", true);
    printMSG("SSID: ");
    printMSG(ssid, true);
    printMSG("IP do ESP32: ");
    printMSG(WiFi.softAPIP().toString(), true);
}

void SHtools_ESP32_OTA_AP::onOTAStart()
{
    // Log when OTA has started
    printMSG("Update OTA iniciado!", true);
    // <Add your own code here>
}

void SHtools_ESP32_OTA_AP::onOTAProgress(size_t current, size_t final)
{
    // Log every 1 second
    unsigned long cTime = millis();
    if ((cTime - ota_progress_millis) > 1000)
    {
        ota_progress_millis = millis();
        // Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
        char buffer[128]; // Define o buffer com tamanho suficiente para a mensagem
        snprintf(buffer, sizeof(buffer), "OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
        printMSG(String(buffer), true); // Chama diretamente o printMSG com a mensagem formatada
    }
}

void SHtools_ESP32_OTA_AP::onOTAEnd(bool success)
{
    // Log when OTA has finished
    if (success)
    {
        printMSG("Update OTA concluído com sucesso!", true);
    }
    else
    {
        printMSG("Houve um erro durante o update OTA!", true);
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

// Implementação do método setter para DebugInicial
void SHtools_ESP32_OTA_AP::set_DebugInicial(bool valor)
{
    // Grava o valor de DebugInicial e o armazena no Preferences
    config.begin("_config", false);        // Abre o Preferences para escrita
    config.putBool("DebugInicial", valor); // Grava o valor na chave
    DebugInicial = valor;                  // Atualiza a variável interna
    config.end();                          // Fecha o Preferences após a gravação
}

// Implementação de função para imprimir as informações em tela
void SHtools_ESP32_OTA_AP::printMSG(const String &msg, bool newline)
{
    if (SerialCMD(msg))
    {
        return;
    }

    if (ServerMode)
    {
        ws.textAll(msg); // Enviar para o WebSocket (serial remoto)
    }

    if (newline)
    {
        Serial.println(msg); // Envia com nova linha
    }
    else
    {
        Serial.print(msg); // Envia sem nova linha
    }
}

bool SHtools_ESP32_OTA_AP::SerialCMD(String _msg)
{
    // Verifica se a mensagem recebida é um comando (case insensitive)
    if (_msg.substring(0, 4).equalsIgnoreCase("cmd:"))
    {
        // Extrair o comando após "cmd:"
        String cmd = _msg.substring(4); // Remove "cmd:" e obtém o comando

        // Verificar qual comando foi enviado (case insensitive)
        if (cmd.equalsIgnoreCase("DebugInicial"))
        {
            // Alterna o valor de DebugInicial
            DebugInicial = !DebugInicial;
            set_DebugInicial(DebugInicial); // Grava a mudança via set_DebugInicial()

            // Reinicia o ESP32 após alterar o valor
            delay(1000);
            ESP.restart();
            while (true)
            {
            };
        }

        return true;
    }
    else
    {
        return false;
    }
}