# System Configuration

Linux PC with Intel Core i7-4790 Processor (Quad Core HT, 3.60GHz), 16GB 1600 MHz memory and 256 Gb SSD storage.

# Description 

## ICDE17
File: icde17.txt

Names of the datasets we tested for our ICDE17 paper. 

## SAX-SEQL

Files: sax-vfseql.csv, sax-vseql.csv

Dataset: name of the dataset.

Error: Classification error by SAX-VFSEQL/SAX-VSEQL.

SAX_time: Time (in seconds) to convert numeric time series (both training and test set) to SAX representation.

TotalLearn_time: Total time (in seconds) to train SEQL models (more than 1 model if the dataset has more than 2 classes) from the SAX representation. 

MaxLearn_time: Time (in seconds) to train a SEQL model from SAX representation. For 2-class dataset, it equals to TotalLearn_time. For n-class dataset (n > 2), it is the longest time measured for each one-vs-all training iteration.

TotalTest_time: Total time (in seconds) to test SEQL models (more than 1 model if the dataset has more than 2 classes) on the test set.

MaxTest_time: Time (in seconds) to test a SEQL model on the test set. For 2-class dataset, it equals to TotalTest_time. For n-class dataset (n > 2), it is the longest time measured for each one-vs-all testing iteration.

## BOSS VS

File: bossvs.csv

Dataset: name of the dataset.

Error: Classification error.

Runtime: Total runtime (SFA transformation + learn + test) in seconds.