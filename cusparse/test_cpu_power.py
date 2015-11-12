import re
import os
import subprocess
from multiprocessing import Process
import numpy as np
import scipy.sparse as sp
import time
dirs = ['alexnet_fc6',
'alexnet_fc7',
'alexnet_fc8', 
'vgg_fc6',   
'vgg_fc7',   
'vgg_fc8',  
'lenet_300_ip1',  
'lenet_300_ip2',  
'lenet_300_ip3',  
'lenet5_ip1',  
'lenet5_ip2',  
'neutalk_We',  
'neutalk_Wd',  
'neutalk_WLSTM']  
dirs = map(lambda x: 'data/' + x, dirs)

def load_fromfile(filename):
    f = open(filename)
    m, n, nnz = np.fromfile(f, 'int32', 3)
    data = np.fromfile(f, 'f', nnz)
    indptr = np.fromfile(f, 'int32', m+1)
    indices =np.fromfile(f, 'int32', nnz)
    
    mat = sp.csr_matrix((data, indices, indptr), shape=[m,n])
    return mat


def sparse_mul(matrix, vectors, batchsize = 1):
    n_v = vectors.shape[1]
    while True:
        for i in range((n_v-1)/batchsize+1):
            res = matrix * vectors[:, batchsize * i:batchsize* i + batchsize]

def sparse_mul_sp(matrix, vectors):
    while True:
        for vector in vectors:
            res = matrix * vector

def dense_mul(matrix, vectors, batchsize = 1):
    n_v = vectors.shape[1]
    res = np.zeros((matrix.shape[0], batchsize), dtype = vectors.dtype)
    while True:
        for i in range((n_v-1)/batchsize+1):
            vector = vectors[:, batchsize * i:batchsize* i + batchsize]
            a = np.dot(matrix,vector)

def parse_pcm(filename):
    lines = open(filename).read()
    pick = re.findall(r'Watts:\ [0-9]+\.[0-9]+', lines)
    power = map(lambda x:float(x.split()[-1]), pick)
    return power

def monitor():
    os.system('/home/maohz12/IntelPerformanceCounterMonitor-V2.9/pcm-power.x > tmp_log 2> /dev/null')

def test_time(func, *args):
    p = Process(target = func, args = args)
    p2 = Process(target = monitor)
    p.start()
    p2.start()
    time.sleep(10)
    p.terminate()
    p2.terminate()
    power = parse_pcm('tmp_log')
    time.sleep(5)
    return max(power)


f = open('power_cpu_log','w')
power = [0] * 6
for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    print file_t
    time_t = [0] * 6

    csr_mat = load_fromfile(file_t)
    mat = csr_mat.todense()
    vector_num = 256
    if 'neutalk' in dir_t:
        acts = np.ones((mat.shape[1], vector_num), dtype = 'f')
    else:
        acts = np.fromfile(dir_t + '/act.dat', dtype='f')
        n_v = acts.size / mat.shape[1]
        assert acts.size % mat.shape[1] == 0
        acts = np.repeat(acts, vector_num/n_v+1)
        acts = acts.reshape((mat.shape[1], acts.size/mat.shape[1]))
        acts = acts[:, :vector_num]

    spv = map(lambda x:sp.csr_matrix(acts[:,x:x+1]), range(acts.shape[1]))
    spm = map(lambda x:sp.csr_matrix(acts[:,64 * x:64*x + 64]), range(acts.shape[1]/64))

    power[0] = test_time(dense_mul, mat, acts)
    power[1] = test_time(dense_mul, mat, acts, 64)
    power[2] = test_time(sparse_mul, csr_mat, acts)
    power[3] = test_time(sparse_mul, csr_mat, acts, 64)
    power[4] = test_time(sparse_mul_sp, csr_mat, spv) 
    power[5] = test_time(sparse_mul_sp, csr_mat, spm) 

    type = dir_t.split('/')[-1]
    f.write('%s, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n'%((type,) +tuple(power)))
    f.flush()
f.close()
