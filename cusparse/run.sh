echo $1 >> log 
./sparse_test $1 | grep cusparse_time >> log
