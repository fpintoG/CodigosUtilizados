
Este repositorio contiene los codigos para implementar los siguientes sistemas:

- Sistema de deteccion de caidas inteligente: 

Este sistema recibe 32 pixeles de temperatura provenientesde un arreglo compuesto por 4 sensores 
de temperatura Omron D6T. Con esta información se implementa un algoritmo de machine learning para 
detectar en tiempo real si una persona se calló en el rango de visión. Por otra parte, se lee un sensor de 
temperatura y humedad DTH11 junto a otro sensor de gas (este último aún no se agrega a la placa).

Luego el sistema se comunica con el servidor para enviar una alerta de deteccion de caidas y también de forma
periódica cada 5 minutos. Además se guardan los datos en el subdirectorio ModelosCaidas/caidas. 
También se envían datos de temperatura y humedad cada 5 minutos (de igual forma podría hacerlo
el de gas si este se implementa en la placa).

El código encargado de leer cada uno de los sensores implementados y posteriormente enviar los datos por comunicación
serial se encuentra en el directorio arduinoCode/SensorTermico4en1 (este debe ser cargado al arduino que ira en la placa).
En la Odroid se debe correr el script ubicado en ModelosCaidas/fall_detection.py que carga el modelo de red neuronal 
seleccionado para realizar la detección.

Se implementa un bash script encargado de eliminar registros de archivos con lecturas de los sensores termicos que 
tengan una antiguedad mayor a 2 días. Este script se encuentra en ModelosCaidas/deleteFiles.sh y corre automanticamente
en el sistema operativo de la Odroid, cuya hora de aplicación se puede modificar al ingresar el comando 'crontab -e' en
el comand window.
		

- Datalogger: 	

Guarda los datos entregados por el sensor de temperatura Omron D6T en una tarjeta microSD.
Para correr la implementación se debe cargar en una placa de arduino uno o mega el código 
se encuentra en el directorio arduinoCode/Datalogger.
              			  

- Módulo wifi: 

Se implementa un módulo ESP8266 para enviar datos de temperatura, humedad y gas.
El programa correspondiente se encuentra en el directorio arduinoCode/ModuloWiFi.	

- Deteccion de anomalías nicturia: 

Se implementó un algoritmo no supervisado para detectar anomalías en series de tiempo provenientes de los
sensores de conductividad sin requerir un etiquetado previo de los datos (puede funcionar con otro tipo de sensores
que entreguen datos numéricos).
El archivo de entrada debe ser una serie de tiempo con el formato de 'input_file4.csv' (mantener los encabezados
en las primeras 3 lineas) donde la primera columna tiene fecha y hora y la segunda la lectura de los sensores.

Como prerrequisito para utilizar el algoritmo se debe instalar la biblioteca 'nupic', 
en linux seria de la forma (en command line):

pip install nupic

Para correr el programa dirigirse al directorio 'anomalyDetection' y correr el programa de la siguiente forma:

python detection.py input_file.csv output_file.csv

Donde el primer argumento es el archivo de entrada que debe tener el formato previamente descrito y la salida
será output_file.csv que solo muestra las mediciones donde se detecto una anomalia (el programa corre en python 2.7).

Para generar los archivos con el formate necesario utilicé una función que se llama 'wrap_files()' 
y se encuentra dentro del codigo 'file_wrapper.py' ubicado en el directorio nicturiaFolder. Sin embargo después
se debe modificar el archivo creado para agregar el encabezado (ver 'input_file4.csv' en el directorio anomalyDetection) 
