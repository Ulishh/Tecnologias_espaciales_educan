import serial # Libreria para leer el serial
import time
# Configurar comunicacion serial con Arduino
serial_port = serial.Serial('COM14', 9600)  # Cambiar el puerto COM si es necesario
time.sleep(2)  # Esperar a que el puerto serial se estabilice

while (True):
    if serial_port.in_waiting > 0:  # Si hay datos disponibles
        try:
            # Lee los datos del puerto serial y decodificalos
            datos = serial_port.read(serial_port.in_waiting).decode('utf-8').strip()
            with open("datos_crudos.txt", "a") as archivo:
                archivo.write(datos + "\n" + "\n")
        except UnicodeDecodeError as e:
            # Maneja el error de decodificacion sin cerrar el programa
            print(f"Error de decodificacion: {e}")    

     