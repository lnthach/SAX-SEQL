/*
 * sax_seql_exec.cpp
 *	For binary classification only.
 *  Created on: 9 May 2017
 *      Author: thachln
 */



#include "seql_learn.h"
#include "sax_converter.h"
#include "seql_mkmodel.h"
#include "seql_classify.h"

#include <cmath>
#include <stdlib.h>
#include <time.h>

#define DELIMITER ' '

using namespace std;

class SAXSEQL {

private:



	int window_size;
	int word_length;
	int alphabet_size;

	string train_data;
	string test_data;
	string work_dir;
	string sax_train;
	string sax_test;
	string model;
	string model_bin;
	string model_predictors;

	int train_size;
	int test_size;

	std::vector < std::string > train_sequences;
	std::vector < double > train_labels;

	std::vector < std::string > test_sequences;
	std::vector < double > test_labels;

	vector<double> label_set;

	bool token_type;
	double max_distance;

	// return size of data by counting number of lines
	void learn(vector<string>& sequences, vector<double>& _labels){
		SeqLearner seql_learner;
		unsigned int objective = 0;
		unsigned int maxpat = 0xffffffff;
		unsigned int minpat = 1;
		unsigned int maxitr = 5000;
		unsigned int minsup = 1;

		// Max # of total wildcards allowed in a feature.
		unsigned int maxgap = 0;
		// Max # of consec wildcards allowed in a feature.
		unsigned int maxcongap = 0;
		// Word or character token type. By default char token.
		//bool token_type = 0;
		// BFS vs DFS traversal. By default BFS.
		bool traversal_strategy = 0;

		// The C regularizer parameter in regularized loss formulation. It constraints the weights of features.
		// C = 0 no constraints (standard SLR), the larger the C, the more the weights are shrinked towards each other (using L2) or towards 0 (using L1)
		double C = 1;
		// The alpha parameter decides weight on L1 vs L2 regularizer: alpha * L1 + (1 - alpha) * L2. By default we use an L2 regularizer.
		double alpha = 0.2;

		double convergence_threshold = 0.005;
		int verbosity = 0;
		//seql_learner.clear_skip_list();
		//for (unsigned int i2 = 0; i2 < skips.size();i2++){
		//	seql_learner.add_skips_items(skips[i2] + 1); // first item index is 1
		//}
		//		for (int i = 0; i < _labels.size();i++){
		//			cout << _labels[i] << " ";
		//		}
		seql_learner.external_read(sequences,_labels);


		seql_learner.run (sax_train.c_str(), model.c_str(), alphabet_size, max_distance, objective, maxpat, minpat, maxitr,
				minsup, maxgap, maxcongap, token_type, traversal_strategy, convergence_threshold, C, alpha, verbosity);
	}



	double tune_threshold(vector<string> &sequences, vector<double> &labels){
		unsigned int verbose = 0;
		double threshold = 0; // By default zero threshold = zero bias.
		vector<pair<double,double>> scores;



		SEQLClassifier seql;

		if (verbose >= 3) seql.setRule (true);
		//cout << model_bin << "****" << endl;
		if (! seql.open (model_bin.c_str(), threshold)) {
			std::cerr << " " << model_bin << " No such file or directory" << std::endl;
			return 1.0;
		}

		// Predicted and true scores for all docs.
		//vector<pair<double, int> > scores;


		unsigned int correct = 0;
		// Total number of true positives.
		unsigned int num_positives = 0;


		seql.load_mytrie(model_predictors.c_str(), threshold);

		for (unsigned int item = 0; item < sequences.size();++item){
			double predicted_score = seql.classify_with_mytrie(sequences[item].c_str(), max_distance);
			scores.push_back(pair<double, int>(predicted_score, labels[item]));
			if (labels[item] == 1){
				num_positives++;
			}
		}

		// Sort the scores ascendingly by the predicted score.
		sort(scores.begin(), scores.end());

		unsigned int TP = num_positives;
		unsigned int FP = sequences.size() - num_positives;
		unsigned int FN = 0;
		unsigned int TN = 0;

		unsigned int min_error = FP + FN;
		unsigned int current_error = 0;
		double best_threshold = -numeric_limits<double>::max();

		for (unsigned int i = 0; i < sequences.size(); ++i) {
			// Take only 1st in a string of equal values
			if (i != 0 && scores[i].first > scores[i-1].first) {
				current_error = FP + FN; // sum of errors, e.g # training errors
				if (current_error < min_error) {
					min_error = current_error;
					best_threshold = (scores[i-1].first + scores[i].first) / 2;
					//cout << "\nThreshold: " << best_threshold;
					//cout << "\n# errors (FP + FN): " << min_error;
					//std::printf ("\nAccuracy: %.5f%% (%d/%d)\n", 100.0 * (TP + TN) / all, TP + TN, all);
				}
			}
			if (scores[i].second == 1) {
				FN++; TP--;
			}else{
				FP--; TN++;
			}
		}

		return best_threshold;

	}

