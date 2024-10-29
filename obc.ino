#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <Adafruit_BMP085.h>

//LORA
const int maxPacketSize = 256; // Maximum expected packet size, adjust as needed

//Banderas para envio de sensores dependiendo de la opcion seleccionada desde el GS
bool op1, op2, op3, op4, op5;
bool seleccion_sensor = false;
bool seleccion_tiempo = false;
int tiempo_sensado = 0;
int tiempo_inicial = 0;
int tiempo_actual = 0;

// MPU6050 variables:
const int MPU = 0x69; // MPU6050 I2C address for AD0 High
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
bool imu_enabled = false;

// BMP085 variables:
Adafruit_BMP085 bmp;
float altitude, pressure, tempr;
bool bmp_enabled = false;

// Sensor de luz 
const int fotoresistecia = A1;// Pin LDR
const long A = 1000;     //Resistencia en oscuridad en KΩ
const int B = 15;        //Resistencia a la luz (10 Lux) en KΩ
const int Rc = 10;       //Resistencia calibracion en KΩ

//// Sensor de Temperatura 
//const int thermistor = A2;
//const int resistenciaFija = 4700;  // Resistencia de 10 kΩ
//// Constantes para el cálculo de la temperatura usando la ecuación de Steinhart-Hart
//const float beta = 3950;      // Coeficiente Beta del termistor
//const float resistencia25C = 4700;  // Resistencia del termistor a 25 °C
//const float temperatura0K = 273.15;  // Temperatura cero absoluta en Kelvin


// Variables para funcion de smart delay
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

// Intervalos de tiempo para la lectura de los sensores
const long intervalIMU = 200; // 5 segundos entre lecturas de GPS
const long intervalPRES = 1000; // 0.5 segundos entre lecturas de BMP
const long intervalTEMP = 1000; // 1 segundo entre lecturas de BMP
const long intervalLUZ = 1000; // 0.5 segundos entre lecturas de BMP

// Inicializacion de las funciones
void bmp_init();
void imu_init();
void readbmp();
void readIMU();
void sendSensorData();


void setup()
{
//  Serial.begin(9600); 
//  while (!Serial);
//  Serial.println("CUBEEK Initializing...");

  // communication init:
  LoRa.setPins(8, 9, 2);
  if (!LoRa.begin(433E6))
  {
//    Serial.println("Starting LoRa failed!");
    while (1);
  }

  imu_init();
  bmp_init();
}

void loop()
{
// Condicional en la cual se selecciona el sensor para su lectura y envio
  if (!seleccion_sensor && !seleccion_tiempo){
  
    // Escucha de lora en espera de un comando
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        char receivedData[maxPacketSize];
        int bytesRead = LoRa.readBytes(receivedData, maxPacketSize);
    
        receivedData[bytesRead] = '\0';
        
        // Condiciones para activar la bandera de la opcion seleccionada desde el GS y para leer el sensor que se requiere
        //IMU
        if (strcmp(receivedData, "a") == 0){
          seleccion_sensor = true;
          op1 = true;
        } 
        //Presion
        if (strcmp(receivedData, "b") == 0){
          seleccion_sensor = true;
          op2 = true;
        } 
        //Luz
        if (strcmp(receivedData, "c") == 0){
          seleccion_sensor = true;
          op3 = true;
        } 
        // Temp BMP
        if (strcmp(receivedData, "d") == 0){
          seleccion_sensor = true;
          op4 = true;
        } 
//        // Temp termistor
//        if (strcmp(receivedData, "e") == 0){
//          seleccion_sensor = true;
//          op5 = true;
//        }   
    }
  }
  // Condicional para el seleccionar el tiempo de sensado
  else if (seleccion_sensor && !seleccion_tiempo){
    // Escucha de lora en espera de un comando
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        char receivedData[maxPacketSize];
        int bytesRead = LoRa.readBytes(receivedData, maxPacketSize);
    
        receivedData[bytesRead] = '\0';
        
        // Condiciones para asignar el tiempo de sensado
        //10 segundos de datos
        if (strcmp(receivedData, "a") == 0){
          tiempo_sensado = 20;
          seleccion_tiempo = true;
        }
        else if (strcmp(receivedData, "b") == 0){
          tiempo_sensado = 40;
          seleccion_tiempo = true;
        }
        else if (strcmp(receivedData, "c") == 0){
          tiempo_sensado = 60;
          seleccion_tiempo = true;
        }
    }
  }
// Condicional para enviar los datos de los sensores por el tiempo de sensado seleccionado
  if (seleccion_sensor && seleccion_tiempo){
    tiempo_inicial = millis()/1000;
    while(tiempo_actual-tiempo_inicial < (tiempo_sensado)){
      tiempo_actual = millis()/1000;
      sendSensorData();      
    }
    // Se vuelven todas las banderas en false
    seleccion_tiempo = false;
    seleccion_sensor = false;
    op1 = false;
    op2 = false;
    op3 = false;
    op4 = false;
//    op5 = false;
  }
}


