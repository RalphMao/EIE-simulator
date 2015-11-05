all:
	gcc -I/usr/local/cuda/include sparse.c -g -std=c99 -o sparse_test -L/usr/local/cuda/lib64 -lcudart -lcusparse
	gcc -I/usr/local/cuda/include dense.c -g -std=c99 -o dense_test -L/usr/local/cuda/lib64 -lcudart -lcublas
