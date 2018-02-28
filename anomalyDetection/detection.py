import copy
import csv
import json
import os

from nupic.data.file_record_stream import FileRecordStream
from nupic.engine import Network
from nupic.encoders import MultiEncoder, ScalarEncoder, DateEncoder
from nupic.algorithms import anomaly_likelihood
from nupic.regions.sp_region import SPRegion
from nupic.regions.tm_region import TMRegion

_VERBOSITY = 0
_SEED = 1956
_INPUT_FILE_PATH = "input_file4.csv"
_OUTPUT_PATH = "bathroom_anomaly_detection.csv"
_NUM_RECORDS = 86400


SP_PARAMS = {
    "spVerbosity": _VERBOSITY,
    "spatialImp": "cpp",
    "globalInhibition": 1,
    "columnCount": 2048,
    "inputWidth": 0,
    "numActiveColumnsPerInhArea": 40,
    "seed": 1956,
    "potentialPct": 0.8,
    "synPermConnected": 0.1,
    "synPermActiveInc": 0.0001,
    "synPermInactiveDec": 0.0005,
    "boostStrength": 0.0,
} 

TM_PARAMS = {
    "verbosity": _VERBOSITY,
    "columnCount": 2048,
    "cellsPerColumn": 32,
    "inputWidth": 2048,
    "seed": 1960,
    "temporalImp": "cpp",
    "newSynapseCount": 20,
    "maxSynapsesPerSegment": 32,
    "maxSegmentsPerCell": 128,
    "initialPerm": 0.21,
    "permanenceInc": 0.1,
    "permanenceDec": 0.1,
    "globalDecay": 0.0,
    "maxAge": 0,
    "minThreshold": 9,
    "activationThreshold": 12,
    "outputType": "normal",
    "pamLength": 3,
}    




def createEncoder():
    consumptionEncoder = ScalarEncoder(21, 0, 1024, n=50, name="consumption")
    timeEncoder = DateEncoder(timeOfDay=(21,9.5), name="timestamp_timeOfDay")
    encoder = MultiEncoder()
    encoder.addEncoder("consumption", consumptionEncoder)
    encoder.addEncoder("timestamp", timeEncoder)
    return encoder


def createAnomalyNetwork(dataSource):

    network = Network()
    
    #sensor region
    network.addRegion("sensor", "py.RecordSensor", json.dumps({"verbosity": _VERBOSITY}))
    
    #encoder setup
    sensorRegion = network.regions["sensor"].getSelf()
    sensorRegion.encoder = createEncoder()
    
    sensorRegion.dataSource = dataSource

    #SP width must have sensor output width
    SP_PARAMS["inputWidth"] = sensorRegion.encoder.getWidth()
    
    #Add SP and TM regions
    network.addRegion("SP", "py.SPRegion", json.dumps(SP_PARAMS))
    network.link("sensor", "SP", "UniformLink", "")
    network.link("sensor", "SP", "UniformLink", "",
                 srcOutput="resetOut", destInput="resetIn")
    network.link("SP", "sensor", "UniformLink", "",
                  srcOutput="spatialTopDownOut", destInput="spatialTopDownIn")
    network.link("SP", "sensor","UniformLink", "",
                  srcOutput ="temporalTopDownOut", destInput="temporalTopDownIn")
    network.addRegion("TM", "py.TMRegion", json.dumps(TM_PARAMS))
    network.link("SP", "TM", "UniformLink", "")
    network.link("TM", "SP", "UniformLink", "", srcOutput="topDownOut", destInput="topDownIn")
    
    #Add anomalyLikeliHood
    network.addRegion("ALH", "py.AnomalyLikelihoodRegion", json.dumps({}))
    network.link("TM", "ALH", "UniformLink", "", srcOutput="anomalyScore", destInput="rawAnomalyScore")
    network.link("sensor", "ALH", "UniformLink", "", srcOutput="sourceOut", destInput="metricValue")


    #set layer parameters
    spRegion = network.regions["SP"]
    spRegion.setParameter("learningMode", True)
    spRegion.setParameter("anomalyMode", False)
    
    tmRegion = network.regions["TM"]
    tmRegion.setParameter("topDownMode", True)
    tmRegion.setParameter("learningMode", True)
    tmRegion.setParameter("inferenceMode", True)
    tmRegion.setParameter("anomalyMode", True)
    
    return network


def runNetwork(network, writer):
    sensorRegion = network.regions["sensor"]
    spRegion = network.regions["SP"]
    tmRegion = network.regions["TM"]
    anRegion = network.regions["ALH"]

    prevPredictedColumns = []

    for i in xrange(_NUM_RECORDS):
        network.run(1)

        consumption = sensorRegion.getOutputData("sourceOut")[0]
        anomalyScore = tmRegion.getOutputData("anomalyScore")[0]
        anomalyLikeliHood = anRegion.getOutputData("anomalyLikelihood")[0]
        if anomalyScore != 0.0:
            writer.writerow((i, consumption, anomalyScore))


if __name__ == "__main__":
    dataSource = FileRecordStream(streamID=_INPUT_FILE_PATH)

    network = createAnomalyNetwork(dataSource)
    network.initialize()

    outputPath = os.path.join(os.path.dirname(__file__), _OUTPUT_PATH)

    with open(outputPath, "w") as outputFile:
        writer = csv.writer(outputFile)
        print "Writing output to %s" % outputPath
        runNetwork(network, writer)







