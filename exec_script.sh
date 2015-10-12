TIMEFORMAT=%R
for i in {1..5}
do
	{ time sleep $i ; } 2>> time.log
done
