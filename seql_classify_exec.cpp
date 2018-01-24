/*
 * Authors: Georgiana Ifrim (georgiana.ifrim@gmail.com)
 *		    Thach Le Nguyen (thach.lenguyen@insight-centre.org)
 *
 * A customized (tuned) classification threshold can be provided as input to the classifier.
 * The program simply applies a suffix tree model to the test documents for predicting classification labels.
 * Prec, Recall, F1 and Accuracy are reported.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation.
 *
 */

#include "seql_classify.h"

#define OPT " [-n token_type: 0 word tokens, 1 char tokens] [-t classif_threshold] [-v verbose] test_file binary_model_file"

int main (int argc, char **argv)
{

	//unsigned int sax_window_size = 20;
	//unsigned int sax_word_length = 4;
	//unsigned int sax_alphabet_size = 4;
	std::string predictor;
	double max_distance = 0.0;


    std::istream *is = 0;
    unsigned int verbose = 0;
    double threshold = 0; // By default zero threshold = zero bias.
    // By default char tokens.
    bool token_type = 1;
    // Profiling variables.
    struct timeval t;
    struct timeval t_origin;

    gettimeofday(&t_origin, NULL);

    int opt;
    while ((opt = getopt(argc, argv, "n:t:v:p:d:")) != -1) {
    	switch(opt) {
    	//case 'N':
    	//	sax_window_size = atoi (optarg);
    	//	break;
    	//case 'W':
    	//	sax_word_length = atoi (optarg);
    	//	break;
    	//case 'A':
    	//	sax_alphabet_size = atoi (optarg);
    	//	break;
    	case 'p':
    		predictor = std::string (optarg);
    	    break;
    	case 'n':
    		token_type = atoi(optarg);
    		break;
    	case 't':
    		threshold = atof(optarg);
    		break;
    	case 'd':
    	    max_distance = atof(optarg);
    	    break;
    	case 'v':
    		verbose = atoi(optarg);
    		break;
    	default:
    		std::cout << "Usage: " << argv[0] << OPT << std::endl;
    		return -1;
    	}
    }

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << OPT << std::endl;
        return -1;
    }

    if (! strcmp (argv[argc - 2], "-")) {
        is = &std::cin;
    } else {
        is = new std::ifstream (argv[argc - 2]);
        if (! *is) {
            std::cerr << argv[0] << " " << argv[argc-2] << " No such file or directory" << std::endl;
            return -1;
        }
    }


    SEQLClassifier seql;

    if (verbose >= 3) seql.setRule (true);

    if (! seql.open (argv[argc-1], threshold)) {
        std::cerr << argv[0] << " " << argv[argc-1] << " No such file or directory" << std::endl;
        return -1;
    }

    std::string line;
    char *column[4];
    // Predicted and true scores for all docs.
    vector<pair<double, int> > scores;

    unsigned int all = 0;
    unsigned int correct = 0;
    unsigned int res_a = 0;
    unsigned int res_b = 0;
    unsigned int res_c = 0;
    unsigned int res_d = 0;


    seql.load_mytrie(predictor.c_str(),threshold);

    //SAX sax_converter(sax_window_size,sax_word_length,sax_alphabet_size);

    //cout << "\n\nreading test data...\n";
    while (std::getline (*is, line)) {

        if (line[0] == '\0' || line[0] == ';') continue;
        if (line[line.size() - 1] == '\r') {
            line[line.size() - 1] = '\0';
        }
        //cout << "\nline:*" << aux.c_str() << "*";

        if (2 != tokenize ((char *)line.c_str(), "\t ", column, 2)) {
            std::cerr << "Format Error: " << line.c_str() << std::endl;
            return -1;
        }

        //cout <<"\ncolumn[0]:*" << column[0] << "*";
        //cout <<"\ncolumn[1]:*" << column[1] << "*";
        //cout.flush();

        int y = atoi (column[0]);
        //cout << "\ny: " << y;

        //char *del = new char(' ');
        //column[1] = sax_converter.timeseries2SAX(column[1],del);


        //double predicted_score = seql.classify (column[1], token_type);
        double predicted_score = seql.classify_with_mytrie(column[1], max_distance);


        // Keep predicted and true score.
        scores.push_back(pair<double, int>(predicted_score, y));

        // Transform the predicted_score which is a real number, into a probability,
        // using the logistic transformation: exp^{predicted_score} / 1 + exp^{predicted_score} = 1 / 1 + e^{-predicted_score}.
        double predicted_prob;
        if (predicted_score < -8000) {
            predicted_prob = 0;
        } else {
            predicted_prob = 1.0 / (1.0 + exp(-predicted_score));
        }

        if (verbose == 1) {
            std::cout << y << " " << predicted_score << " " << predicted_prob << std::endl;
        } else if (verbose == 2) {
            std::cout << y << " " << predicted_score << " " << predicted_prob <<  " " << column[1] << std::endl;
        } else if (verbose == 4) {
            std::cout << "<instance>" << std::endl;
            std::cout << y << " " << predicted_score << " " << predicted_prob << " " << column[1] << std::endl;
            seql.printRules (std::cout);
            std::cout << "</instance>" << std::endl;
        } else if (verbose == 5) {
            std::cout << y << " ";
            seql.printIds (std::cout);
        }

        all++;
        if (predicted_score > 0) {
            if(y > 0) correct++;
            if(y > 0) res_a++; else res_b++;
        } else {
            if(y < 0) correct++;
            if(y > 0) res_c++; else res_d++;
        }
    }

    double prec = 1.0 * res_a/(res_a + res_b);
    if (res_a + res_b == 0) prec = 0;
    double rec  = 1.0 * res_a/(res_a + res_c);
    if (res_a + res_c == 0) rec = 0;
    double f1 =  2 * rec * prec / (prec+rec);
    if (prec + rec == 0) f1 = 0;

    double specificity = 1.0 * res_d/(res_d + res_b);
    if (res_d + res_b == 0) specificity = 0;
    // sensitivity = recall
    double sensitivity  = 1.0 * res_a/(res_a + res_c);
    if (res_a + res_c == 0) sensitivity = 0;
    double fss =  2 * specificity * sensitivity / (specificity + sensitivity);
    if (specificity + sensitivity == 0) fss = 0;

    // Sort the scores ascendingly by the predicted score.
    sort(scores.begin(), scores.end());
    double AUC = seql.calcROC(scores);
    double AUC50 = seql.calcROC50(scores);
    double balanced_error = 0.5 * ((1.0 * res_c / (res_a + res_c)) + (1.0 * res_b / (res_b + res_d)));

    //if (verbose >= 3) {
    std::printf ("Classif Threshold:   %.5f\n", -seql.getBias());
    std::printf ("Accuracy:   %.5f%% (%d/%d)\n", 100.0 * correct / all , correct, all);
    std::printf ("Error:      %.5f%% (%d/%d)\n", 100.0 - 100.0 * correct / all, all - correct, all);
    std::printf ("Balanced Error:     %.5f%%\n", 100.0 * balanced_error);
    std::printf ("AUC:        %.5f%%\n", AUC);
    //std::printf ("(1 - AUC):   %.5f%%\n", 100 - AUC);
    std::printf ("AUC50:      %.5f%%\n", AUC50);
    std::printf ("Precision:  %.5f%% (%d/%d)\n", 100.0 * prec,  res_a, res_a + res_b);
    std::printf ("Recall:     %.5f%% (%d/%d)\n", 100.0 * rec, res_a, res_a + res_c);
    std::printf ("F1:         %.5f%%\n",         100.0 * f1);
    std::printf ("Specificity:  %.5f%% (%d/%d)\n", 100.0 * specificity,  res_d, res_d + res_b);
    std::printf ("Sensitivity:     %.5f%% (%d/%d)\n", 100.0 * sensitivity, res_a, res_a + res_c);
    std::printf ("FSS:         %.5f%%\n",         100.0 * fss);

    std::printf ("System/Answer p/p p/n n/p n/n: %d %d %d %d\n", res_a,res_b,res_c,res_d);
    std::printf ("OOV docs:   %d\n", seql.getOOVDocs());

    gettimeofday(&t, NULL);
    cout << "end classification( " << (t.tv_sec - t_origin.tv_sec) << " seconds; " << (t.tv_sec - t_origin.tv_sec) / 60.0 << " minutes )\n";
    cout.flush();
    //}
    if (is != &std::cin) delete is;

    return 0;
}
