DATA=data/alexnet_fc7/matrix.dat
echo "Computing, batch size 4096"
./sparse_test $DATA 
./dense_test
