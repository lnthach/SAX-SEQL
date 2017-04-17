cmake_minimum_required(VERSION 2.8)
add_definitions(-std=c++11)
add_definitions(-Wall)

project("SEQL")

## Install the executable to bin fodler
set(EXECUTABLE_OUTPUT_PATH bin)

add_executable(sax_convert sax_converter.cpp)
add_executable(seql_learn seql_learn_exec.cpp seql_learn.cpp SNode.cpp)
add_executable(seql_mkmodel seql_mkmodel_exec.cpp seql_mkmodel.cpp)
#add_executable(seql_mkmodel_regression seql_mkmodel_regression.cpp)
add_executable(seql_classify seql_classify_exec.cpp seql_classify.cpp
                             str2node_string_symbol.cpp)
#add_executable(seql_regress seql_regress.cpp str2node_string_symbol.cpp)
add_executable(seql_classify_tune_threshold_min_errors seql_classify_tune_threshold_min_errors.cpp
                                                      str2node_string_symbol.cpp)
#add_executable(seql_crv leave_one_out.cpp seql_learn.cpp SNode.cpp seql_mkmodel.cpp seql_classify.cpp
#							str2node_string_symbol.cpp)
add_executable(seql_multiclass seql_multiclass.cpp seql_learn.cpp SNode.cpp seql_mkmodel.cpp seql_classify.cpp
							str2node_string_symbol.cpp)
#add_executable(seql_multiclass_test seql_multiclass_2.cpp seql_learn.cpp SNode.cpp seql_mkmodel.cpp seql_classify.cpp
#							str2node_string_symbol.cpp)
#add_executable(seql_testtrie seql_testtrie.cpp)
