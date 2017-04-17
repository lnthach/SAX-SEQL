/*
 * Author: Georgiana Ifrim (georgiana.ifrim@gmail.com)
 *
 * This library takes as input the classification model provided by seql_learn.cpp (with potential repetitions of the same feature),
 * prepares the final model by aggregating the weights of repeated (identical) features
 * and builds a trie from the resulting (unique) features for fast classification (as done in seql_classify.cpp).
 *
 * The library uses parts of Taku Kudo's
 * open source code for BACT, available from: http://chasen.org/~taku/software/bact/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation.
 *
 */



#include <string>
#include <iostream>
#include <unistd.h>
#include "seql_mkmodel.h"



#define OPT " [-i model_file] [-o binary_model_file] [-O predictors_file]"

int main (int argc, char **argv)
{
    std::string file  = "";
    std::string index = "";
    std::string ofile = "";
    extern char *optarg;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:O:")) != -1) {
        switch(opt) {
        case 'i':
            file = std::string (optarg);
            break;
        case 'o':
            index = std::string (optarg);
            break;
        case 'O':
            ofile = std::string (optarg);
            break;
        default:
            std::cout << "Usage: " << argv[0] << OPT << std::endl;
            return -1;
        }
    }

    if (file.empty () || index.empty ()) {
        std::cout << "Usage: " << argv[0] << OPT << std::endl;
        return -1;
    }

    mkmodel(file,index,ofile);

    return 0;
}
