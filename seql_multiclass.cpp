/*
 * seql_multiclass.cpp
 *
 *  Created on: 6 Jul 2016
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

class MulticlassSEQL {

private:

	// 0 by VOTE
	// 1 by sum score
	int FINAL_DECISION = 0;

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


		seql_learner.run (sax_train.c_str(), model.c_str(), alphabet_size, 1.0, objective, maxpat, minpat, maxitr,
				minsup, maxgap, maxcongap, token_type, traversal_strategy, convergence_threshold, C, alpha, verbosity);
	}

	// only return predicted score
	// use for one-vs-one scheme
	void predict(vector<double>& predict_score){

		unsigned int verbose = 0;
		double threshold = 0; // By default zero threshold = zero bias.

		SEQLClassifier seql;
		predict_score.clear();

		if (verbose >= 3) seql.setRule (true);
		//cout << model_bin << "****" << endl;
		if (! seql.open (model_bin.c_str(), threshold)) {
			std::cerr << " " << model_bin << " No such file or directory" << std::endl;
		}

		unsigned int all = 0;
		unsigned int correct = 0;

		seql.load_mytrie(model_predictors.c_str(), threshold);

		for (unsigned int item = 0; item < test_size;++item){
			//double predicted_score = seql.classify (sequences[item].c_str(), token_type);
			predict_score.push_back(seql.classify_with_mytrie(test_sequences[item].c_str(), max_distance));
		}
	}

	double tune_threshold(double positive_label){
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

		for (unsigned int item = 0; item < train_size;++item){
			double predicted_score = seql.classify_with_mytrie(train_sequences[item].c_str(), max_distance);
			scores.push_back(pair<double, int>(predicted_score, train_labels[item]));
			if (train_labels[item] == positive_label){
				num_positives++;
			}
		}

		// Sort the scores ascendingly by the predicted score.
		sort(scores.begin(), scores.end());

		unsigned int TP = num_positives;
		unsigned int FP = train_size - num_positives;
		unsigned int FN = 0;
		unsigned int TN = 0;

		unsigned int min_error = FP + FN;
		unsigned int current_error = 0;
		double best_threshold = -numeric_limits<double>::max();

		for (unsigned int i = 0; i < train_size; ++i) {
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
			if (scores[i].second == positive_label) {
				FN++; TP--;
			}else{
				FP--; TN++;
			}
		}

		return best_threshold;

	}

	double classify(double positive_label, vector<double> &current_preds,vector<double> &current_scores, double threshold){
		unsigned int verbose = 0;
		//double threshold = 0; // By default zero threshold = zero bias.


		// Profiling variables.
		// struct timeval t;
		//		struct timeval t_origin;
		//		gettimeofday(&t_origin, NULL);

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

		for (unsigned int item = 0; item < test_size;++item){
			//int item = test_fold[ic];
			//int y = int (tmp_labels[item]);
			//double predicted_score = seql.classify (sequences[item].c_str(), token_type);
			double predicted_score = seql.classify_with_mytrie(test_sequences[item].c_str(), max_distance);

			double predicted_prob;
			if (predicted_score < -8000) {
				predicted_prob = 0;
			} else {
				predicted_prob = 1.0 / (1.0 + exp(-predicted_score));
			}

			//if (predicted_score > current_scores[item]){
			//	current_scores[item] = predicted_score;
			//	current_preds[item] = positive_label;
			//}
			if (predicted_prob > current_scores[item]){
				current_scores[item] = predicted_prob;
				current_preds[item] = positive_label;
			}

			// Keep predicted and true score.
			//scores.push_back(pair<double, int>(predicted_score, y));

			// Transform the predicted_score which is a real number, into a probability,
			// using the logistic transformation: exp^{predicted_score} / 1 + exp^{predicted_score} = 1 / 1 + e^{-predicted_score}.


			//			if (verbose == 1) {
			//				std::cout << y << " " << predicted_score << " " << predicted_prob << std::endl;
			//			} else if (verbose == 2) {
			//				std::cout << y << " " << predicted_score << " " << predicted_prob <<  " " << sequences[item] << std::endl;
			//			} else if (verbose == 4) {
			//				std::cout << "<instance>" << std::endl;
			//				std::cout << y << " " << predicted_score << " " << predicted_prob << " " << sequences[item] << std::endl;
			//				seql.printRules (std::cout);
			//				std::cout << "</instance>" << std::endl;
			//			} else if (verbose == 5) {
			//				std::cout << y << " ";
			//				seql.printIds (std::cout);
			//			}

			//			all++;
			//			if (predicted_score > 0) {
			//				if(y > 0) correct++;
			//				//if(y > 0) res_a++; else res_b++;
			//			} else {
			//				if(y < 0) correct++;
			//				//if(y > 0) res_c++; else res_d++;
			//			}
		}

		// Sort the scores ascendingly by the predicted score.
		//sort(scores.begin(), scores.end());
		//double error = 1.0 - 1.0*correct / all;

		//if(std::isnan(error)){
		//	cout << "FATAL ERROR: NAN" << endl;
		//	cout << "correct/all=" << correct << "/" << all << endl;

		//}
		//return error;

	}

public:
	vector<int> stratified_k_folds(vector<int> &labels,int k){
		vector<int> folds;
		int fold_capacities[k];
		int remaining = labels.size() % k;
		int base_cap = labels.size() / k;

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
		for (unsigned int i = 0; i < labels.size(); i++){
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



	MulticlassSEQL(string _train_data, string _test_data, string _work_dir, int sax_N, int sax_w, int sax_a){

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

	double multiclass_ova_undersampling(){
		clock_t learn_starttime, classification_starttime;
		double tmp_learn_time, learn_time, tmp_classification_time, classification_time, max_time = 0;
		vector<string> tmp_sequences;
		vector<double> train_tmp_labels;
		vector<double> selection;
		std::vector<double> predicted_labels(test_size);
		std::vector<double> predicted_scores(test_size);
		for (int i = 0; i < test_size; ++i){
			predicted_labels[i] = 1.0;
			predicted_scores[i] = -std::numeric_limits<double>::infinity();
		}
		for(auto positive_label : label_set) {
			double threshold = 0;
			train_tmp_labels.clear();
			tmp_sequences.clear();

			undersampling(selection,positive_label);
			for (int i = 0; i < train_size; ++i){
				if (selection[i] != 0.0){
					train_tmp_labels.push_back(selection[i]);
					tmp_sequences.push_back(train_sequences[i]);
				}
			}

			cout << "Label " << positive_label << " - Train Size: " << train_tmp_labels.size() << endl;


			//NOTE: remove later
			model = model + "1";
			model_predictors = model_predictors + "1";

			// ******************************LEARNING PHASE******************************
			learn_starttime = clock();
			learn(tmp_sequences, train_tmp_labels);
			tmp_learn_time = double(clock() - learn_starttime) / CLOCKS_PER_SEC;
			// ******************************MKMODEL PHASE******************************
			if (mkmodel(model,model_bin,model_predictors) < 0){
				cout << "Fail to prepare model file.\n";
			}
			// ******************************TUNE THRESHOLD******************************
			threshold = tune_threshold(positive_label);
			// ******************************CLASSIFICATION PHASE******************************
			classification_starttime = clock();
			classify(positive_label,predicted_labels,predicted_scores,threshold);
			tmp_classification_time = double(clock() - classification_starttime) / CLOCKS_PER_SEC;


			// only keep runtime of the most expensive
			if (tmp_learn_time + tmp_classification_time > max_time){
				learn_time = tmp_learn_time;
				classification_time = tmp_classification_time;
				max_time = tmp_learn_time + tmp_classification_time;
			}


		}

		cout << "(Longest) learning time: " << learn_time << endl;
		cout << "(Longest) classifying time: " << classification_time << endl;

		int correct = 0;
		for (int i = 0; i < test_size; ++i){
			if (test_labels[i] == predicted_labels[i]){
				correct++;
			}
		}
		return 1.0 - correct*1.0/test_size;
	}

	double multiclass_seql(){
		clock_t learn_starttime, classification_starttime;
		double tmp_learn_time, learn_time, tmp_classification_time, classification_time, max_time = 0;


		std::vector<double> predicted_labels(test_size);
		std::vector<double> predicted_scores(test_size);
		for (int i = 0; i < test_size; ++i){
			predicted_labels[i] = 1.0;
			predicted_scores[i] = -std::numeric_limits<double>::infinity();
		}
		for(auto positive_label : label_set) {
			double threshold = 0;
			std::vector<double> train_tmp_labels(train_size);
			for (int i = 0; i < train_size; ++i){
				if (train_labels[i] == positive_label){
					train_tmp_labels[i] = 1.0;
				} else {
					train_tmp_labels[i] = -1.0;
				}
			}

			std::vector<double> test_tmp_labels(test_size);
			for (int i = 0; i < test_size; ++i){
				if (test_labels[i] == positive_label){
					test_tmp_labels[i] = 1.0;
				} else {
					test_tmp_labels[i] = -1.0;
				}
			}
			//NOTE: remove later
			model = model + "1";
			model_predictors = model_predictors + "1";

			// ******************************LEARNING PHASE******************************
			learn_starttime = clock();
			learn(train_sequences, train_tmp_labels);
			tmp_learn_time = double(clock() - learn_starttime) / CLOCKS_PER_SEC;
			// ******************************MKMODEL PHASE******************************
			if (mkmodel(model,model_bin,model_predictors) < 0){
				cout << "Fail to prepare model file.\n";
			}
			// ******************************TUNE THRESHOLD******************************
			threshold = tune_threshold(positive_label);
			// ******************************CLASSIFICATION PHASE******************************
			classification_starttime = clock();
			classify(positive_label,predicted_labels,predicted_scores,threshold);
			tmp_classification_time = double(clock() - classification_starttime) / CLOCKS_PER_SEC;


			// only keep runtime of the most expensive
			if (tmp_learn_time + tmp_classification_time > max_time){
				learn_time = tmp_learn_time;
				classification_time = tmp_classification_time;
				max_time = tmp_learn_time + tmp_classification_time;
			}


		}

		cout << "(Longest) learning time: " << learn_time << endl;
		cout << "(Longest) classifying time: " << classification_time << endl;

		int correct = 0;
		for (int i = 0; i < test_size; ++i){
			if (test_labels[i] == predicted_labels[i]){
				correct++;
			}
		}
		return 1.0 - correct*1.0/test_size;
	}

	// Implement one-vs-one strategy
	double multiclass_ovo_seql(){
		clock_t learn_starttime, classification_starttime;
		double tmp_learn_time, learn_time, tmp_classification_time, classification_time, max_time = 0;

		vector<string> tmp_sequences;
		vector<double> train_tmp_labels;

		//std::vector<double> predicted_labels(test_size);
		std::vector<double> predicted_scores(test_size);

		int votes[test_size][label_set.size()];
		double sum_scores[test_size][label_set.size()];

		double positive_label, negative_label;

		for (int i = 0; i < test_size; ++i){
			for (int j = 0; j < label_set.size(); ++j){
				votes[i][j] = 0;
				sum_scores[i][j] = 0;
			}
			//predicted_labels[i] = 1.0;
			//predicted_scores[i] = -std::numeric_limits<double>::infinity();

		}
		// for every pair of labels
		for (int pl = 0; pl < label_set.size() - 1; pl++) {
			for (int nl = pl+1; nl < label_set.size(); nl++ ) {
				train_tmp_labels.clear();
				tmp_sequences.clear();
				positive_label = label_set[pl];
				negative_label = label_set[nl];
				// extract training data
				for (int i = 0; i < train_size; ++i){
					if (train_labels[i] == positive_label){
						train_tmp_labels.push_back(1.0);
						tmp_sequences.push_back(train_sequences[i]);
					} else if (train_labels[i] == negative_label) {
						train_tmp_labels.push_back(-1.0);
						tmp_sequences.push_back(train_sequences[i]);
					}
				}


				//NOTE: remove later
				model = model + "1";
				model_predictors = model_predictors + "1";

				// ******************************LEARNING PHASE******************************
				learn_starttime = clock();
				learn(tmp_sequences,train_tmp_labels);
				tmp_learn_time = double(clock() - learn_starttime) / CLOCKS_PER_SEC;
				// ******************************MKMODEL PHASE******************************
				if (mkmodel(model,model_bin,model_predictors) < 0){
					cout << "Fail to prepare model file.\n";
				}
				// ******************************CLASSIFICATION PHASE******************************
				classification_starttime = clock();
				//classify(positive_label,predicted_labels,predicted_scores);
				predict(predicted_scores);
				// add vote and score
				for (int ip = 0; ip < test_size; ip++){
					if (predicted_scores[ip] > 0){
						votes[ip][pl]++;
						sum_scores[ip][pl] += predicted_scores[ip];
					} else {
						votes[ip][nl]++;
						sum_scores[ip][nl] -= predicted_scores[ip];
					}
				}


				tmp_classification_time = double(clock() - classification_starttime) / CLOCKS_PER_SEC;


				// only keep runtime of the most expensive
				if (tmp_learn_time + tmp_classification_time > max_time){
					learn_time = tmp_learn_time;
					classification_time = tmp_classification_time;
					max_time = tmp_learn_time + tmp_classification_time;
				}

			}
		}

		cout << "(Longest) learning time: " << learn_time << endl;
		cout << "(Longest) classifying time: " << classification_time << endl;

		int correct = 0;
		for (int i = 0; i < test_size; ++i){
			int max_vote = 0;
			double max_score = 0;
			double selected_label = 0;
			for (int l = 0; l < label_set.size(); l++){

				if (FINAL_DECISION == 0){ // decide by votes
					if (votes[i][l] > max_vote){
						max_vote = votes[i][l];
						selected_label = label_set[l];
					}
				} else if (FINAL_DECISION == 1){ // decide by sum score
					if (sum_scores[i][l] > max_score){
						max_score = sum_scores[i][l];
						selected_label = label_set[l];
					}
				}
			}
			if (selected_label == test_labels[i]){
				correct++;
			}
		}

		return 1.0 - correct*1.0/test_size;
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
	MulticlassSEQL seql_obj(train_data,test_data, work_dir, window_size, word_length, alphabet_size);

	cout << "N=" << window_size << ",w=" << word_length << ",a=" << alphabet_size << "," << seql_obj.multiclass_seql() << endl;

}