	double classify(vector<string> &test_samples, vector<double> &test_true_labels, double threshold){
		unsigned int verbose = 0;


		SEQLClassifier seql;

		if (verbose >= 3) seql.setRule (true);
		//cout << model_bin << "****" << endl;
		if (! seql.open (model_bin.c_str(), threshold)) {
			std::cerr << " " << model_bin << " No such file or directory" << std::endl;
			return 1.0;
		}

		// Predicted and true scores for all docs.
		//vector<pair<double, int> > scores;

		unsigned int all = 0;
		unsigned int correct = 0;

		seql.load_mytrie(model_predictors.c_str(), threshold);

		for (unsigned int item = 0; item < test_samples.size(); ++item){
			//int item = test_fold[ic];
			int y = int (test_true_labels[item]);
			//double predicted_score = seql.classify (sequences[item].c_str(), token_type);
			double predicted_score = seql.classify_with_mytrie(test_samples[item].c_str(), max_distance);

			double predicted_prob;
			if (predicted_score < -8000) {
				predicted_prob = 0;
			} else {
				predicted_prob = 1.0 / (1.0 + exp(-predicted_score));
			}

			if (verbose == 1) {
				std::cout << y << " " << predicted_score << " " << predicted_prob << std::endl;
			} else if (verbose == 2) {
				std::cout << y << " " << predicted_score << " " << predicted_prob <<  " " << test_samples[item] << std::endl;
			} else if (verbose == 4) {
				std::cout << "<instance>" << std::endl;
				std::cout << y << " " << predicted_score << " " << predicted_prob << " " << test_samples[item] << std::endl;
				seql.printRules (std::cout);
				std::cout << "</instance>" << std::endl;
			} else if (verbose == 5) {
				std::cout << y << " ";
				seql.printIds (std::cout);
			}

			all++;
			if (predicted_score > 0) {
				if(y > 0) correct++;
				//if(y > 0) res_a++; else res_b++;
			} else {
				if(y < 0) correct++;
				//if(y > 0) res_c++; else res_d++;
			}

		}


		double error = 1.0 - 1.0*correct / all;

		if(std::isnan(error)){
			cout << "FATAL ERROR: NAN" << endl;
			cout << "correct/all=" << correct << "/" << all << endl;

		}
		return error;
	}


public:
	vector<int> stratified_k_folds(int sample_size,int k){
		vector<int> folds;
		int fold_capacities[k];
		int remaining = sample_size % k;
		int base_cap = sample_size / k;

		//calculate size of each fold
		for (int i = 0; i < k;i++){
			fold_capacities[i] = base_cap;
			if (remaining > 0){
				fold_capacities[i]++;
				remaining--;
			}
		}

		// assign each item to only available slots
		// in the beginning there are k available slots
		int remaining_slots = k;
		for (unsigned int i = 0; i < sample_size; i++){
			// choose slot
			int slot = rand()%remaining_slots;

			for (int j = 0; j < k; j++){
				if (slot > 0) {
					if (fold_capacities[j] > 0){
						slot--;
					}

				} else {
					if (fold_capacities[j] > 0){
						fold_capacities[j]--;
						folds.push_back(j);
						if (fold_capacities[j] == 0){
							remaining_slots--;
						}
						break;
					}
				}
			}

		}
		return folds;

	}
	// transform
	bool convert_numeric_data_to_sax(string input_data,string output_sax, vector<string> &sequences, vector<double> &labels){
		SAX sax_converter(window_size,word_length,alphabet_size,2);

		string del = " ";

		std::ifstream myfile (input_data);
		std::ofstream outfile (output_sax);



		string line;
		size_t pos = 0;
		double label;
		string sax_str;
		bool new_label;

		if (myfile.is_open() && outfile.is_open()){
			while (getline (myfile, line)){
				pos = line.find(del);
				label = atof(line.substr(0, pos).c_str());
				line.erase(0, pos + del.length());
				sax_str = sax_converter.timeseries2SAX(line,del);
				outfile << label << " " << sax_str << '\n';
				sequences.push_back(string(sax_str));
				labels.push_back(label);
				new_label = true;
				for (double l:label_set){
					if (l == label){
						new_label = false;
					}
				}
				if (new_label){
					label_set.push_back(label);
				}

			}
			myfile.close();
			outfile.close();
		} else {
			if (!myfile.is_open())	std::cout << "Invalid Timeseries Data Input." << std::endl;
			if (!outfile.is_open())	std::cout << "Invalid output for SAX." << std::endl;
			return false;
		}


		return true;
	}



