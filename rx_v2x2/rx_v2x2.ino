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
  radio.setDataRate(RF24_250KBPS);  // mejor estabilidad
  radio.setChannel(10);
  radio.openReadingPipe(1, 0xE7E7E7E7E7LL); // Dirección ficticia
  radio.startListening();

  Serial.println("BK2425 iniciado correctamente.");
  radio.printDetails();
}

void loop() {

}