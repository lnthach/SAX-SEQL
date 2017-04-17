/*
 * Author: Georgiana Ifrim (georgiana.ifrim@gmail.com)
 * SEQL: Sequence Learner
 * This library trains ElasticNet-regularized Logistic Regression and L2-loss (squared-hinge-loss) SVM for Classifying Sequences in the feature space of all possible
 * subsequences in the given training set.
 * Elastic Net regularizer: alpha * L1 + (1 - alpha) * L2, which combines L1 and L2 penalty effects. L1 influences the sparsity of the model, L2 corrects potentially high
 * coeficients resulting due to feature correlation (see Regularization Paths for Generalized Linear Models via Coordinate Descent, by Friedman et al, 2010).
 *
 * The user can influence the outcome classification model by specifying the following parameters:
 * [-o objective] (objective function; choice between logistic regression, squared-hinge-svm and squared error. By default: logistic regression.)
 * [-T maxitr] (number of optimization iterations; by default this is set using a convergence threshold on the aggregated change in score predictions.)
 * [-l minpat] (constraint on the min length of any feature)
 * [-L maxpat] (constraint on the max length of any feature)
 * [-m minsup] (constraint on the min support of any feature, i.e. number of sequences containing the feature)
 * [-g maxgap] (number of total wildcards allowed in a feature, e.g. a**b, is a feature of size 4 with any 2 characters in the middle)
 * [-G maxcongap] (number of consecutive wildcards allowed in a feature, e.g. a**b, is a feature of size 4 with any 2 characters in the middle)
 * [-n token_type] (word or character-level token to allow sequences such as 'ab cd ab' and 'abcdab')
 * [-C regularizer_value] value of the regularization parameter, the higher value means more regularization
 * [-a alpha] (weight on L1 vs L2 regularizer, alpha=0.5 means equal weight for l1 and l2)
 * [-r traversal_strategy] (BFS or DFS traversal of the search space), BFS by default
 * [-c convergence_threshold] (stopping threshold for optimisation iterations based on change in aggregated score predictions)
 * [-v verbosity] (amount of printed detail about the model)
 *
 * License:
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation.
 *
 */

/* The obj fct is: loss(x, y, beta) + C * ElasticNetReg(alpha, beta).
 */

using namespace std;
#include "seql_learn.h"

#define OPT " [-o objective_function] [-m minsup] [-l minpat] [-L maxpat] [-g gap] [-r traversal_strategy ] [-T #round] [-n token_type] [-c convergence_threshold] [-C regularizer_value] [-a l1_vs_l2_regularizer_weight] [-v verbosity] training_file model_file"

int main (int argc, char **argv)
{

    extern char *optarg;


    //unsigned int sax_window_size = 20;
    //unsigned int sax_word_length = 4;
    unsigned int sax_alphabet_size = 4;

    double max_distance = 1.0;

    unsigned int loo = 0;
    // By default the objective fct is logistic regression.
    // objective=0: SLR
    // objective=1: for Hinge loss aka l1-loss-SVM (disabled for now)
    // objective=2: for squared-hinge-loss aka l2-loss-SVM
    // objective=3: for squared error
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
    bool token_type = 1;
    // BFS vs DFS traversal. By default BFS.
    bool traversal_strategy = 0;

    // The C regularizer parameter in regularized loss formulation. It constraints the weights of features.
    // C = 0 no constraints (standard SLR), the larger the C, the more the weights are shrinked towards each other (using L2) or towards 0 (using L1)
    double C = 1;
    // The alpha parameter decides weight on L1 vs L2 regularizer: alpha * L1 + (1 - alpha) * L2. By default we use an L2 regularizer.
    double alpha = 0.2;

    double convergence_threshold = 0.005;
    int verbosity = 1;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << OPT << std::endl;
        return -1;
    }

    int opt;
    while ((opt = getopt(argc, argv, "o:T:L:l:m:g:G:n:r:c:C:a:v:A:d:")) != -1) {
        switch(opt) {
//        case 'N':
//        	sax_window_size = atoi (optarg);
//            break;
//        case 'W':
//        	sax_word_length = atoi (optarg);
//            break;
        case 'A':
        	sax_alphabet_size = atoi (optarg);
            break;
        case 'd':
        	max_distance = atof (optarg);
            break;
        case 'o':
            objective = atoi (optarg);
            break;
        case 'T':
            maxitr = atoi (optarg);
            break;
        case 'L':
            maxpat = atoi (optarg);
            break;
        case 'l':
            minpat = atoi (optarg);
            break;
        case 'm':
            minsup = atoi (optarg);
            break;
        case 'g':
            maxgap = atoi (optarg);
            break;
        case 'G':
            maxcongap = atoi (optarg);
            break;
        case 'n':
            token_type = atoi (optarg);
            break;
        case 'r':
            traversal_strategy = atoi (optarg);
            break;
        case 'c':
            convergence_threshold = atof (optarg);
            break;
        case 'C':
            C = atof (optarg);
            break;
        case 'a':
            alpha = atof (optarg);
            break;
        case 'v':
            verbosity = atoi (optarg);
            break;
        default:
            std::cout << "Usage: " << argv[0] << OPT << std::endl;
            return -1;
        }
    }


    if (verbosity >= 1) {
        cout << "\nParameters used: " << "obective fct: " << objective << " T: " << maxitr << " minpat: " << minpat << " maxpat: " << maxpat << " minsup: " << minsup
             << " maxgap: " << maxgap << " maxcongap: " << maxcongap << " token_type: " << token_type << " traversal_strategy: " << traversal_strategy
             << " convergence_threshold: "  << convergence_threshold << " C (regularizer value): " << C << " alpha (weight on l1_vs_l2_regularizer): "
             << alpha  << " verbosity: " << verbosity<< endl;
    }

    SeqLearner seql_learner;

    seql_learner.run (argv[argc-2], argv[argc-1], sax_alphabet_size, max_distance, objective, maxpat, minpat, maxitr,
                      minsup, maxgap, maxcongap, token_type, traversal_strategy, convergence_threshold, C, alpha, verbosity);

    return 0;
}
