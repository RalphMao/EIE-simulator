
MKLPATH=/opt/intel/mkl/
icc -std=c99 -L$MKLPATH/lib/intel64 -I$MKLPATH/include -o dense dense.c -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -dl -lm 
