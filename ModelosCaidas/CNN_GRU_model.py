import os
os.environ["PATH"] += os.pathsep + 'C:/Program Files (x86)/Graphviz2.38/bin/'
from keras.utils import plot_model
import numpy
from time import time
from keras.models import Sequential
from keras.layers import Dense, CuDNNLSTM, GRU, Dropout, Activation, TimeDistributed, Flatten
from keras.layers.convolutional import Conv1D, MaxPooling1D
from keras.layers.embeddings import Embedding
from keras.preprocessing import sequence
from keras.models import model_from_json
from keras.callbacks import TensorBoard
from keras import regularizers
from keras.layers.normalization import BatchNormalization
from keras.regularizers import l2
from keras import optimizers
from sklearn import preprocessing
import pandas as pd
import scipy as sp
from scipy import signal
from keras.callbacks import EarlyStopping, ModelCheckpoint
from sklearn.metrics import mean_squared_error
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import confusion_matrix, recall_score, precision_score
from textInBatch import data_sequence


class CONV_GRU_model():

	def __init__(self, model="", dataset_length=1048575,  
				seq_length = 60, l2_lambda = 0.0001, embedding_vector_length = 32, 
				num_epochs = 1000, compile=True):
				
		self.loss = 0
		self.acc = 0
		self.dataset_length = dataset_length
		self.seq_length = seq_length
		self.l2_lambda = l2_lambda
		self.num_epochs = num_epochs
		self.embedding_vector_length = embedding_vector_length
		if model != "":
			self.load(model, compile)
		else:
			self.create_network()
			

	def load(self, file, compile=True):
		try:
			del self.network
		except Exception:
			pass
		json_file = open(file + ".json", "r")	
		loaded_model_json = json_file.read()
		json_file.close()
		self.network = model_from_json(loaded_model_json)
		self.network.load_weights(file + ".h5")
		if compile == True:
			rmsprop = optimizers.RMSprop(lr=0.0001, rho=0.9, epsilon=None, decay=0.0)
			self.network.compile(loss='binary_crossentropy', optimizer=rmsprop, metrics=['accuracy'])
					
	def save(self, file):
		model_json = self.network.to_json()	
		with open(file + ".json", "w") as json_file:
			json_file.write(model_json)
		#self.network.save_weights(file + ".h5")
	
	def create_network(self):
		self.network = Sequential()
		#self.network.add(Embedding(self.dataset_length, self.embedding_vector_length, input_length=self.seq_length))
		# self.network.add(Dense(32, input_shape = (1, 32)))
		# self.network.add(BatchNormalization())
		# self.network.add(Activation('relu'))
		self.network.add(Conv1D(filters=32, input_shape = (80, 32), kernel_size=3, padding='same', kernel_initializer='he_uniform', 
				kernel_regularizer=l2(self.l2_lambda),activation='relu'))
		#self.network.add(MaxPooling1D(pool_size=2))
		#self.network.add(Flatten())	
		self.network.add(Dropout(0.2))
		self.network.add(GRU(100, return_sequences=True))
		self.network.add(Dropout(0.2))
		self.network.add(GRU(50, return_sequences=False))
		#self.network.add(CuDNNGRU(100, return_sequences=False)) 
		#self.network.add(Dropout(0.2))
			
		self.network.add(Dense(1, kernel_initializer="uniform"))
		self.network.add(BatchNormalization())
		self.network.add(Activation('sigmoid'))
		plot_model(self.network, to_file='modelGRU.png', show_shapes=True, show_layer_names=True)
		self.compile()
	
	def compile(self):
		sgd = optimizers.SGD(lr=0.0001, decay=1e-6, momentum=0.9, nesterov=True)
		rmsprop = optimizers.RMSprop(lr=0.0001, rho=0.9, epsilon=None, decay=0.0)
		self.network.compile(loss='binary_crossentropy', optimizer=rmsprop, metrics=['accuracy'])
		print(self.network.summary())
	
	def getData(self, X_train, Y_train, X_val, Y_val, X_test, Y_test):
		self.X_train = X_train
		self.Y_train = Y_train
		self.X_val =  X_val
		self.Y_val =  Y_val
		self.X_test = X_test
		self.Y_test = Y_test
        	
	
	def train(self, file):
		tensorboard = TensorBoard(log_dir="logs/{}".format(time()), histogram_freq=0, batch_size=4, write_graph=True, write_grads=True, 
						write_images=True, embeddings_freq=0, embeddings_layer_names=True, embeddings_metadata=None)
		checkpoint = ModelCheckpoint(file + ".h5", monitor='val_loss', verbose=1, save_best_only=True, mode='min')				
		self.network.fit(self.X_train, self.Y_train, batch_size=4,
                        epochs=self.num_epochs,
                        validation_data=(self.X_val, self.Y_val),
                        verbose=1,
                        callbacks=[EarlyStopping(monitor='val_loss', patience=100), checkpoint, tensorboard])
		
	def evaluate(self):
		scores = self.network.evaluate(self.X_test, self.Y_test, verbose=1)
		y_pred = self.network.predict_classes(self.X_test,verbose=1, batch_size=1)
		precision = precision_score(self.Y_test, y_pred)
		recall = recall_score(self.Y_test, y_pred)
		f1 = 2 * (precision * recall)/(precision + recall) 
		print( 'precision = ', precision, '\n', 'recall = ', recall,'\n', 'F1-score:', f1)
		print("Accuracy: %.2f%%" % (scores[1]*100))

				
		
#read xtrain data
X_train, Y_train = data_sequence('NNData/')
#X_train = signal.medfilt(X_train, 5)
# #read xval data
X_val, Y_val = data_sequence('valData/')
# #read xtest data
X_test, Y_test = data_sequence('testData/') 
# scaler, train_scaled, val_scaled, test_scaled = scale(X_train, X_val, X_test)
# train_scaled = train_scaled.reshape(train_scaled.shape[0], 1, train_scaled.shape[1])
# val_scaled = val_scaled.reshape(val_scaled.shape[0], 1, val_scaled.shape[1])
# test_scaled = test_scaled.reshape(test_scaled.shape[0], 1, test_scaled.shape[1])	

model = CONV_GRU_model("conv_gru_model4")
model.getData(X_train, Y_train, X_val, Y_val, X_test, Y_test)
#model.train("conv_gru_model4")
#model.save("conv_gru_model4")
model.evaluate()		

