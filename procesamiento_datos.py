import time # Libreria para timers
import re  # Libreria para usar expresiones regulares
import numpy as np # Libreria para procesar datos
import sys 
import matplotlib.pyplot as plt
import math
import csv

# Se declaran los vectores para almacenar los datos recibidos de un txt
tiempo = []
alt = []
AccX = []
AccY = []
AccZ = []
GrX = []
GrY = []
GrZ = []


# Función para calcular matriz de rotación
def rotation_matrix(roll, pitch):
    Rx = np.array([[1, 0, 0],
                   [0, np.cos(roll), -np.sin(roll)],
                   [0, np.sin(roll), np.cos(roll)]])
    
    Ry = np.array([[np.cos(pitch), 0, np.sin(pitch)],
                   [0, 1, 0],
                   [-np.sin(pitch), 0, np.cos(pitch)]])
    
    return Rx @ Ry # Rotación total


# Se abre el archivo y dependiendo su terminacion se asignan a su respectivo vector
with open('datos_crudos_40m.txt', 'r') as archivo:
    for linea in archivo:
        # Inicializar una lista para los matches
        matches = []
        if linea.strip().endswith('s'): 
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            tiempo.append(float(matches[0]))
        elif linea.strip().endswith('m'): 
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            alt.append(float(matches[0]))
        elif linea.strip().endswith('ax'): 
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            AccX.append(float(matches[0]))
        elif linea.strip().endswith('ay'):
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            AccY.append(float(matches[0]))
        elif linea.strip().endswith('az'):
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            AccZ.append(float(matches[0]))
        elif linea.strip().endswith('gx'):
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            GrX.append(float(matches[0]))
        elif linea.strip().endswith('gy'):
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            GrY.append(float(matches[0]))
        elif linea.strip().endswith('gz'):
            matches = re.findall(r'-?\d+\.?\d*', linea.strip())
            GrZ.append(float(matches[0]))


muestra = [tiempo,alt,AccX,AccY,AccZ,GrX,GrY,GrZ]
no_muestras = []

# Se obtienen los tamanios de los vectores para obtener el tamanio minimo
for i in range(1,9):
    size = len(muestra[i-1])
    no_muestras.append(size)

size_min = min(no_muestras)
# print(no_muestras)
# print(size_min)

# Se normalizan al mismo tamanio los vectores
for i in range(1,9):
    dif = no_muestras[i-1]-size_min
    while(dif != 0):
        muestra[i-1].pop(0) # Si se utiliza pop() sin argumento elimina el ultimo valor del vector
        dif-=1

# Se obtienen los tamanios de los vectores para obtener el tamanio minimo
no_muestras = []
for i in range(1,9):
    size = len(muestra[i-1])
    no_muestras.append(size)

# print(no_muestras)


# Mostrar grafica de altitud
plt.plot(tiempo, alt, marker='o', linestyle='-', color='b', label='Datos de altitud')  # Línea con puntos
# Personalizar la gráfica
plt.title('Altitud')  
plt.xlabel('Tiempo (s)')  
plt.ylabel('Altitud (m)')  
plt.legend()  
plt.grid(True) 
plt.show()


# Mostrar las graficas de aceleraciones y giros en datos crudos
# Crear subplots (x fila, x columnas) 
fig, ax = plt.subplots(2, 3)  # Figura con 2 gráficas
# Primera gráfica
ax[0, 0].plot(tiempo, AccX, color='b', marker='o')
ax[0, 0].set_title('Aceleracion X')
ax[0, 0].set_xlabel('tiempo')
ax[0, 0].set_ylabel('acc')

ax[0, 1].plot(tiempo, AccY, color='r', marker='o')
ax[0, 1].set_title('Aceleracion Y')
ax[0, 1].set_xlabel('tiempo')
ax[0, 1].set_ylabel('acc')

ax[0, 2].plot(tiempo, AccZ, color='g', marker='o')
ax[0, 2].set_title('Aceleracion Z')
ax[0, 2].set_xlabel('tiempo')
ax[0, 2].set_ylabel('acc')

