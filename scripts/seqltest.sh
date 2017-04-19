traindata=data/test_TRAIN
testdata=data/test_TEST
wd=seql/test
a=8
d=1

mkdir -p $wd
rm $wd/*

./seql_learn -n 1 -v 0 -A $a -d $d $traindata $wd/seql.model #> $wd/learn.log
./seql_mkmodel -i $wd/seql.model -o $wd/seql.model.bin -O $wd/seql.predictor #> $wd/mkmodel.log
./seql_classify -n 1 -v 0 -p $wd/seql.predictor -d $d $testdata $wd/seql.model.bin #> $wd/classify.log

#cat $wd/classify.log | grep Error
