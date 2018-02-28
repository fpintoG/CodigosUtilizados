from keras.models import model_from_json
#from textInBatch import data_sequence
from keras import optimizers



def load_model(file):
    json_file = open(file + ".json", "r")
    loaded_model_json = json_file.read()
    print('loaded model')
    json_file.close()
    model = model_from_json(loaded_model_json)
    model.load_weights(file + ".h5")
    print('loaded weights')
    return model


