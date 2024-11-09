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

// Velocidad
float ax, ay, az, ax_anterior, ay_anterior, az_anterior;
float dt = 0.1;
float velocidad_x = 0.0;
float velocidad_y = 0.0;
float velocidad_z = 0.0;

// Variables para funcion de smart delay
float currentMillis = 0;
unsigned long previousMillis = 0;

// Intervalos de tiempo para la lectura de los sensores
const long intervalIMU = 200; // Intervalo de lecturas del IMU
const long intervalPRES = 1000; // Intervalo de lecturas de BMP
const long intervalTEMP = 1000; // Intervalo de lecturas de BMP
const long intervalLUZ = 1000; // Intervalo de lecturas del sensor de luz
const long intervalVEL = 10; // Intervalo de lecturas de velocidad

// Inicializacion de las funciones
void bmp_init();
void imu_init();
void readbmp();
void readIMU();
void readVEL();
float filtro_pasobajo();
void sendSensorData();


void setup()
{
//  Serial.begin(9600); 
//  while (!Serial);
//  Serial.println("CUBEEK Initializing...");
  imu_init();
  bmp_init();
  // communication init:
  LoRa.setPins(8, 9, 2);
  if (!LoRa.begin(433E6))
  {
//    Serial.println("Starting LoRa failed!");
    while (1);
  }

  
}

void loop()
{
  op1 = true; // IMU
  op2 = true; // Presion
  op5 = true; // Velocidad
  
  sendSensorData();   
  delay(100);   
  
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
//  pressure = bmp.readPressure() / 100.0F;
//  tempr = bmp.readTemperature();
}
/* BMP085 Reading Functions End!*/

void readVEL(){
  Wire.beginTransmission(MPU);
  Wire.write(0x3F);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 2, true);
  AccZ = (((Wire.read() << 8 | Wire.read()) / 16384.0)*9.81)+9.81-2.34;// Se suma un error
  
//  az = filtro_pasobajo(AccZ, az_anterior, 0.9);

  // Integra la aceleración para obtener velocidad
  velocidad_z += AccZ * dt;

  // Actualiza las lecturas anteriores
//  az_anterior = az;
  }
  
// Filtro para reduccion de ruido
float filtro_pasobajo(float nuevoValor, float valorAnterior, float alfa) {
  return alfa * nuevoValor + (1 - alfa) * valorAnterior;
}

// Funcion para envio de datos de los sensores
void sendSensorData()
{
  currentMillis = millis()/1000.0; // Se obtiene el tiempo transcurrido para enviar datos cada cierto intervalo de tiempo
  
  //Paquetes de cada sensor para enviar por lora 
  String sensores = "";
  
  sensores += currentMillis;
  //IMU
  if(imu_enabled && op1)
  {
    readIMU();  // Función para leer IMU
    sensores += "\n";
//    sensores += "\n(YPR): ";
    sensores += yaw;
    sensores += ", ";
    sensores += pitch;
    sensores += ", ";
    sensores += roll;
    sensores += "\n";
    }
  
  //Sensor de presion 
  if(bmp_enabled && op2){
    readbmp();  // Función para leer BMP
    sensores += altitude;
    sensores += " m \n";
    }

  // Aceleracion y Velocidad
  if(op5){
    readVEL();
    sensores += AccZ;
    sensores += " m/s2 \n";
    sensores += velocidad_z;
    sensores += " m/s";
    }
  
  LoRa.beginPacket();
  LoRa.print(sensores);
  LoRa.endPacket();
}
