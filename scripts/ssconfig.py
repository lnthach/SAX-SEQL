import datetime
#
data = "Coffee"
all_dataset = (
	'Coffee',
	'Earthquakes',
	'ECG200',
	'ECGFiveDays',
	'FordA',
	'FordB',
	'Gun_Point',
	# 'ItalyPowerDemand',
	'Lighting2',
	'MoteStrain',
	'Passgraph',
	'SonyAIBORobotSurface',
	'SonyAIBORobotSurfaceII',
	'TwoLeadECG',
	'wafer',
	'yoga',
	'Cricket',
	'Ham',
	'Herring',
	'ShapeletSim',
	'Wine',
	'HandOutlines',
)

lightweight_dataset = (
	'Coffee',
	'Cricket',
	'ECG200',
	'ECGFiveDays',
	'Gun_Point',
	'ItalyPowerDemand',
	'SonyAIBORobotSurface',
	'SonyAIBORobotSurfaceII',
	'TwoLeadECG',
	'MoteStrain',
)

tslength_dict = {
	'ECG200': 96,
	'Gun_Point': 150,
	'Lighting2': 637,
	'wafer': 152,
	'Coffee': 286,
	'yoga': 426,
	'Herring': 512,
	'Wine': 234,
	'ShapeletSim': 500,
	'Ham': 421,
	'FordA': 500,
	'FordB': 500,
	'Earthquakes': 512,
	'ECGFiveDays': 136,
	'ItalyPowerDemand': 24,
	'MoteStrain': 84,
	'SonyAIBORobotSurface': 70,
	'SonyAIBORobotSurfaceII': 65,
	'TwoLeadECG': 82,
	'HandOutlines': 2709,
	'Cricket': 308,
	'Passgraph': 364,
}

rootdatapath = 'UCR_TS_Archive_2015/'

def getTrainingDataPath():
	return rootdatapath + data + '/' + data + '_TRAIN'
def getTestDataPath():
	return rootdatapath + data + '/' + data + '_TEST'

#------------------------------
# SAX ALGORITHM
const_SAX = 0
const_1dSAX = 1
const_Multi_SAX = 3
# 0 - SAX
# 1 - 1d-SAX
# 2 - t-SAX (developing)
sax = const_SAX
#------------------------------

sax_remove_repeated_words = 1

#------------------------------
# TYPES OF SAX REPRESENTATION
const_ORG_CHAR_BASED = 0
const_WORD_BASED = 1
const_SAX_V3 = 4  # char-based SAX by window slicing
# 2: word-char hybrid 1 : converting words back to characters
# 3: word-char hybrid 2 : recomputing window size
sax_rep = const_WORD_BASED
#------------------------------

# SEQL CONFIGURATION
seql_minpat = 1
seql_maxpat = 0
seql_gap = 0

def seql_gram_config():
	if (seql_maxpat):
		return "-l " + str(seql_minpat) + ' -L ' + str(seql_maxpat) + ' -g ' + str(seql_gap)
	else:
		return "-l " + str(seql_minpat) + ' -g ' + str(seql_gap)


def seql_minmaxpat():
	return str(seql_minpat) + '_' + str(seql_maxpat) + 'pat'
#seql_gram_choices = ['ngr'] + [str(x) + 'gr' for x in range(1,100)]





seql_tune_threshold = 0
seql_tunethreshold_choices = ('nothre','tuthres')

def printSEQLConfig(writer):
	writer.write("---------- SEQL CONFIGURATION ---------\n")

	if (seql_minpat):
		writer.write("MIN LENGTH OF FEATURES: " + str(seql_minpat) + '\n')


	if (seql_maxpat):
		writer.write("MAX LENGTH OF FEATURES: " + str(seql_maxpat) + '\n')
	else:
		writer.write("MAX LENGTH OF FEATURES: UNLIMITED" + '\n')

	if (seql_tune_threshold):
		writer.write("THRESHOLD TUNING: 1" + '\n')
	else:
		writer.write("THRESHOLD TUNING: 0" + '\n')

	print("---------------------------------------\n")


# SINGLE-RUN CONFIGURATION
sax_config = {
	'N': 50,
	'n': 10,
	'alphabet_size': 4,
	'ns': 2,
	'na': 2,
	'wstep': 1.
}

def setSAXConfig(N = 0, n = 0, alphabet_size = 0, ns = 0, na = 0):
	if N:
		sax_config['N'] = N
	if n:
		sax_config['n'] = n
	if alphabet_size:
		sax_config['alphabet_size'] = alphabet_size
	if ns:
		sax_config['ns'] = ns
	if na:
		sax_config['na'] = na

def getSAXConfigStr():
	if sax_rep == 1 or sax_rep == 2:
		return ','.join([str(sax_config['N']), str(sax_config['n']), str(sax_config['alphabet_size'])])
	elif sax_rep == 0 or sax_rep == 3:
		return ','.join([str(sax_config['n']), str(sax_config['alphabet_size'])])
	if sax_rep == const_SAX_V3:
		return ','.join([str(sax_config['N']), str(sax_config['alphabet_size'])])
	return 'NA,NA,NA'


# CROSS VALIDATION CONFIGURATION

crv_k = 5
crv_reshuffle = 1


# DIRECT OPTIMIZATION CONFIGURATION
dro_algmethod=0
dro_maxT=100
dro_maxf = 500

seql_performance_metric = 0
seql_metric_choices = ['err','berr','lglos','scloss']

def printSAXSEQLOptConfig(writer):
	writer.write('CV Folds: ' + str(crv_k) + '\n')
	writer.write('Re-Shuffle: ' + str(crv_reshuffle) + '\n')
	if (seql_performance_metric == 0):
		writer.write("Metric: Total Error Rate\n")
	if (seql_performance_metric == 1):
		writer.write("Metric: Balanced Error\n")


runStamp = datetime.datetime.now().strftime('%Y%m%d%H%M%S')

def getSEQLDir():
	return data + '/' + runStamp

def getFileName(data):
	return '.'.join([data
						,seql_minmaxpat()
						,seql_tunethreshold_choices[seql_tune_threshold]
						,seql_metric_choices[seql_performance_metric]
						,runStamp])

# DONT USE THIS !!!
def setCharBasedSAXConfig(dataset, sax_length, alphabet_size):
	data = dataset
	sax_config['n'] = sax_length
	sax_config['alphabet_size'] = alphabet_size
