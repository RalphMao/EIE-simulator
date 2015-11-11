

import os,sys
import numpy as np
import scipy.sparse as sp

def dump2file(mat, filename):
    assert mat.dtype == np.float32
    csr_m = sp.csr_matrix(mat)

    f = open(filename, 'wb')
    nnz = csr_m.getnnz()
    m,n = mat.shape
    np.array([m,n,nnz], dtype = 'int32').tofile(f)
    csr_m.data.tofile(f)
    csr_m.indptr.tofile(f)
    csr_m.indices.tofile(f)
    f.close()

import pickle
a = pickle.load(open('/home/maohz12/dnn_simulator/data_rnn/coco_1.6.pkl'))

for key in a:
    os.mkdir('/home/maohz12/dnn_simulator/cusparse/data/neutalk_' + key)
    dump2file(a[key].transpose().astype('f'), 
        '/home/maohz12/dnn_simulator/cusparse/data/neutalk_' + key + '/matrix.dat')