	SAXSEQL(string _train_data, string _test_data, string _work_dir, int sax_N, int sax_w, int sax_a){

		work_dir = _work_dir;
		train_data = _train_data;
		test_data = _test_data;

		window_size = sax_N;
		word_length = sax_w;
		alphabet_size = sax_a;

		sax_train = work_dir + "/sax.train";
		sax_test = work_dir + "/sax.test";
		model = work_dir + "/seql.model";
		model_bin = work_dir + "/seql.model.bin";
		model_predictors = work_dir + "/seql.model.predictors";

		//data_size = compute_data_size();
		token_type = 1;
		max_distance = 1.0;

		clock_t start = clock();
		convert_numeric_data_to_sax(train_data,sax_train,train_sequences,train_labels);
		convert_numeric_data_to_sax(test_data,sax_test,test_sequences,test_labels);
		cout << "SAX conversion time: " << double(clock() - start) / CLOCKS_PER_SEC << endl;

		train_size = train_labels.size();
		test_size = test_labels.size();


		cout << "Train: " << train_data << endl;
		cout << "Train size: " << train_size << endl;
		cout << "Test: " << test_data << endl;
		cout << "Test size: " << test_size << endl;
		cout << "Number of classes: " << label_set.size() << endl;
		//cout << train_sequences[0] << endl;
		//cout << test_sequences[0] << endl;
		// READ DATA
		//		char *columns[5];
		//		string doc;

		//vector<string> sax;
		//		std::string test_line;
		//		std::istream *test_is = new std::ifstream (sax_data);
		//		while (std::getline (*test_is, test_line)) {
		//			//sax.push_back(test_line);
		//
		//
		//			char * line = new char[test_line.size() + 1];
		//			std::copy(test_line.begin(), test_line.end(), line);
		//			line[test_line.size()] = '\0'; // don't forget the terminating 0
		//
		//			if (2 != tokenize (line, "\t ", columns, 2)) {
		//				std::cerr << "FATAL: Format Error: " << line << std::endl;
		//
		//			}
		//			// Prepare class. _y is +1/-1.
		//			double _y = atof (columns[0]);
		//			labels.push_back (_y);
		//			label_set.insert(_y);
		//
		//
		//
		//			// Prepare doc. Assumes column[1] is the original text, e.g. no bracketing of original doc.
		//			doc.assign(columns[1]);
		//			sequences.push_back(doc);
		//		}
		//		delete test_is;

	}