ax[1, 0].plot(tiempo, GrX, color='orange', marker='o')
ax[1, 0].set_title('Giro X')
ax[1, 0].set_xlabel('tiempo')
ax[1, 0].set_ylabel('giro')

ax[1, 1].plot(tiempo, GrY, color='purple', marker='o')
ax[1, 1].set_title('Giro Y')
ax[1, 1].set_xlabel('tiempo')
ax[1, 1].set_ylabel('giro')

ax[1, 2].plot(tiempo, GrZ, color='brown', marker='o')
ax[1, 2].set_title('Giro Z')
ax[1, 2].set_xlabel('tiempo')
ax[1, 2].set_ylabel('giro')

# Mostrar las gráficas
plt.tight_layout()  # Ajusta el espacio entre subplots
plt.show()

# Se declaran las variables iniciales
alt_norm = []
vel_alt = []
roll = []
pitch = []
yaw = []
ang_x_prev = 0
ang_y_prev = 0
ang_z_prev = 0
girosc_ang_x_prev = 0
girosc_ang_y_prev = 0
AccX_norm = []
AccY_norm = []
AccZ_norm = []
a_vertical_filtrada = 0
AccZ_local = []
AccX_glob = []
AccY_glob = []
AccZ_glob = []
AccT_pitagoras = []
Velocidad_local_Z = []
Velocidad_global_Z = []
Velocidad_pitagoras = []
vel1 = 0
vel2 = 0
vel3 = 0

######################## Procesamiento de los datos ##############################
for i in range(1,size_min):
    # dt = (tiempo[i]-tiempo[i-1])
    dt = 0.04

    ## Calcular la altitud con respecto al suelo
    alt_norm.append(alt[i]-158)
    ## Calcular velocidad con la derivada de la altitud
    
    
    #### Calcular los ángulos con acelerometro
    accel_ang_x = math.atan2((AccY[i]),math.sqrt(AccX[i]**2 + AccZ[i]**2))
    # accel_ang_x = math.atan2((AccY[i]),(math.sqrt(AccX[i]**2 + AccZ[i]**2)))
    accel_ang_y = math.atan2((-AccX[i]),(math.sqrt(AccY[i]**2 + AccZ[i]**2)))
    # Calcular angulo de rotación con giroscopio y filtro complemento  
    ang_x = 0.98*(ang_x_prev+((GrX[i]/131)*math.pi/180)*dt) + 0.02*accel_ang_x
    ang_y = 0.98*(ang_y_prev+((GrY[i]/131)*math.pi/180)*dt) + 0.02*accel_ang_y
    ang_z = ang_z_prev+((GrZ[i]/131))*dt
    ang_x_prev=ang_x
    ang_y_prev=ang_y
    ang_z_prev=ang_z

    # Calculo de los angulos sin filtro
    girosc_ang_x = (GrX[i]/131)*dt + girosc_ang_x_prev
    girosc_ang_y = (GrY[i]/131)*dt + girosc_ang_y_prev
    girosc_ang_x_prev=girosc_ang_x
    girosc_ang_y_prev=girosc_ang_y
    
    roll.append(ang_x*180/math.pi)
    pitch.append(ang_y*180/math.pi)
    yaw.append(ang_z)

    
    #### Se calculan las aceleraciones
    # Normalizacion de los valores
    ax_m_s2 = AccX[i] * (9.81/16384.0)
    AccX_norm.append(ax_m_s2)
    ay_m_s2 = AccY[i] * (9.81/16384.0)
    AccY_norm.append(ay_m_s2)
    az_m_s2 = AccZ[i] * (9.81/16384.0)
    AccZ_norm.append(az_m_s2) 

    ## Primer metodo: Suponer que el cansat cayo completamente vertical
    AccZ_local.append(az_m_s2+9.81) 
    

    ## Segundo metodo: Calculo de la aceleracion paralela a la gravedad con matrices de rotacion
    # calculo con matrices de rotacion con angulos en radianes
    matriz_rotacion = rotation_matrix(ang_x, ang_y)
    acc_local = np.array([[ax_m_s2],[ay_m_s2],[az_m_s2]])
    acc_global = matriz_rotacion @ acc_local
    AccX_glob.append(acc_global[0])
    AccY_glob.append(acc_global[1])
    AccZ_glob.append(acc_global[2]+9.81)


    # Tercer metodo: Teorema de pitagoras en 3D
    a_vertical = math.sqrt(ax_m_s2**2 + ay_m_s2**2 + az_m_s2**2) - 9.81
    AccT_pitagoras.append(a_vertical)
    # a_vertical_filtrada = 0.6 * a_vertical_filtrada + 0.4 * a_vertical



    # Calculo de la Velocidad con la integral de la aceleracion
    # 1.-
    vel1 = vel1 + AccZ_local[i-1]*dt
    Velocidad_local_Z.append(vel1)
    # 2.-
    vel2 = vel2 + AccZ_glob[i-1]*dt
    Velocidad_global_Z.append(vel2)
    # 3.-
    vel3 = vel3 + AccT_pitagoras[i-1]*dt
    Velocidad_pitagoras.append(vel3)


