declare -A dtlength=( ["Coffee"]=286 ["Earthquakes"]=512 ["ECG200"]=96 ["ECGFiveDays"]=136 ["FordA"]=500 ["FordB"]=500 ["Gun_Point"]=150 ["Lighting2"]=637 ["MoteStrain"]=84 ["Passgraph"]=364 ["SonyAIBORobotSurface"]=70 ["SonyAIBORobotSurfaceII"]=65 ["TwoLeadECG"]=82 ["wafer"]=152 ["yoga"]=426 ["HandOutlines"]=2709 ["ArrowHead"]=495 ["Beef"]=470 ["CBF"]=128 ["ChlorineConcentration"]=166 ["DiatomSizeReduction"]=345 ["ElectricDevices"]=96 ["FaceAll"]=131 ["FaceFour"]=350 ["FacesUCR"]=131 ["Haptics"]=1092 ["Lighting7"]=319 ["MALLAT"]=1024 ["MedicalImages"]=99 ["OliveOil"]=570 ["OSULeaf"]=427 ["StarLightCurves"]=1024 ["SwedishLeaf"]=129 ["Symbols"]=398 ["Trace"]=275 ["Two_Patterns"]=128 ["uWaveGestureLibrary_X"]=315 ["uWaveGestureLibrary_Y"]=315 ["uWaveGestureLibrary_Z"]=315 ["FISH"]=463 ["InlineSkate"]=1882 ["CinC_ECG_torso"]=1639 ["Cricket_X"]=300 ["Cricket_Y"]=300 ["Cricket_Z"]=300)

bindata=( Coffee Earthquakes ECG200 ECGFiveDays FordA FordB Gun_Point Lighting2 MoteStrain Passgraph SonyAIBORobotSurface SonyAIBORobotSurfaceII TwoLeadECG wafer yoga )
multidata=( ArrowHead Beef CBF ChlorineConcentration CinC_ECG_torso DiatomSizeReduction ElectricDevices FaceAll FaceFour FacesUCR FISH Haptics InlineSkate Lighting7 MALLAT MedicalImages OliveOil OSULeaf StarLightCurves SwedishLeaf Symbols Trace Two_Patterns uWaveGestureLibrary_X uWaveGestureLibrary_Y uWaveGestureLibrary_Z )

window=32
word=10
alphabet=4
dist=1
step=1

