train=data/${1}_TRAIN
test=data/${1}_TEST
N=$2
w=$3
a=$4
g=$5

./sax_convert -m 0 -N $N -w $w -a $a -i $train -o seql/sax.train
./sax_convert -m 0 -N $N -w $w -a $a -i $test -o seql/sax.test
./seql_learn -n 1 -g $g -v 1 seql/sax.train seql/seql.model > seql/learn.log
./seql_mkmodel -i seql/seql.model -o seql/seql.model.bin -O seql/seql.predictor > seql/mkmodel.log
./seql_classify -n 1 -v 1 -w $w seql/sax.test seql/seql.model.bin #> seql/classify.log
