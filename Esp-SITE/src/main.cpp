#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid     = "PREDIALNET_2G_3078";
const char* password = "71514802";

AsyncWebServer server(80);

// Função para responder 404
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain; charset=utf-8", "Recurso não encontrado");
}

void setup() {
  // Alterado para 115200 baud para maior compatibilidade com ESP32
  Serial.begin(115200);
  // Adicionado esta linha para esperar a porta serial conectar.
  // Essencial para garantir que as primeiras mensagens sejam vistas.
  while (!Serial); 
  
  // Adicionado um atraso maior para garantir que o monitor serial inicie
  delay(2000); // Atraso de 2 segundos para o Serial.begin()

  Serial.println("Iniciando...");

  // 1) Monta o sistema de ficheiros
  if (!LittleFS.begin(true)) {               // true = formata se falhar
    Serial.println("LittleFS não inicializado!");
    return;
  }
  Serial.println("LittleFS inicializado com sucesso.");

  // 2) Liga-se ao Wi-Fi
  Serial.printf("Ligando-se à rede %s", ssid);
  WiFi.begin(ssid, password); // Inicia a conexão Wi-Fi
  
  // Loop de espera pela conexão Wi-Fi
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 40) { // Tenta por cerca de 10 segundos (40 * 250ms)
    delay(250);
    Serial.print('.');
    tentativas++;
  }
  Serial.println(); // Nova linha após os pontos

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("IP obtido: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi. Verifique SSID e senha.");
    Serial.print("Status do Wi-Fi: ");
    Serial.println(WiFi.status()); // Imprime o código de status da falha
    // Se não conectar, o servidor não será iniciado.
    return; 
  }

  // 3) Rotas para ficheiros estáticos
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html; charset=utf-8");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/script.js", "application/javascript");
  });

  server.on("/plantas.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/plantas.json", "application/json; charset=utf-8");
  });

  // Rota para receber dados da planta selecionada do site (POST)
  server.on("/selectPlant", HTTP_POST,
            [](AsyncWebServerRequest *request){ /* Handler para requisição POST sem corpo */ },
            NULL, // Não estamos lidando com upload de arquivos aqui
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if (index + len == total) { // Se esta é a última parte dos dados
        String jsonString;
        for (size_t i = 0; i < len; i++) {
            jsonString += (char)data[i];
        }

        Serial.print("\nRecebida string JSON no ESP32: ");
        Serial.println(jsonString);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, jsonString);

        if (error) {
            Serial.print(F("deserializeJson() falhou: "));
            Serial.println(error.f_str());
            request->send(400, "text/plain", "Erro ao parsear JSON no ESP32.");
            return;
        }

        String nomePopular = doc["nome_popular"];
        String umidadeSolo = doc["umidade_solo"];
        String uvDia = doc["uv_dia"];

        Serial.print("  Planta Selecionada: ");
        Serial.println(nomePopular);
        Serial.print("  Umidade do Solo (range): ");
        Serial.println(umidadeSolo);
        Serial.print("  UV do Dia (range): ");
        Serial.println(uvDia);

        request->send(200, "text/plain", "Dados da planta recebidos com sucesso pelo ESP32!");
    }
  });

  // 4) 404 genérico
  server.onNotFound(notFound);

  // 5) Inicia o servidor
  server.begin();
  Serial.println("Servidor web iniciado.");
}

void loop() {
  // nada a fazer; AsyncWebServer cuida de tudo
}
