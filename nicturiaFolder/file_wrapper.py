import glob
import pandas as pd
import numpy as np
import csv
import os
def wrap_files():
	"""
	wrap a file into one keeping just 1 sample per second
	
	En este caso busca en la carpeta de la persona Patricio Ramirez 
	y genera un archivo de texto que incluye las mediciones por segundo de 
	todo el dia 18 del mes 02.
	Como salida genera un archivo de nombre 1802out.csv.
	"""
	read_files = glob.glob("Patricio_Ramirez/*1802.txt")

	with open("1802out.txt","w+") as fout:
		for file in read_files:
			f = open(file, "r+")
			lineTempLast = ' '
			name = os.path.basename(file)
			
			for num, line in enumerate(f):
				lineTemp = line.split(';')
				lineTemp1 = lineTemp[0]	
				if lineTemp1 != lineTempLast:
					fout.write("2016-" + name[4:6] + '-' + name[2:4] + ' ' + lineTemp[0] +','+lineTemp[2]+'\n')
					#fout.write(line)

					lineTempLast = lineTemp1	
					
			f.close() 
		fout.close()

wrap_files()				



def read_multiple_files():
	read_files = glob.glob('**/*.txt')
	seq = []
	for file in read_files:
		data = pd.read_csv(file, sep= ';', engine = 'python', error_bad_lines=False, quoting=csv.QUOTE_NONE)
		dataTemp = data
		data = data.iloc[:, 2]
		data = data.apply(pd.to_numeric, errors='ignore')
		data = data.values
		data = data[::2]
		
		for i in range(len(data) - windowLen):
			if i%120 == 0:
				seq.append(data[i:(i+windowLen)])
				return np.asarray(seq)

