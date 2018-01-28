'''
    File name: map_features.py
    Author: Le Nguyen Thach    
    Python Version: 3.6
	
	This is a python module to visualize SAX sequence on raw time series. 
'''


import re
import saxv3
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np
import math
from scipy.stats import norm
import random as rd
import matplotlib
from numpy import genfromtxt

# search for pattern in sequence
# allow mismatch within limit defined by max_dist (i.e. fuzzy matching)
def fuzzysearch(pattern,subsequence):
	#print pattern
	#print subsequence
	max_dist = 1
	for i in range(0,len(subsequence) - len(pattern) + 1):
		current_dist = 0
		found = 1
		for j in range(i,i+len(pattern)):
			current_dist += abs(ord(subsequence[j]) - ord(pattern[j-i]))
			if current_dist > max_dist:
				found = 0
				break
		if found:
			return i
	return -1

# visualize SAX pattern on raw time series
# timeseries: numeric vector of raw time series
# N: window size
# n: word length
# alphabet_size: alphabet size
# pattern: a SAX sequence

def plotSequence(timeseries,N,n,alphabet_size,pattern):
	cl1 = 'g--'
	cl2 = 'r-'
	saxConverter = saxv3.SAXV3(N,n,alphabet_size)
	sd = saxConverter.timeseries2saxseq(timeseries).split(' ')
	pd = saxConverter.getPosData()
	t_full = range(0, len(timeseries))
	plt.plot(t_full, timeseries, cl1)
	#print sd
	for seg,cpos in zip(sd,pd):
		rel_pos = fuzzysearch(pattern,seg)
		if rel_pos >= 0:
			pt_start = int(cpos + rel_pos * N * 1.0 / n)
			pt_end = int(cpos + rel_pos * N * 1.0 / n + N * len(pattern) * 1.0 / n)			
			plt.plot(range(pt_start,pt_end+1), timeseries[pt_start:(pt_end + 1)], cl2,lw=5.0)			
			#break 
	plt.show()

# datapath: path to the raw time series
# N: window size
# n: word length
# alphabet_size: alphabet size
# patterns: list of symbolic sequences 
# positive: a list of the same size of patterns contains only 1 and 0 indicate whether corresponding sequence belong to positive class (1) or negative class (0)
def plotFVSEQL(datapath,N,n,alphabet_size,patterns,positive):
	#config sax output
	#sax.getSAXPosition = True
	#freader=csv.reader(open(data,"rb"),delimiter=',')
	#data=np.array(list(freader)).astype('float')

	data = genfromtxt(datapath, delimiter=' ')



	# change class label to 1 and -1
	meanclass = np.mean(np.unique(data[:,0]))
	data[:,0] = [1 if x > meanclass else -1 for x in data[:,0]]

	# posGP = sax.normalizeMat(data[data[:,0] == 1,1:])
	# negGP = sax.normalizeMat(data[data[:,0] == -1,1:])
	posGP = data[data[:,0] == 1,1:]
	negGP = data[data[:,0] == -1,1:]



	# alphabet = [str(x).rjust(len(str(alphabet_size - 1)),'0') for x in range(0,alphabet_size)]
	saxConverter = saxv3.SAXV3(N,n,alphabet_size)


	#subseq = SAXtoVal(pattern,alphabet,N/n)
	# for rc in plotdata:
	# 	t_full = range(0, len(rc))
	# 	plt.plot(t_full, rc, cl1)
	plt.figure(1)
	for i in range(0,len(patterns)):
		pattern = patterns[i]
		plt.subplot(1,len(patterns),i+1)

		if positive[i]:
			plotdata = posGP
			cl1 = 'b--'
			cl2 = 'r-'
		else:
			plotdata = negGP
			cl1 = 'g--'
			cl2 = 'r-'

		for rc in plotdata:
			# t_full = range(0, len(rc))
			# plt.plot(t_full, rc, cl1)
			# convert time series to sax. function return sax data (sd) and its position (pd)
			found = 0;
			sd = saxConverter.timeseries2saxseq(rc).split(' ')
			pd = saxConverter.getPosData()
			#print sd
			for seg,cpos in zip(sd,pd):
				#try:
					#rel_pos = seg.index(pattern)
				rel_pos = fuzzysearch(pattern,seg)
				if rel_pos >= 0:
					pt_start = int(cpos + rel_pos * N * 1.0 / n)
					pt_end = int(cpos + rel_pos * N * 1.0 / n + N * len(pattern) * 1.0 / n)
					t_full = range(0, len(rc))
					plt.plot(t_full, rc, cl1)
					plt.plot(range(pt_start,pt_end+1), rc[pt_start:(pt_end + 1)], cl2,lw=5.0)
					found = 1
					break
				#except ValueError:
				#	print "not found"
				#else:
				#	print "found"
			if found:
				break
	plt.show()

#plotFVSEQL("Resp_TRAIN",57,16,4,["bcdccdd","bcdccdd","baaaacdcddc","baabacdddd"],[1,1,0,0])
#plotFVSEQL("UCR_TS_Archive_2015/Gun_Point/Gun_Point_TRAIN",30,16,4,["dcaaabb","dcabaaaaaaa","aabbbbbbbccd","bbbbbbbcbaa"],[1,1,0,0])
#plotFVSEQL("UCR_TS_Archive_2015/Gun_Point/Gun_Point_TRAIN",30,16,4,["dcaaabb","aabbbbbbbccd"],[1,0])
#plotFVSEQL("../dataset/UCR_TS_Archive_2015/Coffee/Coffee_TRAIN",20,16,4,["ccccddaaaabc","aaabbbbbbcc"],[1,0])

tss = [float(x) for x in open("Resp_TRAIN").readlines()[0][1:].split()]
plotSequence(tss,60,16,4,'abcabc')

