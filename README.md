# 🧠 Sprint 4 – Edge Computing  
### Projeto: Contador Inteligente de Pessoas com ESP32 e MQTT  

---

## 👥 Integrantes do Grupo
| Nome | RM |
|------|------|
| Jean Carlos Rodrigues         | RM566439 |
| Giovanni Tarzoni Piccin       | RM564014 |
| Enrico Gianni Nóbrega Puttini | RM561400 |
| Henrique Infanti Coratolo     | RM561865 |
| Bruno Lobosque                | RM561254 |

---

## 💡 Descrição do Projeto

O **Sprint 4 - Edge Computing** tem como objetivo o desenvolvimento de um **sistema IoT de contagem de pessoas** utilizando o **ESP32**, **sensor ultrassônico** e **integração via MQTT** com a **plataforma FIWARE (Orion Context Broker)**.

O projeto detecta a **passagem de pessoas** em um ambiente através da variação de distância captada pelo sensor.  
Cada detecção **incrementa o contador de fluxo** (`flow`) e envia os dados em tempo real para a **nuvem**, possibilitando o **monitoramento remoto** e integração com aplicações web.

---

## ⚙️ Componentes Utilizados

| Componente | Descrição |
|-------------|------------|
| **ESP32** | Microcontrolador principal com Wi-Fi integrado |
| **Sensor Ultrassônico HC-SR04** | Mede a distância instantânea e detecta passagem de pessoas |
| **Broker MQTT** | Responsável pela comunicação entre o ESP32 e o FIWARE |
| **Plataforma FIWARE** | Recebe e armazena os dados (Orion Context Broker) |
| **Wokwi** | Simulador utilizado para testes e validação do código |

---

## 🔌 Ligações do Circuito

| Componente | Pino ESP32 |
|-------------|-------------|
| **Trigger (TRIG)** | GPIO 13 |
| **Echo (ECHO)** | GPIO 12 |
| **VCC** | 5V |
| **GND** | GND |

---

## 💻 Código do ESP32

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

// Configurações Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = "sprint4/edge/flow";

// Pinos do sensor ultrassônico
#define TRIG 13
#define ECHO 12

// Variáveis de controle
long duration;
int distance;
int lastDistance = 0;
int flowCount = 0;

// Clientes Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Função de conexão Wi-Fi
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Setup inicial
void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

// Publica dados MQTT
void publishData() {
  char jsonData[100];
  sprintf(jsonData, "{\"flow\": %d, \"distancia\": %d}", flowCount, distance);
  client.publish(topic, jsonData);
  Serial.println(jsonData);
}

// Loop principal
void loop() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  duration = pulseIn(ECHO, HIGH);
  distance = duration * 0.034 / 2;

  if (distance < 50 && lastDistance >= 50) {
    flowCount++;
    publishData();
  }

  lastDistance = distance;

  if (!client.connected()) {
    client.connect("ESP32Client");
  }

  client.loop();
  delay(500);
}
```

---

## 🌐 Integração IoT com FIWARE (Postman)

A integração foi realizada através de requisições **HTTP/MQTT** utilizando o **Postman**.  
Cada leitura do sensor é enviada para o **Orion Context Broker** como uma entidade `Lamp`, com o atributo `flow` representando o número de passagens detectadas.

### Exemplo de Resposta no Postman:
```json
{
    "contextResponses": [
        {
            "contextElement": {
                "attributes": [
                    {
                        "name": "flow",
                        "values": [5]
                    }
                ],
                "id": "urn:ngsi-ld:Lamp:001",
                "type": "Lamp"
            },
            "statusCode": {
                "code": "200",
                "reasonPhrase": "OK"
            }
        }
    ]
}
```

### 📸 Print da Integração:
![Print da integração no Postman](https://imgur.com/a/R24Y42j)

---

## 📊 Resultados da PoC

- O sistema foi capaz de **detectar e contar passagens humanas** com alta precisão.  
- A comunicação via **MQTT + FIWARE** demonstrou **baixa latência** e **estabilidade**.  
- O **monitoramento em tempo real** foi validado através da simulação no **Wokwi** e do **envio de dados para o Postman**.

---

## 🧩 Organização do Repositório

```
📂 sprint4-edge-computing/
├── 📁 src/
│   └── contador_pessoas.ino
├── 📁 docs/
│   └── prints_integracao/
│       └── postman.png
└── 📄 README.md
```

---

## 🔁 Reprodutibilidade do Projeto

Para replicar o projeto:
1. Clone o repositório:  
   ```bash
   git clone https://github.com/Sprint4Edge/Sprint4-Edge.git
   ```
2. Abra o código no **Wokwi** ou **Arduino IDE**.
3. Configure o **Wi-Fi** e o **broker MQTT**.
4. Execute o projeto e visualize os dados no **Postman** ou **Painel FIWARE**.

---

## 🧪 Simulação Online

🔗 [Clique para testar no Wokwi](https://wokwi.com/projects/446815872629354497)

---

## 🎥 Demonstração em Vídeo
🔗 [Link para o vídeo da PoC (YouTube)](https://youtu.be/RXW5s48QxTU)

---

## 🧱 Tecnologias Utilizadas
- **C++ / Arduino IDE**
- **ESP32**
- **MQTT (HiveMQ)**
- **FIWARE Orion Context Broker**
- **Postman**
- **Wokwi IoT Simulator**

---

## 🏁 Conclusão

O projeto **Sprint 4 - Edge Computing** demonstrou de forma prática o uso da computação de borda aplicada a sistemas IoT.  
A contagem de pessoas com envio de dados em tempo real para a nuvem comprova a **eficiência da arquitetura distribuída** e sua **aplicabilidade em monitoramento inteligente de ambientes.**
