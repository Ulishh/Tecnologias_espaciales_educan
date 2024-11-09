//Librerias lora
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
//Librerias pantalla oled
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//Resolucion oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Definimos la dirección I2C de la pantalla OLED (puede variar según el modelo)
#define OLED_ADDRESS 0x3C

// Crear un objeto para la pantalla OLED usando I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int maxPacketSize = 256; // Maximum expected packet size, adjust as needed


void setup() {
  //Iniciamos el puerto serial
  Serial.begin(9600);
  while (!Serial);

  // Inicializar la pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("No se pudo encontrar una pantalla OLED"));
    //while(1);  // Detener ejecución si no se encuentra la pantalla
  }

  //Configuraciones para lora
  LoRa.setPins(8, 9, 2);
//  Serial.println("LoRa Receiver");
  
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Limpiar la pantalla oled al inicio
  display.clearDisplay();
  display.display();
}

void loop() {
// Recibe los paquetes que mande el OBC por lora y los despliega en el serial y la pantalla oled
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    char receivedData[maxPacketSize];
    int bytesRead = LoRa.readBytes(receivedData, maxPacketSize);
    receivedData[bytesRead] = '\0';
    // Se imprimen los datos recibidos en el serial
    Serial.print(receivedData);
    //Muestra en la pantalla oled el mensaje recibido por lora
    display.clearDisplay();         // Limpiar la pantalla
    display.setTextSize(0.5);         // Establecer el tamaño del texto
    display.setTextColor(WHITE);    // Establecer color del texto
    display.setCursor(0, 0);      // Posicionar el cursor en el centro
    display.print(receivedData);               // Imprimir el número
    display.display();              // Mostrar en la pantalla
    Serial.println();
  }
}  
