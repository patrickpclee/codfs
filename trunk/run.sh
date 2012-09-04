
for((i=0; i < $1; i++));
do
	echo "run number: $i";
	echo "UPLOADING..."
	./CLIENT clientconfig.xml upload testfile_large;
	
	echo "DOWNLOADING..."
	./CLIENT clientconfig.xml download $2 "test_$i";
done
