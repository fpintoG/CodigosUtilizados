import glob
from random import shuffle
import pandas as pd
from sklearn import preprocessing
from sklearn.preprocessing import MinMaxScaler
import numpy as np
from keras.preprocessing import sequence

#data file by file			
def data_generator(path):
	read_files = glob.glob(path + "*.txt")
	shuffle(read_files)
	#print(len(read_files))
	while 1:
		for f in read_files:
			data = pd.read_csv(f, sep='\*| ', engine='python')
			data = data.dropna(axis=1, how='all')
			data = data.drop(data.columns[[0, 1]], axis=1) 
			data = data.apply(pd.to_numeric, errors='ignore')
			data = data.values
			dataX = data[:,0:-1]
			dataY = data[[2], [-1]]
			dataX = dataX.reshape(1, dataX.shape[0], dataX.shape[1])
			yield dataX, dataY
			




def data_sequence(path):
	read_files = glob.glob(path + "*.txt")
	shuffle(read_files)
	#print(len(read_files))
	seqX = []
	seqY = []
	for f in read_files:
		data = pd.read_csv(f, sep='\*| ', engine='python')
		data = data.dropna(axis=1, how='all')
		data = data.drop(data.columns[[0, 1]], axis=1) 
		data = data.apply(pd.to_numeric, errors='ignore')
		data = data.values
		dataX = data[:,0:-1]
		dataY = data[[2], [-1]]
		#dataX = dataX.reshape(1, dataX.shape[0], dataX.shape[1])
		seqX.append(dataX)
		seqY.append(dataY)
	x = sequence.pad_sequences(seqX, maxlen=80, dtype='float32')
	y = np.asarray(seqY)
	return x, y		

def process_sequences(input_sequences, output_sequences, depth=None):
    """
    Expects lists of lists of floats
    Returns 3d int array (batch size, depth, [0=character, 1=mask])
    mask values: 1-input, 2-training output, 3-testing output, 0-padding
    :param sequences:
    :return:
    """
    assert (len(input_sequences) == len(output_sequences))
    if not depth:
        depth = max(len(i) + len(o) + 4 for i, o in zip(input_sequences, output_sequences))
    n = len(input_sequences)
    x = np.zeros((n, depth, 2), dtype=np.float32)
    for i, (iseq, oseq) in enumerate(zip(input_sequences, output_sequences)):
        idx = 0
        for c in iseq:
            x[i, idx, 0] = c + 1
            x[i, idx, 1] = 1
            idx += 1
        x[i, idx, 1] = 1
        idx += 1
        x[i, idx, 1] = 2
        idx += 1
        for c in oseq:
            x[i, idx, 0] = c + 1
            x[i, idx, 1] = 2
            idx += 1
    y = np.concatenate((x[:, 1:, 0], np.zeros((x.shape[0], 1), dtype=np.float32)), axis=1)
    x = np.concatenate((x, np.random.random((x.shape[0], x.shape[1], 1)).astype(np.float32)), axis=2)
    return x, y


def scale(train, val, test):
	# fit scaler
	scaler = MinMaxScaler(feature_range=(0, 1))
	scaler = scaler.fit(train)
	# transform train
	train = train.reshape(train.shape[0], train.shape[1])
	train_scaled = scaler.transform(train)
	# transform train
	val = val.reshape(val.shape[0], val.shape[1])
	val_scaled = scaler.transform(val)
	# transform test
	test = test.reshape(test.shape[0], test.shape[1])
	test_scaled = scaler.transform(test)
	return scaler, train_scaled, val_scaled, test_scaled
	
#x, y = data_sequence('testData/')
#print(np.count_nonzero(y))  