	//	int compute_data_size(){
	//		std::ifstream inFile(input_data);
	//		int count = std::count(std::istreambuf_iterator<char>(inFile),
	//				std::istreambuf_iterator<char>(), '\n');
	//		inFile.close();
	//		return count;
	//	}
	void undersampling(vector<double> & selection, double positive_label){
		int positive_count = 0;
		// add positive sample first
		selection.resize(train_size);
		for (int i = 0; i < train_size; ++i){
			if (train_labels[i] == positive_label){
				selection[i] = 1.0;
				positive_count++;
			} else {
				selection[i] = 0.0;
			}
		}
		// add negative sample
		int negative_count = 0;
		int current_label_index = 0;
		int not_found = 0;
		while (negative_count < positive_count && not_found < label_set.size()){ // in case no negative sample found ,i.e., the current positive class is big majority
			if (label_set[current_label_index] == positive_label){ // skip positive label
				current_label_index++;
			}
			not_found++;
			for (int i = 0; i < train_size; ++i){
				if(train_labels[i] == label_set[current_label_index] && selection[i] == 0){ // not taken
					selection[i] = -1;
					negative_count++;
					not_found = 0;
					break;
				}
			}
			//next label
			current_label_index++;


		}
	}


	// run sax-seql with leave-one-out cross validation on training set
	double run_sax_seql_with_loo(){
		clock_t learn_starttime, classification_starttime;
		double tmp_learn_time, learn_time, tmp_classification_time, classification_time, max_time = 0;



		vector<string> tmp_test_sequences;
		vector<double> tmp_test_labels;
		tmp_test_sequences.resize(1);
		tmp_test_labels.resize(1);

		double total_error = 0.0;

		// CROSS VALIDATION
		if (true){
			for (int i = 0; i < train_size;i++){
				double threshold = 0;
				// pop the first element for test purpose
				tmp_test_sequences.clear();
				tmp_test_labels.clear();
				tmp_test_sequences.push_back(train_sequences.front());
				tmp_test_labels.push_back(train_labels.front());
				train_sequences.erase(train_sequences.begin());
				train_labels.erase(train_labels.begin());

				// ******************************LEARNING PHASE******************************
				learn_starttime = clock();
				learn(train_sequences, train_labels);
				tmp_learn_time = double(clock() - learn_starttime) / CLOCKS_PER_SEC;
				// ******************************MKMODEL PHASE******************************
				if (mkmodel(model,model_bin,model_predictors) < 0){
					cout << "Fail to prepare model file.\n";
				}
				// ******************************TUNE THRESHOLD******************************
				// threshold = tune_threshold(positive_label);
				// ******************************CLASSIFICATION PHASE******************************
				classification_starttime = clock();
				total_error += classify(tmp_test_sequences,tmp_test_labels,threshold);
				tmp_classification_time = double(clock() - classification_starttime) / CLOCKS_PER_SEC;

				// push back the test sample
				train_sequences.push_back(tmp_test_sequences.back());
				train_labels.push_back(tmp_test_labels.back());

				// only keep runtime of the most expensive
				if (tmp_learn_time + tmp_classification_time > max_time){
					learn_time = tmp_learn_time;
					classification_time = tmp_classification_time;
					max_time = tmp_learn_time + tmp_classification_time;
				}

			}
			cout << "Training error (Leave-one-out): " << total_error/ train_size << " (" << total_error << "/" << train_size << ")" << endl;
		}

		// EVALUATE ON TEST DATA
		double threshold = 0;
		// ******************************LEARNING PHASE******************************
		learn(train_sequences, train_labels);
		// ******************************MKMODEL PHASE******************************
		if (mkmodel(model,model_bin,model_predictors) < 0){
			cout << "Fail to prepare model file.\n";		}
		// ******************************TUNE THRESHOLD******************************
		//threshold = tune_threshold(train_sequences, train_labels);
		//cout << "Tuned threshold: " << threshold << endl;
		// ******************************CLASSIFICATION PHASE******************************
		cout << "Test error with default threshold: " << classify(test_sequences,test_labels,0) << endl;
		//cout << "Test error with tuned threshold: " << classify(test_sequences,test_labels,threshold) << endl;


		//cout << "(Longest) learning time: " << learn_time << endl;
		//cout << "(Longest) classifying time: " << classification_time << endl;



	}

