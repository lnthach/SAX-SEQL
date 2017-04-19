./seql_learn -n 1 -v 1 data/test_TRAIN seql/seql.model 
./seql_mkmodel -i seql/seql.model -o seql/seql.model.bin -O seql/seql.predictor 
./seql_classify -n 1 -v 4 -w 4 data/test_TEST seql/seql.model.bin 
