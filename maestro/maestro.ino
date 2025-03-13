#include <vector>
#include <functional>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <esp_now.h>
#include <WiFi.h>

bool start = true;
////VARIABLES DE JUEGO_ SIMON DICE//////////////
std::vector<int> userButtonSequence;
//////////////////////////////////////////////////////////////////////////
// Definición de la estructura de mensaje que compartiremos con los esclavos
//////////////////////////////////////////////////////////////////////////
typedef struct {
  int red;
  int green;
  int blue;
  bool returnCommand; 
  int slaveID; 
} ColorMessage;

//////////////////////////////////////////////////////////////////////////
// Clase para representar un esclavo
//////////////////////////////////////////////////////////////////////////
class SlaveDevice {
  private:
    uint8_t macAddress[6];
    int     slaveID;
    
  public:
    // Constructor: recibe un array con la MAC y el ID
    SlaveDevice(const uint8_t *mac, int id) {
      memcpy(macAddress, mac, 6);
      slaveID = id;
    }

    // Inicializa el peer en ESP-NOW (agrega el dispositivo a la lista de peers)
    bool initPeer() {
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, macAddress, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        return true;
      } else {
        return false;
      }
    }

    // Envía un color (o comando) al esclavo
    void sendColor(int r, int g, int b, bool returnCmd = false) {
      ColorMessage msg;
      msg.red           = r;
      msg.green         = g;
      msg.blue          = b;
      msg.returnCommand = returnCmd;
      msg.slaveID       = slaveID;
      
      // Envía el struct por ESP-NOW
      esp_now_send(macAddress, (uint8_t*)&msg, sizeof(msg));
    }

    // (Opcional) Getter del ID, si quieres usarlo en otras partes
    int getID() {
      return slaveID;
    }
};

//////////////////////////////////////////////////////////////////////////
// Variables globales
//////////////////////////////////////////////////////////////////////////

// Direcciones MAC de los esclavos (agrega o quita según necesites)
uint8_t macSlave1[6] = {0xF0, 0xF5, 0xBD, 0xFD, 0x41, 0xA8};
uint8_t macSlave2[6] = {0x64, 0xE8, 0x33, 0x87, 0xD3, 0x48};
uint8_t macSlave3[6] = {0x3C, 0x84, 0x27, 0xAF, 0x0D, 0xD0};
uint8_t macSlave4[6] = {0x3C, 0x84, 0x27, 0xAD, 0xC3, 0x9C};
uint8_t macSlave5[6] = {0x18, 0x8B, 0x0E, 0x2A, 0xA3, 0x04};

// Creamos objetos "SlaveDevice" para cada uno de los esclavos
SlaveDevice slave1(macSlave1, 1);
SlaveDevice slave2(macSlave2, 2);
SlaveDevice slave3(macSlave3, 3);
SlaveDevice slave4(macSlave4, 4);
SlaveDevice slave5(macSlave5, 5);

// Callback al enviar datos
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Mensaje enviado a: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito" : "Fallido");
}

// Callback al recibir datos
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  // Determinar el ID del slave basándonos en su MAC
  int senderID = -1;
  if (memcmp(mac_addr, macSlave1, 6) == 0) {
    senderID = 1;
  } else if (memcmp(mac_addr, macSlave2, 6) == 0) {
    senderID = 2;
  } else if (memcmp(mac_addr, macSlave3, 6) == 0) {
    senderID = 3;
  } else if (memcmp(mac_addr, macSlave4, 6) == 0) {
    senderID = 4;
  } else if (memcmp(mac_addr, macSlave5, 6) == 0) {
    senderID = 5;
  }
  
  // Para mayor claridad, se puede imprimir la MAC recibida
  Serial.print("Mensaje recibido de slave con MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" -> ID asignado: ");
  Serial.println(senderID);

  // Se agrega el ID obtenido a la secuencia, sin importar el contenido de returnCommand u otros campos
  userButtonSequence.push_back(senderID);
}


