/*
 * seql_mkmodel.h
 *
 *  Created on: 6 Jul 2016
 *      Author: thachln
 */

#ifndef SEQL_MKMODEL_H_
#define SEQL_MKMODEL_H_


#include <string>

//#define OPT " [-i model_file] [-o binary_model_file] [-O predictors_file]"



int mkmodel (std::string file, std::string index, std::string ofile);



#endif /* SEQL_MKMODEL_H_ */
