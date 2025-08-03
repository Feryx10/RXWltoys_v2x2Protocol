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

// ID usado para seleccionar tabla hopping
// All - 177, 9, 156 
// Tx WlToys A242 - 223, 150, 69
// Tx iRangeX IRX6 - 158, 192, 5
uint8_t txid[3] = { 223, 150, 69 }; 

uint8_t rf_channels[16]; // tabla de canales calculada
uint8_t rf_ch_num = 0;

uint8_t packet[16];

bool bound = false;
unsigned long lastPacketTime = 0;

// Dirección RX (típica V202)
// [102, 136, 104, 104, 104]
uint8_t rx_address[5] = { 0x66, 0x88, 0x68, 0x68, 0x68 };

// Hopping forzado por tiempo
unsigned long lastHopTime = 0;
const unsigned long hopInterval = 300; // ms

void calcularHopping() {
  uint8_t sumId = txid[0] + txid[1] + txid[2];
  const uint8_t* sel_list = freq_hopping[sumId & 0x03];
  uint8_t increment = (sumId & 0x1E) >> 2;

  for (int i = 0; i < 16; i++) {
    uint8_t val = sel_list[i] + increment;
    rf_channels[i] = (val & 0x0F) ? val : val - 3;
  }
}

void configurarRadio() {
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

  if (!radio.begin()) {
    Serial.println(F(" !! No se detectó BK2425."));
    while (1);
  }

  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(16);
  radio.setCRCLength(RF24_CRC_16);

  radio.openReadingPipe(0, rx_address);
  radio.startListening();
}

bool validarChecksum(uint8_t* data) {
  uint8_t sum = 0;
  for (int i = 0; i < 15; i++) sum += data[i];
  return (sum == data[15]);
}

void imprimirDatos(uint8_t* data) {
  // Canales
  uint8_t throttle = data[2];
  uint8_t yaw      = data[1];
  //uint8_t pitch      = data[0];
  //uint8_t roll       = data[3];
  // Trims - medio = 0x40 - 64 - 01000000
  uint8_t yawTrim = data[4];
  uint8_t thrTrim = data[5];
  uint8_t rolTrim = data[6];

  uint8_t C    = data[7];
  uint8_t D    = data[8];
  uint8_t E    = data[9];
  uint8_t F   = data[10];
  uint8_t G    = data[11];
  uint8_t H    = data[12];
  uint8_t I    = data[13];
  uint8_t J    = data[14];
  uint8_t K    = data[15];


  Serial.print(F("Throttle: ")); Serial.print(throttle);
  Serial.print(F(" | Yaw: ")); Serial.print(yaw);
  //Serial.print(F(" | Pitch: ")); Serial.print(pitch);
  //Serial.print(F(" | Roll: ")); Serial.print(roll);
  Serial.print(F(" | T.thr: ")); Serial.print(thrTrim);
  Serial.print(F(" | T.yaw: ")); Serial.print(yawTrim);
  //Serial.print(F(" | T.rol: ")); Serial.print(rolTrim);
  Serial.print(F(" | Flags: 0b")); Serial.println(F, BIN); 
 

  Serial.print(C);
  Serial.print(" | ");
  Serial.print(D);
  Serial.print(" | ");
  Serial.print(E);
  Serial.print(" TxID|  ");
  Serial.print(F);
  Serial.print("|");
  Serial.print(G);
  Serial.print("|");
  Serial.print(H);
  Serial.print("|");
  Serial.print(I);
  Serial.print("|");
  Serial.print(J);
  Serial.print("|");
  Serial.print(K); 
  Serial.println("|");
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

void loop() {
  if (radio.available()) {
    radio.read(packet, sizeof(packet));
    lastPacketTime = millis();

    if (validarChecksum(packet)) {
      if (!bound) {
        bound = true;
        Serial.println(F("Bind detectado. Escuchando control..."));
      }
      imprimirDatos(packet);

      // Hopping a siguiente canal
      //rf_ch_num = (rf_ch_num + 1) % 16;
      //radio.setChannel(rf_channels[rf_ch_num]);
      //Serial.print(F("Hopping al canal: "));
      //Serial.println(rf_channels[rf_ch_num], HEX);
      //delay(2);
    } else {
      Serial.println(F("Paquete con checksum inválido"));
    }
  } 
 // Forzar salto de canal por tiempo
  if (millis() - lastHopTime > hopInterval) {
    rf_ch_num = (rf_ch_num + 1) % 16;
    radio.setChannel(rf_channels[rf_ch_num]);
    Serial.print(F("Hopping al canal: "));
    Serial.println(rf_channels[rf_ch_num], HEX);
    lastHopTime = millis();
  }

  // Timeout: si pasan 4 segundos sin paquete, se pierde bind
  if (bound && millis() - lastPacketTime > 4000) {
    Serial.println(F("Perdida de señal, buscando bind..."));
    bound = false;
    rf_ch_num = 0;
    radio.setChannel(rf_channels[rf_ch_num]);
    lastHopTime = millis();
    Serial.print(F("Canal reiniciado: "));
    Serial.println(rf_channels[rf_ch_num], HEX);
  }
}
