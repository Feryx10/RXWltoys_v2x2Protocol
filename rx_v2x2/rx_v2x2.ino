// Feryx - Rx WlToys v202 Protocol
// Hardware:
// Esp32-C3 - Super Mini WiFi
// RF - Beken BK2425  (RFM75)
#include <SPI.h>
#include <RF24.h>

// Pines
#define CE_PIN   2
#define CSN_PIN  3
#define SCK_PIN  4
#define MISO_PIN 5
#define MOSI_PIN 6

RF24 radio(CE_PIN, CSN_PIN);  // CE, CSN

// Variables del receptor
uint8_t packet[16];
uint8_t bound_address[5] = {0};
uint8_t current_channel = 0;
bool bound = false;

// Rangos típicos del protocolo V202
#define MIN_CHANNEL 2
#define MAX_CHANNEL 80

// Intentos de bind por canal antes de avanzar
#define BIND_RETRIES 100

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando BK2425 con ESP32...");

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

  if (!radio.begin()) {
    Serial.println("Fallo de comunicación.");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);  // Segun Protocolo 
  radio.setCRCLength(RF24_CRC_16);
  radio.setAutoAck(false);
  radio.setChannel(10);
  radio.setRetries(0, 0);
  radio.disableDynamicPayloads();  
  radio.startListening();
  radio.setPayloadSize(16);

  Serial.println("Buscando señal del control remoto...");
  Serial.println("BK2425 iniciado correctamente.");
  //radio.printDetails();
}

void bind_loop() {
  for (uint8_t ch = MIN_CHANNEL; ch <= MAX_CHANNEL; ch++) {
    radio.setChannel(ch);
    radio.startListening();

    for (int i = 0; i < BIND_RETRIES; i++) {
      if (radio.available()) {
        radio.read(&packet, sizeof(packet));

        // Verificar si el paquete parece válido (simple heurística)
        if ((packet[0] ^ packet[1]) == 0xFF && packet[4] == 0xA7) {
          memcpy(bound_address, packet + 2, 5);
          current_channel = ch;
          bound = true;
          Serial.printf("✅ ¡Bind exitoso! Canal %d | Dirección: %02X %02X %02X %02X %02X\n",
            current_channel,
            bound_address[0], bound_address[1],
            bound_address[2], bound_address[3],
            bound_address[4]
          );
          return;
        }
      }
      delay(5);
    }
    radio.stopListening();
  }
}

void loop() {  
  if (radio.available()) {
    uint8_t payload[32];
    radio.read(&payload, sizeof(payload));
    Serial.println("Paquete recibido:");
    for (int i = 0; i < 32; i++) {
      Serial.printf("%02X ", payload[i]);
    }
    Serial.println();
  }
}