# # Nombre del archivo 
# archivo_csv = "vel_global.csv"
# # Escribir en el archivo CSV
# with open(archivo_csv, mode='w', newline='') as archivo:
#     escritor = csv.writer(archivo)
#     escritor.writerow(["Tiempo", "Velocidad"])  # Escribir encabezados
#     for t, a in zip(tiempo, Velocidad_global_Z):
#         escritor.writerow([t, a])  # Escribir los datos
    
    

######################### Graficacion ################################
# Se elimina el primer valor del vector de tiempo para que sea de la misma dimension que los demas vectores
tiempo.pop(0)
print("Datos para calcular velocidad con altitud")
print("Tiempo: x1: " + str(tiempo[66]) + "     x2: " + str(tiempo[130]))
print("Altitud: y1: " + str(alt_norm[66]) + "     y2: " + str(alt_norm[130]))
# Mostrar grafica de altitud normalizada
plt.plot(tiempo, alt_norm, marker='o', linestyle='-', color='b', label='Datos de altitud')  # Línea con puntos
# Personalizar la gráfica
plt.title('Altitud')  
plt.xlabel('Tiempo (s)')  
plt.ylabel('Altitud (m)')  
plt.legend()  
plt.grid(True) 
plt.show()

# Mostrar las graficas de aceleraciones y giros normalizadas
# Crear subplots (x fila, y columnas) 
fig, ax = plt.subplots(2, 3)  # Figura con x*y gráficas
# Primera gráfica
ax[0, 0].plot(tiempo, AccX_norm, color='b', marker='o')
ax[0, 0].set_title('Aceleracion X')
ax[0, 0].set_xlabel('tiempo')
ax[0, 0].set_ylabel('acc')

ax[0, 1].plot(tiempo, AccY_norm, color='r', marker='o')
ax[0, 1].set_title('Aceleracion Y')
ax[0, 1].set_xlabel('tiempo')
ax[0, 1].set_ylabel('acc')

ax[0, 2].plot(tiempo, AccZ_norm, color='g', marker='o')
ax[0, 2].set_title('Aceleracion Z')
ax[0, 2].set_xlabel('tiempo')
ax[0, 2].set_ylabel('acc')

ax[1, 0].plot(tiempo, roll, color='orange', marker='o')
ax[1, 0].set_title('Roll')
ax[1, 0].set_xlabel('Tiempo (s)')
ax[1, 0].set_ylabel('Grados (°)')

ax[1, 1].plot(tiempo, pitch, color='purple', marker='o')
ax[1, 1].set_title('Pitch')
ax[1, 1].set_xlabel('Tiempo (s)')
ax[1, 1].set_ylabel('Grados (°)')

ax[1, 2].plot(tiempo, yaw, color='brown', marker='o')
ax[1, 2].set_title('Yaw')
ax[1, 2].set_xlabel('Tiempo (s)')
ax[1, 2].set_ylabel('Grados (°)')

# Mostrar las gráficas
plt.tight_layout()  # Ajusta el espacio entre subplots
plt.show()





