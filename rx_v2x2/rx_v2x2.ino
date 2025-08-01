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

// Tablas freq_hopping (valores de canales RF)
const uint8_t freq_hopping[4][16] = {
  { 0x27, 0x1B, 0x39, 0x28, 0x24, 0x22, 0x2E, 0x36, 0x19, 0x21, 0x29, 0x14, 0x1E, 0x12, 0x2D, 0x18 },
  { 0x2E, 0x33, 0x25, 0x38, 0x19, 0x12, 0x18, 0x16, 0x2A, 0x1C, 0x1F, 0x37, 0x2F, 0x23, 0x34, 0x10 },
  { 0x11, 0x1A, 0x35, 0x24, 0x28, 0x18, 0x25, 0x2A, 0x32, 0x2C, 0x14, 0x27, 0x36, 0x34, 0x1C, 0x17 },
  { 0x22, 0x27, 0x17, 0x39, 0x34, 0x28, 0x2B, 0x1D, 0x18, 0x2A, 0x21, 0x38, 0x10, 0x26, 0x20, 0x1F }
};

uint8_t txid[3] = { 177, 9, 156 }; // ID usado para seleccionar tabla hopping


// Variables del receptor
// Estado del protocolo V202 - Dirección RX (típica V202)
uint8_t rx_address[5] = {0x66, 0x88, 0x68, 0x68, 0x68};
uint8_t hopping_sequence[16];
uint8_t packet[16];
uint8_t bound_address[5] = {0};
uint8_t rf_channels[16]; // tabla de canales 
uint8_t rf_ch_num = 0;
bool bound = false;
unsigned long lastPacketTime = 0;

// Rangos típicos del protocolo V202
//#define MIN_CHANNEL 2
//#define MAX_CHANNEL 80

void calcularHopping() {
  uint8_t sumId = txid[0] + txid[1] + txid[2];
  const uint8_t* sel_list = freq_hopping[sumId & 0x03];
  uint8_t increment = (sumId & 0x1E) >> 2;

  for (int i = 0; i < 16; i++) {
    uint8_t val = sel_list[i] + increment;
    rf_channels[i] = (val & 0x0F) ? val : val - 3;
  }
}

bool validarChecksum(uint8_t* data) {
  uint8_t sum = 0;
  for (int i = 0; i < 15; i++) sum += data[i];
  return (sum == data[15]);
}

void configurarRadio() {
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
  //radio.setChannel(10);
  //radio.setRetries(0, 0);
  //radio.disableDynamicPayloads();    
  radio.setPayloadSize(16);
  radio.openReadingPipe(0, rx_address);
  radio.startListening();

  Serial.println("Buscando señal del control remoto...");
  Serial.println("BK2425 iniciado correctamente.");
  //radio.printDetails();
}

// Canales segun protocolo V2x2 
void imprimirDatos(uint8_t* data) {
  uint8_t throttle = data[0];
  uint8_t yaw      = data[1];
  uint8_t pitch    = data[2];
  uint8_t roll     = data[4];
  uint8_t flags    = data[5];

  Serial.print(F("Throttle: ")); Serial.print(throttle);
  Serial.print(F(" | Yaw: ")); Serial.print(yaw);
  Serial.print(F(" | Pitch: ")); Serial.print(pitch);
  Serial.print(F(" | Roll: ")); Serial.print(roll);
  Serial.print(F(" | Flags: 0b")); Serial.println(flags, BIN);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Receptor V202 - ESP32-C3"));

  calcularHopping();
  configurarRadio();

  radio.setChannel(rf_channels[rf_ch_num]);
  lastPacketTime = millis();
}

void bind_loop() {
  for (uint8_t ch = MIN_CHANNEL; ch <= MAX_CHANNEL; ch++) {
    radio.setChannel(ch);
    radio.startListening();
    Serial.printf("Canal %d...\n", ch);

    for (int i = 0; i < BIND_RETRIES; i++) {
      if (radio.available()) {
        radio.read(&packet, sizeof(packet));

        Serial.print("Paquete recibido: ");
        for (int j = 0; j < 16; j++) {
          Serial.printf("%02X ", packet[j]);
        }
        Serial.println();

        // Verificar si el paquete parece válido 
        if ((packet[0] ^ packet[1]) == 0xFF && packet[4] == 0xA7) {
          memcpy(bound_address, packet + 2, 5);
          current_channel = ch;
          bound = true;
          Serial.printf("✓ ¡Bind exitoso! Canal %d | Dirección: %02X %02X %02X %02X %02X\n",
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

void start_reception() {
  radio.stopListening();
  radio.openReadingPipe(0, bound_address);
  radio.setChannel(current_channel);
  radio.startListening();
  Serial.println("Receptor conectado, escuchando paquetes...");
}

void loop() {  
  if (radio.available()) {
    radio.read(packet, sizeof(packet));
    lastPacketTime = millis();

    if (validarChecksum(packet)) {
      if (!bound) {
        bound = true;
        Serial.println(F("✓ Bind detectado. Escuchando control..."));
      }
      imprimirDatos(packet);

      // Hopping a siguiente canal
      rf_ch_num = (rf_ch_num + 1) % 16;
      radio.setChannel(rf_channels[rf_ch_num]);
    } else {
      Serial.println(F("Paquete con checksum inválido"));
    }
  } else {
    // Timeout: si pasaron > 2 segundos sin paquete, se pierde bind
    if (bound && millis() - lastPacketTime > 2000) {
      Serial.println(F("Perdida de señal, buscando bind..."));
      bound = false;
      rf_ch_num = 0;
      radio.setChannel(rf_channels[rf_ch_num]);
    }
  }
}