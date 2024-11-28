// Librerias I2C para controlar el mpu6050
// la libreria MPU6050.h necesita I2Cdev.h, I2Cdev.h necesita Wire.h
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
// Libreria para la el BMP
#include <Adafruit_BMP085.h>
// Libreria para la comunicacion con lora
#include <LoRa.h>
#include <math.h>


/*********** IMU ***********/
// La dirección del MPU6050 puede ser 0x68 o 0x69, dependiendo 
// del estado de AD0. Si no se especifica, 0x68 estará implicito
//MPU6050 sensor;
MPU6050 sensor(0x69);
// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
int ax, ay, az;
int gx, gy, gz;
long tiempo_prev;
float dt;
float ang_x, ang_y;
float ang_x_prev, ang_y_prev;
float a_vertical, velocidad, tiempo_anterior;

/*********** LORA ************/
const int maxPacketSize = 256; // Maximum expected packet size, adjust as needed
float tiempo = 0;

/*********** BMP **************/
Adafruit_BMP085 bmp;
float altitude, pressure, tempr;
bool bmp_enabled = false;


/*********** GPS **************/
float currentMillis = 0;
unsigned long previousMillis = 0;



void setup() {
//  Serial.begin(57600);    //Iniciando puerto serial
  Wire.begin();           //Iniciando I2C  
  sensor.initialize();    //Iniciando el sensor

//  if (sensor.testConnection()) Serial.println("Sensor iniciado correctamente");
//  else Serial.println("Error al iniciar el sensor");
  
  LoRa.setPins(8, 9, 2);
  if (!LoRa.begin(433E6))
  {
//    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  if (!bmp.begin())
  {
//    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1);
  }

  // Valores de una calibracion previa del IMU
  sensor.setXAccelOffset(-2038);
  sensor.setYAccelOffset(-1071);
  sensor.setZAccelOffset(1468);
  sensor.setXGyroOffset(-18);
  sensor.setYGyroOffset(-64);
  sensor.setZGyroOffset(-32);
}

void loop() {
  // Paquete para mandar por lora
  String sensores = "";
  
  // Tiempo
  tiempo = millis()/1000.0; // Se obtiene el tiempo transcurrido para enviar datos cada cierto intervalo de tiempo
  sensores += tiempo;
  sensores += " s \n";
  
  // Altitud
  altitude = bmp.readAltitude();
  sensores += altitude;
  sensores += " m \n";

  // IMU - angulo de rotacion
  // Leer las aceleraciones y velocidades angulares
  sensor.getAcceleration(&ax, &ay, &az);
  sensor.getRotation(&gx, &gy, &gz);
  
  sensores += ax;
  sensores += " ax\n";
  sensores += ay;
  sensores += " ay\n";
  sensores += az;
  sensores += " az\n";
  sensores += gx;
  sensores += " gx\n";
  sensores += gy;
  sensores += " gy\n";
  sensores += gz;
  sensores += " gz\n";

  // Envio del paquete de lora
  LoRa.beginPacket();
  LoRa.print(sensores);
  LoRa.endPacket();
  delay(100);
}
