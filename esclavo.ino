#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN_NEO_PIXEL 9
#define NUM_PIXELS 12
#define PIN_BOTON 10

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

typedef struct struct_message {
  int red;
  int green;
  int blue;
  bool returnCommand;
  int slaveID; // ID único del esclavo
} struct_message;

struct_message datosRecibidos;
struct_message datosEnvio;

// Direccion MAC del maestro (ajustar si es necesario)
uint8_t masterAddress[] = {0xC8, 0xF0, 0x9E, 0x9E, 0x68, 0x88};

//CAMBIA EL IDENTIFICADOR PARA CADA UNO
const int slaveID = 3; 

int redOriginal = 50;
int greenOriginal = 200;
int blueOriginal = 0;

bool colorModificado = false;

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&datosRecibidos, incomingData, sizeof(datosRecibidos));

  if (datosRecibidos.returnCommand) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      NeoPixel.setPixelColor(i, NeoPixel.Color(redOriginal, greenOriginal, blueOriginal));
    }
    colorModificado = false;
    Serial.println("Restaurando color original.");
  } else {
    for (int i = 0; i < NUM_PIXELS; i++) {
      NeoPixel.setPixelColor(i, NeoPixel.Color(datosRecibidos.red, datosRecibidos.green, datosRecibidos.blue));
    }
    colorModificado = true;
    Serial.println("Color cambiado por el maestro.");
  }
  NeoPixel.show();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(PIN_BOTON, INPUT_PULLUP);

  NeoPixel.begin();
  NeoPixel.clear();
  for (int i = 0; i < NUM_PIXELS; i++) {
    NeoPixel.setPixelColor(i, NeoPixel.Color(redOriginal, greenOriginal, blueOriginal));
  }
  NeoPixel.show();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Fallo al agregar el maestro");
  }
}

void loop() {
  if (digitalRead(PIN_BOTON) == HIGH) { 
    Serial.println("¡Botón presionado!");

    datosEnvio.slaveID = slaveID;
    datosEnvio.red = redOriginal;
    datosEnvio.green = greenOriginal;
    datosEnvio.blue = blueOriginal;
    datosEnvio.returnCommand = false;

    esp_now_send(masterAddress, (uint8_t *)&datosEnvio, sizeof(datosEnvio));
    delay(500); // Evita rebotes
  }
}
