/*
 * Authors: Georgiana Ifrim (georgiana.ifrim@gmail.com)
 *		    Thach Le Nguyen (thach.lenguyen@insight-centre.org)
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation.
 *
 */

#ifndef SEQL_MKMODEL_H_
#define SEQL_MKMODEL_H_


#include <string>

//#define OPT " [-i model_file] [-o binary_model_file] [-O predictors_file]"



int mkmodel (std::string file, std::string index, std::string ofile);



#endif /* SEQL_MKMODEL_H_ */