void simonDiceGame() {
  std::vector<int> simonSequence;
  int round = 1;
  bool gameRunning = true;
  
  // Al inicio, apagamos todos los slaves (sin returnCommand = true)
  sleep();
  Serial.println("=== Simon Dice ===");

  while (gameRunning) {
    Serial.print("Ronda ");
    Serial.println(round);
    
    // Agrega un nuevo paso a la secuencia (IDs de 1 a 5)
    int newStep = random(1, 6); // random(1,6) retorna 1,2,3,4 o 5
    simonSequence.push_back(newStep);
    
    // Mostrar la secuencia en el Serial Terminal
    Serial.print("Simon dice: ");
    for (int i = 0; i < simonSequence.size(); i++) {
      Serial.print(simonSequence[i]);
      if (i < simonSequence.size() - 1) {
        Serial.print(" -> ");
      }
    }
    Serial.println();
    
    // Recorre la secuencia y enciende el color correspondiente en cada slave
    for (int id : simonSequence) {
      switch (id) {
        case 1:
          // Slave1: Amarillo (por ejemplo 255,255,0)
          slave1.sendColor(255, 255, 0);
          break;
        case 2:
          // Slave2: Turquesa (64,224,208)
          slave2.sendColor(64, 224, 208);
          break;
        case 3:
          // Slave3: Naranja (255,165,0)
          slave3.sendColor(255, 165, 0);
          break;
        case 4:
          // Slave4: Púrpura (128,0,128)
          slave4.sendColor(128, 0, 128);
          break;
        case 5:
          // Slave5: Azul (0,0,255)
          slave5.sendColor(0, 0, 255);
          break;
      }
      delay(1000); // Permite visualizar el color
      
      // Luego se apaga el slave, pero sin indicar returnCommand
      switch (id) {
        case 1: slave1.sendColor(0, 0, 0); break;
        case 2: slave2.sendColor(0, 0, 0); break;
        case 3: slave3.sendColor(0, 0, 0); break;
        case 4: slave4.sendColor(0, 0, 0); break;
        case 5: slave5.sendColor(0, 0, 0); break;
      }
      delay(500);
    }
    rest();

    // Vaciar la secuencia de botones recibidos antes de esperar la entrada del usuario
    userButtonSequence.clear();
    Serial.println("Presiona los botones en el orden de la secuencia...");

    // Esperamos que se presionen todos los botones de la secuencia
    unsigned long startTime = millis();
    while (userButtonSequence.size() < simonSequence.size()) {
      // Timeout de 10 segundos para evitar esperas infinitas
      if (millis() - startTime > 60000) {
        Serial.println("Tiempo agotado. ¡Perdiste!");
        gameRunning = false;
        break;
      }
      delay(50);
    }
    if (!gameRunning) break;
    
    // Compara la secuencia recibida (botones presionados) con la secuencia generada
    bool correct = true;
    for (int i = 0; i < simonSequence.size(); i++) {
      if (userButtonSequence[i] != simonSequence[i]) {
        correct = false;
        Serial.print("Error en el paso ");
        Serial.println(i + 1);
        break;
      }
    }
    
    if (!correct) {
      Serial.println("Secuencia incorrecta. ¡Perdiste!");
      gameRunning = false;
      red();
      delay(1000);
      start = true;
    } else {
      Serial.println("¡Correcto! Pasas a la siguiente ronda.\n");
      green();
      round++;
      delay(1000);
      sleep();
    }
  }
  
  Serial.print("Juego terminado. Llegaste a la ronda ");
  Serial.println(round);
}

void reflejos() {
  // Inicializamos la semilla para números aleatorios
  randomSeed(millis());
  
  Serial.println("=== Juego de Reflejos ===");
  Serial.println("Ingrese la cantidad de slaves: ");
  // Esperar a que haya datos disponibles
  while (Serial.available() == 0) { delay(10); }
  int n = Serial.parseInt();
  // Limpiar buffer
  while(Serial.available() > 0) { Serial.read(); }
  
  // Creamos un vector con los números de slave de 1 a n
  std::vector<int> slaves;
  for (int i = 1; i <= n; i++) {
    slaves.push_back(i);
  }

  bool jugando = true;
  int turno = 1;

  while (jugando) {
    int indexAleatorio = random(0, n); // índice entre 0 y n-1
    int slaveSeleccionado = slaves[indexAleatorio];

    Serial.print("\nTurno ");
    Serial.print(turno);
    Serial.print(": Slave num ");
    Serial.print(slaveSeleccionado);
    Serial.println(" está encendido.");

    // (Opcional) Aquí se podría enviar algún comando a los slaves para indicar que se encienda el correspondiente.
    // Por ejemplo: enviarColor al slave correspondiente (si se mapea de alguna forma)
    
    Serial.print("Presiona el número del slave seleccionado: ");
    while (Serial.available() == 0) { delay(10); }
    int respuesta = Serial.parseInt();
    // Limpiar buffer
    while(Serial.available() > 0) { Serial.read(); }

    if (respuesta == slaveSeleccionado) {
      Serial.println("¡Correcto! Continuando al siguiente turno...");
      turno++;
    } else {
      Serial.println("¡Incorrecto! El juego ha terminado.");
      Serial.print("Sobreviviste ");
      Serial.print(turno - 1);
      Serial.println(" turno(s).");
      jugando = false;
    }
  }
}


void setup() {
  Serial.begin(115200);

  // Modo Station
  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  // Registrar los callbacks
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Inicializamos cada esclavo agregándolo como "peer"
  if (!slave1.initPeer()) Serial.println("Error al agregar peer 1");
  if (!slave2.initPeer()) Serial.println("Error al agregar peer 2");
  if (!slave3.initPeer()) Serial.println("Error al agregar peer 3");
  if (!slave4.initPeer()) Serial.println("Error al agregar peer 4");
  if (!slave5.initPeer()) Serial.println("Error al agregar peer 5");
}


void loop() {
  if (start) {
    rest();
    start = !start;
  }
  
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equalsIgnoreCase("simon")) {
      simonDiceGame();
    }
    else if (command.equalsIgnoreCase("reflejos")) {
      reflejos();
    }
  }
}


void rest(){
  slave1.sendColor(255, 255, 0);
  slave2.sendColor(64, 224, 208);
  slave3.sendColor(255, 165, 0);
  slave4.sendColor(128, 0, 128);
  slave5.sendColor(0, 0, 255);
}

void green(){
  slave1.sendColor(0, 255, 0);
  slave2.sendColor(0, 255, 0);
  slave3.sendColor(0, 255, 0);
  slave4.sendColor(0, 255, 0);
  slave5.sendColor(0, 255, 0);

}
void red(){
  slave1.sendColor(255, 0, 0);
  slave2.sendColor(255, 0, 0);
  slave3.sendColor(255, 0, 0);
  slave4.sendColor(255, 0, 0);
  slave5.sendColor(255, 0, 0);
}
void sleep(){
  slave1.sendColor(0, 0, 0);
  slave2.sendColor(0, 0, 0);
  slave3.sendColor(0, 0, 0);
  slave4.sendColor(0, 0, 0);
  slave5.sendColor(0, 0, 0);
}
