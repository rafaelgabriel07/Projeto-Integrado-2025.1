#include <WiFi.h>
#include <HTTPClient.h> // Para fazer requisições HTTP como cliente

// Configurações do seu hotspot do celular
const char* ssid = "Matheus";     // *** SUBSTITUA PELO NOME DO SEU HOTSPOT ***
const char* password = "12345678"; // *** SUBSTITUA PELA SENHA DO SEU HOTSPOT ***

// Endereço IP do seu PC na rede do hotspot do celular
// Você precisa descobrir esse IP (use ipconfig no Windows ou ifconfig/ip a no Linux/macOS)
const char* flaskServerIP = "192.168.254.105"; // *** SUBSTITUA PELO IP REAL DO SEU PC ***
const int flaskServerPort = 5000;          // A porta que o seu servidor Flask está rodando

// Variável para armazenar a informação recebida
String receivedPlantData = "Nenhum dado recebido";

void setup() {
  Serial.begin(115500); // Velocidade comum para monitor serial
  delay(2000);          // Atraso para garantir que o Serial.begin() inicialize

  Serial.println("\nIniciando ESP32 como cliente Wi-Fi...");

  // Configura o ESP32 para o modo Station (cliente Wi-Fi)
  WiFi.mode(WIFI_STA);

  // Conecta-se ao hotspot do seu celular
  Serial.printf("Tentando conectar ao hotspot: %s\n", ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // Tenta por 10 segundos
    delay(250);
    Serial.print(".");
    attempts++;
    if (attempts % 10 == 0) { // A cada 10 tentativas, imprime o status atual
        Serial.printf("\nTentativa %d, Status: %d\n", attempts, WiFi.status());
    }
  }
  Serial.println(); // Nova linha

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Conectado ao Wi-Fi! IP local do ESP32: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Servidor Flask esperado em: http://%s:%d/\n", flaskServerIP, flaskServerPort);
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi. Verifique SSID e senha do hotspot.");
    Serial.print("Status do WiFi: ");
    Serial.println(WiFi.status()); // Exibe o código de status da falha
    // Se não conectar, não há como buscar dados do Flask.
  }
}

void loop() {
  // Verifique se está conectado antes de tentar buscar dados
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Constrói a URL para a rota /dados do seu servidor Flask
    String serverPath = "http://" + String(flaskServerIP) + ":" + String(flaskServerPort) + "/dados";

    Serial.print("Buscando dados da planta em: ");
    Serial.println(serverPath);

    // Inicia a conexão HTTP
    http.begin(serverPath);

    // Faz a requisição GET
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) { // Se o código de resposta for positivo (ex: 200 OK)
      Serial.printf("Código de resposta HTTP: %d\n", httpResponseCode);
      if (httpResponseCode == HTTP_CODE_OK) { // HTTP_CODE_OK é 200
        receivedPlantData = http.getString(); // Lê o corpo da resposta
        Serial.print("Dados da planta recebidos: ");
        Serial.println(receivedPlantData);

        // --- Processamento dos dados recebidos ---
        // Exemplo: "Rosa;[40,60];[6,8]"
        int firstDelimiter = receivedPlantData.indexOf(';');
        int secondDelimiter = receivedPlantData.indexOf(';', firstDelimiter + 1);

        if (firstDelimiter != -1 && secondDelimiter != -1) {
          String nomePopular = receivedPlantData.substring(0, firstDelimiter);
          String umidadeSoloStr = receivedPlantData.substring(firstDelimiter + 1, secondDelimiter);
          String uvDiaStr = receivedPlantData.substring(secondDelimiter + 1);

          Serial.printf("  Nome Popular: %s\n", nomePopular.c_str());
          Serial.printf("  Umidade Solo: %s\n", umidadeSoloStr.c_str());
          Serial.printf("  UV Dia: %s\n", uvDiaStr.c_str());

          // Agora você pode parsear umidadeSoloStr e uvDiaStr para extrair os números
          // e usar esses valores para controlar seus sensores/atuadores.
          // Exemplo de como extrair valores de "[40,60]"
          umidadeSoloStr.replace("[", "");
          umidadeSoloStr.replace("]", "");
          int umidadeMin = umidadeSoloStr.substring(0, umidadeSoloStr.indexOf(',')).toInt();
          int umidadeMax = umidadeSoloStr.substring(umidadeSoloStr.indexOf(',') + 1).toInt();
          Serial.printf("  Umidade Min: %d, Max: %d\n", umidadeMin, umidadeMax);

          uvDiaStr.replace("[", "");
          uvDiaStr.replace("]", "");
          int uvMin = uvDiaStr.substring(0, uvDiaStr.indexOf(',')).toInt();
          int uvMax = uvDiaStr.substring(uvDiaStr.indexOf(',') + 1).toInt();
          Serial.printf("  UV Min: %d, Max: %d\n", uvMin, uvMax);

        } else {
          Serial.println("Formato de dados inesperado. Esperado 'nome;umidade;uv'.");
        }

      }
    } else {
      Serial.printf("Erro na requisição HTTP: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end(); // Fecha a conexão
  } else {
    Serial.println("ESP32 não conectado ao Wi-Fi. Não é possível buscar dados.");
    // Tenta reconectar se a conexão caiu
    WiFi.begin(ssid, password);
    // Pode adicionar um delay aqui para evitar loop rápido de reconexão
    delay(1000);
  }

  delay(5000); // Espera 5 segundos antes de buscar os dados novamente
}