/* Initialization Code Start!*/
void bmp_init()
{
  bmp_enabled = true;
  if (!bmp.begin())
  {
//    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1);
  }
}

void imu_init()
{
  imu_enabled = true;
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  calculate_IMU_error();
}

/* Initialization Code End!*/

// Funcion para calibrar el sensor una vez que se inicializa
/* IMU Reading Functions Start!*/
void calculate_IMU_error()
{
  int c = 0;

  while (c < 200)
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccErrorX += atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI;
    AccErrorY += atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI;
    c++;
  }
  AccErrorX /= 200;
  AccErrorY /= 200;

  c = 0;

  while (c < 200)
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroErrorX += GyroX;
    GyroErrorY += GyroY;
    GyroErrorZ += GyroZ;
    c++;
  }
  GyroErrorX /= 200;
  GyroErrorY /= 200;
  GyroErrorZ /= 200;
}

void readIMU() 
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + AccErrorY;

  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000;

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  GyroX -= GyroErrorX;
  GyroY -= GyroErrorY;
  GyroZ -= GyroErrorZ;

  gyroAngleX += GyroX * elapsedTime;
  gyroAngleY += GyroY * elapsedTime;
  
  yaw += GyroZ * elapsedTime;
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;
}
/* IMU Reading Functions End!*/


/* BMP085 Reading Functions Start!*/
void readbmp()
{
  altitude = bmp.readAltitude();
  pressure = bmp.readPressure() / 100.0F;
  tempr = bmp.readTemperature();
}
/* BMP085 Reading Functions End!*/



// Funcion para envio de datos de los sensores
void sendSensorData()
{
  currentMillis = millis(); // Se obtiene el tiempo transcurrido para enviar datos cada cierto intervalo de tiempo

  //Paquetes de cada sensor para enviar por lora 
  String imu = "";
  String presion = "";
  String tempBMP = "";
  String luz = "";
//  String temp = "";
  

  //IMU
  if(imu_enabled && op1)
  {
    if (currentMillis - previousMillis >= intervalIMU) {
      previousMillis = currentMillis;
      readIMU();  // Función para leer IMU
      imu += "(YPR): ";
      imu += yaw;
      imu += ", ";
      imu += pitch;
      imu += ", ";
      imu += roll;
      imu += "\n";
      LoRa.beginPacket();
      LoRa.print(imu);
      LoRa.endPacket();
    }
  }
  
  //Sensor de presion 
  if(bmp_enabled && op2){
    if (currentMillis - previousMillis >= intervalPRES) {
      previousMillis = currentMillis;
      readbmp();  // Función para leer BMP
      presion += pressure;
      presion += " hPa\n";
      LoRa.beginPacket();
      LoRa.print(presion);
      LoRa.endPacket();
      }
  }
  
  // Sensor de luz
  if(op3){
    if (currentMillis - previousMillis >= intervalLUZ) {
      previousMillis = currentMillis;
      int V = analogRead(fotoresistecia);
      int brightness = ((long)V*A*10)/((long)B*Rc*(1024-V));    //usar si LDR entre A1 y Vcc
      luz += "Brightness: ";
      luz += brightness;
      luz += " lm\n";
      LoRa.beginPacket();
      LoRa.print(luz);
      LoRa.endPacket();    
    }
  }

  //Sensor de temperatura bmp 
  if(bmp_enabled && op4){
    if (currentMillis - previousMillis >= intervalTEMP) {
      previousMillis = currentMillis;
      readbmp();  // Función para leer BMP
      tempBMP += tempr;
      tempBMP += " C\n";
      LoRa.beginPacket();
      LoRa.print(tempBMP);
      LoRa.endPacket();
      }
  }

  // Sensor de temperatura thermistor
//  if(bmp_enabled && op5){
//    if (currentMillis - previousMillis >= intervalTEMP) {
//      previousMillis = currentMillis;
//      int valorAnalogico = analogRead(thermistor);
//      // Convertir el valor leído en voltaje
//      float voltaje = valorAnalogico * (3.3 / 1023.0);
//      // Calcular la resistencia del termistor
//      float resistenciaTermistor = (3.3 / voltaje - 1) * resistenciaFija;
////      // Calcular la temperatura en Kelvin usando la ecuación de Steinhart-Hart
////      float temperaturaK = 1.0 / (1.0 / (temperatura0K + 25) + log(resistenciaTermistor / resistencia25C) / beta);
////      // Convertir la temperatura a Celsius
////      float temperaturaC = temperaturaK - temperatura0K;
//      float temperaturaC = (beta * 298.15) / (beta + (298.15 * log(resistenciaTermistor / resistencia25C))) - 273.15;
//      temp += temperaturaC;
//      temp += " C\n";
//      LoRa.beginPacket();
//      LoRa.print(temp);
//      LoRa.endPacket();
//      }
//  }
}
