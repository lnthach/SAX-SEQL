train=data/${1}_TRAIN
test=data/${1}_TEST
wd=seql/$1
N=$2
w=$3
a=$4
d=$5

mkdir -p $wd
rm $wd/*

./sax_convert -n 0 -s 2 -N $N -w $w -a $a -i $train -o $wd/sax.train
./sax_convert -n 0 -s 2 -N $N -w $w -a $a -i $test -o $wd/sax.test
./seql_learn -n 1 -v 1 -A $a -d $d $wd/sax.train $wd/seql.model > $wd/learn.log
./seql_mkmodel -i $wd/seql.model -o $wd/seql.model.bin -O $wd/seql.predictor > $wd/mkmodel.log
./seql_classify -n 1 -v 0 -p $wd/seql.predictor -d $d $wd/sax.test $wd/seql.model.bin > $wd/classify.log

cat $wd/classify.log | grep Error
