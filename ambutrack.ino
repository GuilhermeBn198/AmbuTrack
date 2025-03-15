#include <WiFi.h>       // Biblioteca para conexão Wi-Fi 
#include <HTTPClient.h> // Biblioteca para requisições HTTP 
#include <TinyGPS++.h>  // Biblioteca para comunicação com o GPS 
#include <math.h>       // Biblioteca para funções matemáticas

// Configurações de rede e servidor
const char* ssid = "Awawa";       
const char* password = "12345677";  
const char* server = "https://paint-quaint-earthquake.glitch.me/";

// Configurações do GPS
#define RXD2 16               // Pino RX do ESP32 conectado ao TX do GPS 
#define TXD2 17               // Pino TX do ESP32 conectado ao RX do GPS 
#define GPS_BAUD 9600         // Taxa de comunicação do GPS 
TinyGPSPlus gps;              // Instância do objeto GPS 
HardwareSerial gpsSerial(2);  // Configuração da porta serial 2 para comunicação com o GPS

// Definição da área de interesse (localização alvo)
const float targetLat = 2.836118;  // Latitude alvo (substitua pelo valor desejado)
const float targetLon = -60.689867;  // Longitude alvo (substitua pelo valor desejado)
const float targetRadius = 150.0;     // Raio da área em metros

// 2.836118
// -60.689867

// Configuração dos pinos do LED e do buzzer para o alarme
#define LED_PIN  19      // Exemplo de pino para o LED
#define BUZZER_PIN 23    // Exemplo de pino para o buzzer

// Função para calcular a distância entre duas coordenadas (Fórmula de Haversine)
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000; // Raio da Terra em metros
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);
  double a = sin(dLat / 2) * sin(dLat / 2) +
             cos(radians(lat1)) * cos(radians(lat2)) *
             sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

// Função que aciona o alarme: pisca o LED e ativa o buzzer
void triggerAlarm() {
  // Exemplo: piscar e buzinar 3 vezes
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);             // Inicia a comunicação serial para monitoramento
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2); // Inicializa a comunicação com o GPS 
  Serial.println("Serial 2 do GPS iniciada");

  // Configura os pinos do LED e do buzzer como saída
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  } 
  Serial.println("\nWiFi Conectado!");
}

void loop() {
  unsigned long start = millis(); // Marca o tempo inicial do loop
  bool newData = false;           // Variável para verificar se há novos dados do GPS
  
  // Lê os dados do GPS durante 1 segundo
  while (millis() - start < 1000) { 
    while (gpsSerial.available() > 0) { 
      if (gps.encode(gpsSerial.read())) { 
        newData = true;
      }
    } 
  }

  if (newData && gps.location.isValid()) {
    float lat = gps.location.lat(); // Obtém a latitude
    float lon = gps.location.lng(); // Obtém a longitude

    // Calcula a distância entre a localização atual e a área definida
    double distance = calculateDistance(targetLat, targetLon, lat, lon);
    Serial.print("Distância: ");
    Serial.print(distance);
    Serial.println(" metros");

    // Se estiver dentro do raio definido, dispara o alarme
    if (distance <= targetRadius) {
      Serial.println("Dentro da área definida! Alarme acionado!");
      triggerAlarm();
    }
    
    // Envio da localização para o servidor, caso o Wi-Fi esteja conectado
    if (WiFi.status() == WL_CONNECTED) { 
      HTTPClient http; 
      String url = String(server) + "/update?lat=" + String(lat, 6) + "&lon=" + String(lon, 6); 
      http.begin(url); 
      int httpCode = http.GET(); 
      http.end();
      if (httpCode > 0) { 
        Serial.println("Localização enviada com sucesso!");
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
      } else { 
        Serial.println("Falha ao enviar a localização!"); 
      } 
    } else { 
      Serial.println("Wi-Fi desconectado, tentando reconectar..."); 
      WiFi.begin(ssid, password); 
    }
  } else { 
    Serial.println("Aguardando coordenadas válidas do GPS..."); 
  }
  delay(2000);
}
