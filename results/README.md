## Description

Dataset: name of the dataset.
Error: Classification error by SAX-VFSEQL.
SAX_time: Time (in seconds) to convert numeric time series (both training and test set) to SAX representation.
TotalLearn_time: Total time (in seconds) to train SEQL models (more than 1 if the dataset has more than 2 classes) from the SAX representation. 
MaxLearn_time: Time (in seconds) to train a SEQL model from SAX representation. For 2-class dataset, it equals to TotalLearn_time. For n-class dataset (n > 2), it is the longest time measured for each one-vs-all training iteration.
TotalTest_time: Total time (in seconds) to test SEQL models (more than 1 if the dataset has more than 2 classes) on the test set.
MaxTest_time: Time (in seconds) to test a SEQL model on the test set. For 2-class dataset, it equals to TotalTest_time. For n-class dataset (n > 2), it is the longest time measured for each one-vs-all testing iteration.