	// run sax-seql with k-folds cross validation on training set
	double run_sax_seql_with_k_folds(){
		int k = 10;
		vector<int> folds = stratified_k_folds(train_size,k);
		clock_t learn_starttime, classification_starttime;
		double tmp_learn_time, learn_time, tmp_classification_time, classification_time, max_time = 0;





		double total_error = 0.0;

		// CROSS VALIDATION
		if (true){
			vector<string> tmp_test_sequences;
			vector<double> tmp_test_labels;
			vector<string> tmp_train_sequences;
			vector<double> tmp_train_labels;

			for (int fold = 0; fold < k;fold++){
				double threshold = 0;
				double this_fold_error = 0;
				// pop the first element for test purpose
				tmp_test_sequences.clear();
				tmp_test_labels.clear();
				tmp_train_sequences.clear();
				tmp_train_labels.clear();

				int test_fold_size = 0;
				int train_fold_size = 0;
				for (int i = 0; i < train_size;i++){
					if (folds[i] == fold){
						tmp_test_sequences.push_back(train_sequences[i]);
						tmp_test_labels.push_back(train_labels[i]);
						test_fold_size++;
					} else {
						tmp_train_sequences.push_back(train_sequences[i]);
						tmp_train_labels.push_back(train_labels[i]);
						train_fold_size++;
					}
				}




				// ******************************LEARNING PHASE******************************
				learn_starttime = clock();
				learn(tmp_train_sequences, tmp_train_labels);
				tmp_learn_time = double(clock() - learn_starttime) / CLOCKS_PER_SEC;
				// ******************************MKMODEL PHASE******************************
				if (mkmodel(model,model_bin,model_predictors) < 0){
					cout << "Fail to prepare model file.\n";
				}
				// ******************************TUNE THRESHOLD******************************
				// threshold = tune_threshold(positive_label);
				// ******************************CLASSIFICATION PHASE******************************
				classification_starttime = clock();
				this_fold_error = classify(tmp_test_sequences,tmp_test_labels,threshold);
				tmp_classification_time = double(clock() - classification_starttime) / CLOCKS_PER_SEC;

				// only keep runtime of the most expensive
				if (tmp_learn_time + tmp_classification_time > max_time){
					learn_time = tmp_learn_time;
					classification_time = tmp_classification_time;
					max_time = tmp_learn_time + tmp_classification_time;
				}

				cout << "Training data split: " << train_fold_size << " / " << test_fold_size << " CV Error: " << this_fold_error << endl;
				total_error += this_fold_error;

			}
			cout << "Training error (" << k <<  "-folds): " << total_error/ k << endl;
		}

		// EVALUATE ON TEST DATA
		double threshold = 0;
		// ******************************LEARNING PHASE******************************
		learn(train_sequences, train_labels);
		// ******************************MKMODEL PHASE******************************
		if (mkmodel(model,model_bin,model_predictors) < 0){
			cout << "Fail to prepare model file.\n";		}
		// ******************************TUNE THRESHOLD******************************
		//threshold = tune_threshold(train_sequences, train_labels);
		//cout << "Tuned threshold: " << threshold << endl;
		// ******************************CLASSIFICATION PHASE******************************
		cout << "Test error with default threshold: " << classify(test_sequences,test_labels,0) << endl;
		//cout << "Test error with tuned threshold: " << classify(test_sequences,test_labels,threshold) << endl;


		//cout << "(Longest) learning time: " << learn_time << endl;
		//cout << "(Longest) classifying time: " << classification_time << endl;



	}



};



int main(int argc, char **argv){

	cout << "Hello World!" << endl;
	string train_data = string(argv[1]);
	string test_data = string(argv[2]);
	string work_dir = string(argv[3]);
	int window_size = atoi(argv[4]);
	int word_length = atoi(argv[5]);
	int alphabet_size = atoi(argv[6]);
	//srand (0);
	//int N = 10;

	//double min_error = 1.0;
	//int best_config[3] = {0,0,0};
	SAXSEQL seql_obj(train_data,test_data, work_dir, window_size, word_length, alphabet_size);

	cout << "N=" << window_size << ",w=" << word_length << ",a=" << alphabet_size << "," << endl;
	seql_obj.run_sax_seql_with_k_folds();

}






