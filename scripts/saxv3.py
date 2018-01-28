import numpy as np
from scipy.stats import norm
import math
import ssconfig as conf

class SAXV3:
	def __init__(self,window_length, word_length, alphabet_size):
		self.window_length = window_length
		self.word_length = word_length
		self.alphabet_size = alphabet_size

		self.y_break_points = np.array(norm.ppf(np.linspace(0,1,num=alphabet_size + 1)))
		# self.y_break_points = np.array([-float('Inf'),-1.07, -0.57, -0.18, 0.18, 0.57, 1.07, float('Inf')])


	def toSAXChar(self, val):
		return np.sum(self.y_break_points <= val) - 1

	# EXPERIMENTING SAX
	def timeseries2wcsax(self,timeseries):

		# Z normalize it.
		mean_ts = np.mean(timeseries)
		std_ts = np.std(timeseries, ddof=0)
		normalized_ts = (timeseries - mean_ts)/std_ts

		sax_arr = []
		for i in range(0,len(normalized_ts) - (self.window_length - 1)):
			# Remove the current subsection.
			sax_arr.append(self.toSAXChar(np.mean(normalized_ts[i:(i + self.window_length)])))

		return sax_arr

	# DEFAULT SAX
	def timeseries2sax(self,timeseries):

		# Z normalize it.
		# mean_ss = np.mean(timeseries)
		# std_ss = np.std(timeseries, ddof=1)
		# timeseries = (timeseries - mean_ss)/std_ss

		sax_data = [[-1]]
		self.pos_data = []

		sum_section = 0.0
		sumsq_section = 0.0
		# Scan across the time series to extract sub sequences, and converting them to strings.
		for i in range(0,len(timeseries) - (self.window_length - 1)):
			# Retrieve the current subsection.
			sub_section = timeseries[i:(i + self.window_length)]

			# Z normalize it.
			# mean_ss = np.mean(sub_section)
			# std_ss = np.std(sub_section, ddof=1)

			if i == 0:
				sum_section = np.sum(sub_section)
				sumsq_section = np.sum(np.power(sub_section,2))
			else:
				sum_section = sum_section - timeseries[i-1] + sub_section[-1]
				sumsq_section = sumsq_section - timeseries[i-1]**2 + sub_section[-1]**2
			mean_ss = sum_section/self.window_length

			var = sumsq_section/self.window_length - mean_ss**2
			if var < 0:  # floating point error
				std_ss = 0.0
			else:
				std_ss = math.sqrt(var)

			if std_ss > 0:
				normalized_ss = (sub_section - mean_ss)/std_ss
			else:  # all values are equal
				normalized_ss = sub_section - mean_ss

			# take care of the special case where there is no dimensionality reduction
			if self.window_length % self.word_length == 0:
				split_enss = normalized_ss.reshape(self.word_length, self.window_length/self.word_length)
			else:
				split_enss = np.repeat(normalized_ss,self.word_length).reshape((self.word_length,self.window_length))

			PAA = np.mean(split_enss,axis=1)
			sax_ss = np.sum(np.reshape(np.repeat(PAA,self.alphabet_size + 1),(self.word_length, self.alphabet_size + 1)) >= self.y_break_points, axis= 1) - 1

			if not(conf.sax_remove_repeated_words and (sax_ss == sax_data[-1]).all()):
			# if not(conf.sax_remove_repeated_words and (sax_ss == sax_data[-1])):
				sax_data.append(sax_ss)
				self.pos_data.append(i)

		# Delete the first element, it was just used to initialize the data structure
		sax_data.pop(0)



		return sax_data


	def timeseries2saxseq(self,timeseries):
		if conf.sax_rep == conf.const_ORG_CHAR_BASED:  # character-based SAX (original)
			self.window_length = len(timeseries)
			sax = self.timeseries2sax(timeseries)[0]
			return ' '.join(map(str,sax))
		elif conf.sax_rep == conf.const_WORD_BASED:  # word-based SAX
			sax = self.timeseries2sax(timeseries)
			# sax_str = ['.'.join(map(str,ss)) for ss in sax]
			chars = ['a','b','c','d','e','f','g','h','i']
			to_chars_sax = [[chars[s] for s in ss] for ss in sax]
			sax_str = [''.join(ss) for ss in to_chars_sax]
			return ' '.join(sax_str)
		elif conf.sax_rep == 2:  # hybrid SAX
			sax = self.timeseries2sax(timeseries)
			sax_str = [' '.join(map(str,ss)) for ss in sax]
			return ' '.join(sax_str)
		elif conf.sax_rep == 3:  # hybrid SAX with recalculated window length
			self.window_length = int(len(timeseries) - (np.ceil(len(timeseries)*1.0/self.word_length) - 1))
			sax = self.timeseries2sax(timeseries)
			sax_str = [' '.join(map(str,ss)) for ss in sax]
			return ' '.join(sax_str)
		elif conf.sax_rep == conf.const_SAX_V3:  # character-based SAX by sliding window
			return ' '.join(map(str, self.timeseries2wcsax(timeseries)))

	def timeseries2multisaxseq(self,timeseries):
		sax = self.timeseries2sax(timeseries)
		return [' '.join(map(str,ss)) for ss in sax]

	def timeseriesdata2sax(self,ts_data):
		sax_str_arr = []
		for ts in ts_data:
			sax_str_arr.append(' '.join(map(str, self.timeseries2wcsax(ts))))
		return sax_str_arr

	def getPosData(self):
		return self.pos_data


# for a in range(2,24):
# 	breaks = np.array(norm.ppf(np.linspace(0,1,num=a + 1)))
# 	print '{', ', '.join(map(str,breaks[1:a])), '};'