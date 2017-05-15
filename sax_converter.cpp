/*
 * sax_converter.cpp
 *
 *  Created on: 27 Jun 2016
 *      Author: LE NGUYEN THACH
 * Descriptions of parameters:
 * [-i input] (Input of time series data)
 * [-o output] (Output of SAX data)
 * [-N window_size] (Size of the sliding window)
 * [-w word_length] (length of SAX word)
 * [-a alphabet_size] (Size of the alphabet)
 * [-n no_window] (Set window_size with length of input time series. Ignore window_size parameter.)
 * [-s reduction_strategy] 0 - No reduction
 * 						   1 - No identical consecutive words.
 * 						   2 - Unique word only
 */


#include "sax_converter.h"
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc, char **argv){

	int window_size = 20;
	int word_length = 4;
	int alphabet_size = 4;
	int step_size = 1;

	// character-level 1
	// word-level 0
	int token_type = 0;

	int reduction_strategy = 2;


	string train_input = "";
	string test_input = "";
	string train_output = "";
	string test_output = "";

	int opt;
	while ((opt = getopt(argc, argv, "I:i:O:o:N:w:a:n:s:t:")) != -1) {
		switch(opt) {
		case 'i':
			train_input = string (optarg);
			break;
		case 'I':
			test_input = string (optarg);
			break;
		case 'o':
			train_output = string(optarg);
			break;
		case 'O':
			test_output = string(optarg);
			break;
		case 'N':
			window_size = atoi(optarg);
			break;
		case 'w':
			word_length = atoi(optarg);
			break;
		case 'a':
			alphabet_size = atoi(optarg);
			break;
		case 'n':
			token_type = atoi(optarg);
			break;
		case 's':
			reduction_strategy = atoi(optarg);
			break;
		case 't':
			step_size = atoi(optarg);
			break;
		default:
			std::cout << "Usage: " << argv[0] << std::endl;
			return -1;
		}
	}



	//	std::cout << input << window_size << word_length << alphabet_size << std::endl;
	SAX* obj;
	if (token_type){
		obj = new SAX(word_length, alphabet_size);
	} else {
		obj = new SAX(window_size, word_length, alphabet_size, step_size, reduction_strategy);
	}



	//char *del = " ";
	//char *line = new char[100000];
	string del = " ";
	string line;
	size_t pos = 0;
	string label;
	string sax_str;


	// convert train data
	if (!(train_input.empty() || train_output.empty()) ){
		std::ifstream myfile (train_input);
		std::ofstream outfile (train_output);

		bool trained_break_points = false;

		if (myfile.is_open() && outfile.is_open()){
			while (getline (myfile, line)){
				pos = line.find(del);
				label = line.substr(0, pos);
				line.erase(0, pos + del.length());
				sax_str = obj->timeseries2SAX(line,del);
				outfile << label << " " << sax_str << '\n';


			}
			myfile.close();
			outfile.close();
		} else {
			if (!myfile.is_open())	std::cout << "Invalid Timeseries Data Input." << std::endl;
			if (!outfile.is_open())	std::cout << "Invalid output for SAX." << std::endl;
			return false;
		}
	}

	// convert test data
	if (!(test_input.empty() || test_output.empty()) ){
		std::ifstream test_myfile (test_input);
		std::ofstream test_outfile (test_output);

		if (test_myfile.is_open() && test_outfile.is_open()){
			while (getline (test_myfile, line)){
				pos = line.find(del);
				label = line.substr(0, pos);
				line.erase(0, pos + del.length());
				sax_str = obj->timeseries2SAX(line,del);
				test_outfile << label << " " << sax_str << '\n';
			}
			test_myfile.close();
			test_outfile.close();
		} else {
			if (!test_myfile.is_open())	std::cout << "Invalid Timeseries Data Input." << std::endl;
			if (!test_outfile.is_open())	std::cout << "Invalid output for SAX." << std::endl;
			return false;
		}
	}
}