# Mostrar las graficas de aceleracion normalizadas junto con las aceleraciones globales calculadas con matrices de rotacion
# Crear subplots (x fila, y columnas) 
fig, ax = plt.subplots(2, 3)  # Figura con x*y gráficas
# Primera gráfica
ax[0, 0].plot(tiempo, AccX_norm, color='b', marker='o')
ax[0, 0].set_title('Aceleracion X')
ax[0, 0].set_xlabel('tiempo')
ax[0, 0].set_ylabel('acc')

ax[0, 1].plot(tiempo, AccY_norm, color='r', marker='o')
ax[0, 1].set_title('Aceleracion Y')
ax[0, 1].set_xlabel('tiempo')
ax[0, 1].set_ylabel('acc')

ax[0, 2].plot(tiempo, AccZ_local, color='g', marker='o')
ax[0, 2].set_title('Aceleracion Z')
ax[0, 2].set_xlabel('tiempo')
ax[0, 2].set_ylabel('acc')

ax[1, 0].plot(tiempo, AccX_glob, color='orange', marker='o')
ax[1, 0].set_title('Aceleracion global X')
ax[1, 0].set_xlabel('tiempo')
ax[1, 0].set_ylabel('acc')

ax[1, 1].plot(tiempo, AccY_glob, color='purple', marker='o')
ax[1, 1].set_title('Aceleracion global Y')
ax[1, 1].set_xlabel('tiempo')
ax[1, 1].set_ylabel('acc')

ax[1, 2].plot(tiempo, AccZ_glob, color='brown', marker='o')
ax[1, 2].set_title('Aceleracion global Z')
ax[1, 2].set_xlabel('tiempo')
ax[1, 2].set_ylabel('acc')

# Mostrar las gráficas
plt.tight_layout()  # Ajusta el espacio entre subplots
plt.show()




# Mostrar las graficas de aceleracion con los 2 metodos y sus velocidades
# Crear subplots (x fila, y columnas) 
fig, ax = plt.subplots(2, 2)  # Figura con x*y gráficas
# Primera gráfica
ax[0, 0].plot(tiempo, AccZ_local, color='b', marker='o')
ax[0, 0].set_title('Aceleracion local en eje Z')
ax[0, 0].set_xlabel('Tiempo (s)')
ax[0, 0].set_ylabel('Aceleración (m/s^2)')
ax[0, 0].grid(True)  # Agrega la cuadrícula

ax[0, 1].plot(tiempo, AccZ_glob, color='r', marker='o')
ax[0, 1].set_title('Aceleracion global en eje Z')
ax[0, 1].set_xlabel('Tiempo (s)')
ax[0, 1].set_ylabel('Aceleración (m/s^2)')
ax[0, 1].grid(True)  # Agrega la cuadrícula

# ax[0, 2].plot(tiempo, AccT_pitagoras, color='g', marker='o')
# ax[0, 2].set_title('Aceleracion total con pitagoras')
# ax[0, 2].set_xlabel('tiempo')
# ax[0, 2].set_ylabel('acc')

ax[1, 0].plot(tiempo, Velocidad_local_Z, color='orange', marker='o')
ax[1, 0].set_title('Velocidad local en eje Z')
ax[1, 0].set_xlabel('Tiempo (s)')
ax[1, 0].set_ylabel('Velocidad (m/s)')
ax[1, 0].grid(True)  # Agrega la cuadrícula

ax[1, 1].plot(tiempo, Velocidad_global_Z, color='purple', marker='o')
ax[1, 1].set_title('Velocidad global en eje Z')
ax[1, 1].set_xlabel('Tiempo (s)')
ax[1, 1].set_ylabel('Velocidad (m/s)')
ax[1, 1].grid(True)  # Agrega la cuadrícula


# ax[1, 2].plot(tiempo, Velocidad_pitagoras, color='brown', marker='o')
# ax[1, 2].set_title('Velocidad total pitagoras')
# ax[1, 2].set_xlabel('tiempo')
# ax[1, 2].set_ylabel('vel')

# Mostrar las gráficas
plt.tight_layout()  # Ajusta el espacio entre subplots
plt.show()