charseql() {
	train=data/${1}_TRAIN
	test=data/${1}_TEST
	#test=data/${1}_TEST_shifted
	wd=charseql/$1	
	#w=${dtlength[$1]}
	w=16
	a=8

	mkdir -p $wd
	rm $wd/*

	./sax_convert -n 1 -w $w -a $a -i $train -o $wd/sax.train
	./sax_convert -n 1 -w $w -a $a -i $test -o $wd/sax.test
	./seql_learn -n 1 -v 1 -A $a -d 0 $wd/sax.train $wd/seql.model > $wd/learn.log
	./seql_mkmodel -i $wd/seql.model -o $wd/seql.model.bin -O $wd/seql.predictor > $wd/mkmodel.log
	./seql_classify -n 1 -v 0 -p $wd/seql.predictor -d 0 $wd/sax.test $wd/seql.model.bin > $wd/classify.log

	cat $wd/classify.log | grep Error
}



binseql() {
	train=data/${1}_TRAIN
	test=data/${1}_TEST	
	wd=vseql/$1
	N=$2
	w=$3
	a=$4
	d=$5

	mkdir -p $wd
	rm $wd/*

	#./sax_convert -n 0 -s 1 -N $N -w $w -a $a -t $step -i $train -o $wd/sax.train
	#./sax_convert -n 0 -s 1 -N $N -w $w -a $a -t $step -i $test -o $wd/sax.test
	/usr/bin/time -f "\t%e" ./sax_convert -n 0 -s 1 -N $N -w $w -a $a -t $step -i $train -o $wd/sax.train -I $test -O $wd/sax.test
	/usr/bin/time -f "\t%e" ./seql_learn -n 1 -v 1 -A $a -d $d $wd/sax.train $wd/seql.model > $wd/learn.log
	./seql_mkmodel -i $wd/seql.model -o $wd/seql.model.bin -O $wd/seql.predictor > $wd/mkmodel.log
	
	str=$(./seql_classify_tune_threshold_min_errors -n 1 -v 0 $wd/sax.train $wd/seql.predictor | grep Best)
	pattern='[0-9]+\.[0-9]+'
	[[ $str =~ $pattern ]]
	threshold=${BASH_REMATCH[0]}
	echo "Threshold: $threshold"


	/usr/bin/time -f "\t%e" ./seql_classify -t $threshold -n 1 -v 0 -p $wd/seql.predictor -d $d $wd/sax.test $wd/seql.model.bin > $wd/classify.log

	cat $wd/classify.log | grep Error
}

freseql() {
	train=data/${1}_TRAIN
	test=data/${1}_TEST
	wd=freseql/$1
	N=$2
	w=$3
	a=$4
	

	mkdir -p $wd
	rm $wd/*

	./sax_convert -n 0 -s 1 -N $N -w $w -a $a -i $train -o $wd/sax.train
	./sax_convert -n 0 -s 1 -N $N -w $w -a $a -i $test -o $wd/sax.test
	./seql_learn -n 1 -v 2 $wd/sax.train $wd/seql.model > $wd/learn.log
	./seql_mkmodel -i $wd/seql.model -o $wd/seql.model.bin -O $wd/seql.predictor > $wd/mkmodel.log
	./seql_classify -n 1 -v 0 $wd/sax.test $wd/seql.model.bin > $wd/classify.log

	cat $wd/classify.log | grep Error
}

multiseql() {
	wd=multiseql/$1
	mkdir -p $wd
	#rm $wd/*
	#./seql_multiclass /home/thachln/PhD/dataset/MyCBF/$1/$1_TRAIN /home/thachln/PhD/dataset/MyCBF/$1/$1_TEST $wd $2 $3 $4
	#time ./seql_multiclass_test data/$1_TRAIN data/$1_TEST $wd $2 $3 $4
	rm $wd/*
	time ./seql_multiclass data/$1_TRAIN data/$1_TEST $wd $2 $3 $4
}

binclass() {
	echo "==================================="
	let ws="${dtlength[$1]} * 25 / 100"
	#ws=${dtlength[$1]}
	echo $1 $ws $word $alphabet $dist
	binseql $1 $ws $word $alphabet $dist
	echo "==================================="
}

multiclass() {
	echo "==================================="
	let ws="${dtlength[$1]} * 20 / 100"
	echo $1 $ws $word $alphabet $dist
	multiseql $1 $ws $word $alphabet
	echo "==================================="
}

#echo "==================================="
#let ws="128 * 20 / 100"
#echo My1920CBF $ws $word $alphabet $dist
#multiseql My1920CBF $ws $word $alphabet
#echo "==================================="



#binclass Coffee 
#binclass Earthquakes 
#binclass ECG200 
#binclass ECGFiveDays 
#binclass Gun_Point 
#binclass Lighting2

#multiclass Cricket_X
#multiclass Cricket_Y
#multiclass Cricket_Z

for data in "${bindata[@]}"
do
	#echo "==================================="
	echo $data
	binclass $data
	#echo "==================================="
done

#for data in "${multidata[@]}"
#do
	#echo "==================================="
#	echo $data
#	binclass $data
	#echo "==================================="
#done


#for data in "${multidata[@]}"
#do
	#echo "==================================="
#	echo $data
#	multiclass $data
	#echo "==================================="
#done

#binclass FordA 
#binclass FordB 
#binclass Gun_Point 
#binclass Lighting2
#binclass MoteStrain
#binclass Passgraph
#binclass SonyAIBORobotSurface
#binclass SonyAIBORobotSurfaceII
#binclass TwoLeadECG
#binclass wafer
#binclass yoga
#binclass HandOutlines

#multiclass ArrowHead
#multiclass Beef
#multiclass CBF
#multiclass CinC_ECG_torso
#multiclass ChlorineConcentration
#multiclass DiatomSizeReduction
#multiclass ElectricDevices
#multiclass FaceAll
#multiclass FaceFour
#multiclass FacesUCR
#multiclass FISH
#multiclass Haptics
#multiclass InlineSkate
#multiclass Lighting7
#multiclass MALLAT
#multiclass MedicalImages
#multiclass OliveOil
#multiclass OSULeaf
#multiclass StarLightCurves
#multiclass SwedishLeaf
#multiclass Symbols
#multiclass Trace
#multiclass Two_Patterns
#multiclass uWaveGestureLibrary_X
#multiclass uWaveGestureLibrary_Y
#multiclass uWaveGestureLibrary_Z


