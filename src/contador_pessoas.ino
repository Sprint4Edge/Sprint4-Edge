// Autores do projeto com nome e RM
// Giovanni Tarzoni Piccin - RM: 564014
// Enrico Gianni Nóbrega Puttini - RM: 561400
// Henrique Infanti Coratolo - RM: 561865
// Jean Carlos Rodrigues da Silva - RM: 566439
// Bruno Lobosque - RM: 561254

#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>

// --------------------- CONFIGURAÇÕES EDITÁVEIS ---------------------
const char* SSID = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* BROKER_MQTT = "";   // IP do broker FIWARE
const int BROKER_PORT = 1883;

const char* TOPIC_PUBLISH_INSTANT = "/TEF/lamp001/attrs/i";   // Distância instantânea
const char* TOPIC_PUBLISH_COUNT   = "/TEF/lamp001/attrs/c";   // Contador total
const char* TOPIC_PUBLISH_FLOW    = "/TEF/lamp001/attrs/f";   // Fluxo por minuto
const char* ID_MQTT = "lamp001";

// Pinos
#define TRIG_PIN 13
#define ECHO_PIN 12
#define LED_OK 33
#define LED_ALERT 26
#define BUZZER_PIN 25

// Parâmetros de detecção
#define DISTANCE_THRESHOLD_CM 80   // distância em cm para considerar pessoa
#define DEBOUNCE_MS 1500           // intervalo mínimo entre detecções
#define PUBLISH_INTERVAL_MS 10000  // intervalo de publicação (10s)

// ------------------ VARIÁVEIS GLOBAIS ------------------
WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastDetectionMillis = 0;
bool lastDetectionState = false;
unsigned long lastPublishMillis = 0;
unsigned long totalCount = 0;
std::vector<unsigned long> detectionTimestamps; // timestamps das detecções

// ------------------ FUNÇÕES ------------------
void initSerial() {
  Serial.begin(115200);
  delay(50);
}

void initWiFi() {
  Serial.println();
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  mqtt.setServer(BROKER_MQTT, BROKER_PORT);
}

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  long distance = duration / 58;
  return distance;
}

void ensureMqttConnected() {
  if (mqtt.connected()) return;
  Serial.print("Conectando ao broker MQTT...");
  while (!mqtt.connected()) {
    if (mqtt.connect(ID_MQTT)) {
      Serial.println("conectado!");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

void purgeOldTimestamps() {
  unsigned long now = millis();
  unsigned long threshold = now - 60000UL; // janela de 1 minuto
  while (!detectionTimestamps.empty() && detectionTimestamps.front() < threshold) {
    detectionTimestamps.erase(detectionTimestamps.begin());
  }
}

float calcFlowPerMinute() {
  purgeOldTimestamps();
  return (float)detectionTimestamps.size();
}

void publishInstantEvent(long distance) {
  ensureMqttConnected();
  char payload[16];
  snprintf(payload, sizeof(payload), "%ld", distance);
  mqtt.publish(TOPIC_PUBLISH_INSTANT, payload);
  Serial.printf("Publicado i (instant): %s\n", payload);
}

void publishCountSummary() {
  ensureMqttConnected();
  float flow = calcFlowPerMinute();
  char payloadC[16];
  char payloadF[16];

  snprintf(payloadC, sizeof(payloadC), "%lu", totalCount);
  snprintf(payloadF, sizeof(payloadF), "%.0f", flow);

  mqtt.publish(TOPIC_PUBLISH_COUNT, payloadC);
  mqtt.publish(TOPIC_PUBLISH_FLOW, payloadF);

  Serial.printf("Publicado c (count): %s | f (flow): %s\n", payloadC, payloadF);
}

// ------------------ SETUP ------------------
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_ALERT, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  initSerial();
  initWiFi();
  initMQTT();

  digitalWrite(LED_OK, HIGH);
  delay(300);
  digitalWrite(LED_OK, LOW);
  lastPublishMillis = millis();
}

// ------------------ LOOP ------------------
void loop() {
  ensureMqttConnected();
  mqtt.loop();

  long distance = readDistanceCM();
  if (distance > 0)
    Serial.printf("Distância: %ld cm\n", distance);

  bool detectedNow = (distance > 0 && distance <= DISTANCE_THRESHOLD_CM);
  unsigned long now = millis();

  if (detectedNow && !lastDetectionState && (now - lastDetectionMillis > DEBOUNCE_MS)) {
    lastDetectionMillis = now;
    totalCount++;
    detectionTimestamps.push_back(now);

    Serial.printf(">>> Pessoa detectada! Total: %lu\n", totalCount);

    digitalWrite(LED_ALERT, HIGH);
    digitalWrite(LED_OK, LOW);
    tone(BUZZER_PIN, 1000, 150);

    publishInstantEvent(distance);
  }

  if (!detectedNow) {
    digitalWrite(LED_ALERT, LOW);
    digitalWrite(LED_OK, HIGH);
  }

  lastDetectionState = detectedNow;

  if (now - lastPublishMillis >= PUBLISH_INTERVAL_MS) {
    publishCountSummary();
    lastPublishMillis = now;
  }

  delay(100);
}
