train=data/${1}_TRAIN
test=data/${1}_TEST
minl=$2
maxl=$3
step=$4
w=$5
a=$6

timestamp=$(date +%s)
wd=awseql/${1} #${timestamp}
mkdir -p $wd
rm $wd/*

time ./sax_convert -m 1 -s $step -n $minl -x $maxl -w $w -a $a -i $train -o $wd/sax.train
time ./sax_convert -m 1 -s $step -n $minl -x $maxl -w $w -a $a -i $test -o $wd/sax.test
time ./seql_learn -n 1 -v 1 $wd/sax.train $wd/seql.model > $wd/learn.log
./seql_mkmodel -i $wd/seql.model -o $wd/seql.model.bin -O $wd/seql.predictor > $wd/mkmodel.log
time ./seql_classify -n 1 -v 1 -w $w $wd/sax.test $wd/seql.model.bin > $wd/classify.log

tail -20 $wd/classify.log

rm $wd/sax.train
rm $wd/sax.test


