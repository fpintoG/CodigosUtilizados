# -*- coding: utf-8 -*-
"""
Created on Thu Jan  4 17:44:03 2018

@author: odroid
"""

import serial, json, numpy as np, time, socket, os
import datetime
import collections
import requests
import threading

from testLoadedModel import load_model
from keras.preprocessing import sequence

np.set_printoptions(precision=3)

validation = 1

def portIsUsable(port):
    try:
       ser = serial.Serial(port, 115200, timeout=1.0)
       ser.readline()
       return True
    except:
       return False

def reading(val, d, nroS):
    arrDato = np.zeros(d)
    val.write(nroS.encode())
    time.sleep(.005)
    datoT =  val.readline()

    if len(str(datoT)) > 0:
        arrDatoAux =  datoT.decode().split(',')
        arrDato = arrDatoAux[:-1][::-1]
        arrDato.append(arrDatoAux[8][:-2])
    return arrDato


def readingDHT(val, d, nroS):
    arrDato = np.zeros(d)
    val.write(nroS.encode())
    time.sleep(.005)
    datoT = val.readline()
    if len(str(datoT)) > 0:
        arrDatoAux =  datoT.decode().split(' ')
#        arrDato = arrDatoAux[:-1][::-1]
#        arrDato.append(arrDatoAux[8][:-2])
    return arrDatoAux


def functionTest():
    puertoSerial = '/dev/ttyS2'
    count = 0
    prediction = 0
    if portIsUsable(puertoSerial):
        print('Is usable')
    else:
        print('Is not usable')

    if portIsUsable(puertoSerial):
        valor = serial.Serial(puertoSerial, 9600, timeout=1.);
        #inicia sesion con base de datos
        url = 'http://200.14.68.107/modulotomas.php?valor='
        r = requests.session()
        r.trust_env = False
        #modelo a utilizar
        model = load_model('conv_lstm_model10')
        model.summary()
        #buffer circular
        window = collections.deque(maxlen=60)
        #buffer que almacena predicciones para obtener promedio
        predictionBuffer = collections.deque(maxlen=20)
        dimArr = 9

        #arribaBuff,abajoBuff = np.zeros((16), np.float), np.zeros((16), np.float)
        data = np.zeros((32), np.float32)
        while True:
             arrValorT = reading( valor, dimArr, str(1) )
             arrT1 = arrValorT[:8]
             arrValorT = reading( valor, dimArr, str(2) )
             arrT2 = arrValorT[:8]
             arrValorT = reading( valor, dimArr, str(3) )
             arrT3 = arrValorT[:8]
             arrValorT = reading( valor, dimArr, str(4) )
             arrT4 = arrValorT[:8]
             #almacena datos de cada sensor
             arribaIzq = np.array(arrT1 , dtype='f')
             arribaDer = np.array(arrT2 , dtype='f')
             abajoIzq = np.array(arrT3 , dtype='f')
             abajoDer = np.array(arrT4 , dtype='f')

             for x in range(8):
                 data[x] = arribaIzq[x]
                 data[x + 8] = arribaDer[7-x]
                 data[x + 16] = abajoIzq[x]
                 data[x + 24] = abajoDer[7-x]

             #guarda datos en buffer circular
             window.append(data)

             #modelo en accion
             #parte desde 60 (20 segundos aprox) para evitar errores de prediccion al pricipio
             if(len(window) == 60):
                 seq = np.asarray(window)
                 prediction = model.predict_classes(seq.reshape(1, seq.shape[0], seq.shape[1]), verbose=0, batch_size=1)
                 predictionBuffer.append(prediction)

                 if((np.mean(predictionBuffer) >= 0.5) and (len(predictionBuffer) == 20)):
                     print("caida")
                     try:
                         r.post(url + ' '.join(str("{0:.2f}".format(e)) for e in data) + ' ' + str(np.mean(predictionBuffer)))

                     except:
                         print("Seding data caidas")
             else:
                 prediction = 0
             #cuenta tiempo de medicion
             count = count + 1
             now  = datetime.datetime.now()

             #print( "tiempo actual: " + str(now.minute))
             saveInfo1 = now.strftime("%Y-%m-%d %H:%M:%S") + " " + ' '.join(str("{0:.2f}".format(e)) for e in data) + " * " + str(np.mean(predictionBuffer)) + "\n"

             nameFile = "caidas/" + str(now.day) + "-" + str(now.hour) + "-test"  + ".txt"
             f = open(nameFile,"a")
             f.writelines(saveInfo1)
             f.close()
             if((count) == 1000):
                 print( "sending data to database ")
                 try:
                     r.post(url + ' '.join(str("{0:.2f}".format(e)) for e in data) + ' ' + str(np.mean(predictionBuffer)))
                 except:
                     print("Error: unable to connect to server")

             if(count == 1200):
                 print("sending dht11 data to database ")
                 dhtData = readingDHT(valor, 3, str(5))
                 try:
                     r.post(url + ' '.join(str(e) for e in dhtData))
                 except:
                     print("Error: unable to connect to server")

                 count = 0


functionTest()
