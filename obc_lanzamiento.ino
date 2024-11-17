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
  Serial.begin(57600);    //Iniciando puerto serial
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
  
  dt = (millis()-tiempo_prev)/1000.0;
  tiempo_prev=millis();
  //Calcular los ángulos con acelerometro
  float accel_ang_x=atan(ay/sqrt(pow(ax,2) + pow(az,2)))*(180.0/PI);
  float accel_ang_y=atan(-ax/sqrt(pow(ay,2) + pow(az,2)))*(180.0/PI);
  //Calcular angulo de rotación con giroscopio y filtro complemento  
  ang_x = 0.98*(ang_x_prev+(gx/131)*dt) + 0.02*accel_ang_x;
  ang_y = 0.98*(ang_y_prev+(gy/131)*dt) + 0.02*accel_ang_y;
  ang_x_prev=ang_x;
  ang_y_prev=ang_y;
  sensores += ang_x;
  sensores += "   ";
  sensores += ang_y;
  sensores += "\n";

  // IMU - Velocidad
  // Normalizacion de los valores
  float ax_m_s2 = ax * (9.81/16384.0);
  float ay_m_s2 = ay * (9.81/16384.0);
  float az_m_s2 = az * (9.81/16384.0);
  a_vertical = ax_m_s2*sin(ang_y*PI/180) + ay_m_s2*sin(ang_x*PI/180) + az_m_s2*cos(ang_y*PI/180)*cos(ang_x*PI/180);
  a_vertical += 9.81;
  sensores += a_vertical;
  sensores += " m/s2 \n";
  // Calcular tiempo transcurrido
  dt = (millis() - tiempo_anterior) / 1000.0;
  tiempo_anterior = millis();
  // Calculo de la velocidad por medio de la derivada
  velocidad += a_vertical*dt;
  sensores += velocidad;
  sensores += " m/s";

  // Envio del paquete de lora
  LoRa.beginPacket();
  LoRa.print(sensores);
  LoRa.endPacket();
  delay(100);
}
