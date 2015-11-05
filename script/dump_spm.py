
import os,sys
import numpy as np
import scipy.sparse as sp
caffe_root = os.environ["CAFFE_ROOT"]
sys.path.insert(0, caffe_root + 'python')
os.chdir(caffe_root)
import caffe


def dump2file(mat, filename):
    csr_m = sp.csr_matrix(mat)

    f = open(filename, 'wb')
    nnz = csr_m.getnnz()
    m,n = mat.shape
    np.array([m,n,nnz], dtype = 'int32').tofile(f)
    csr_m.indptr.tofile(f)
    mat.data.tofile(f)
    f.close()


layers = {'lenet5':['ip1', 'ip2'],
          'lenet_300':['ip1', 'ip2', 'ip3'],
          'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

nets = layers.keys()
for net in nets:
    if net == 'lenet5':                                             
        prototxt = '3_prototxt_solver/lenet5/train_val.prototxt'       
        caffemodel = '4_model_checkpoint/lenet5/lenet5.caffemodel'     
    elif net == 'alexnet':                                          
        prototxt = '3_prototxt_solver/L2/train_val.prototxt'           
        caffemodel = '4_model_checkpoint/alexnet/alexnet9x.caffemodel' 
    elif net == 'vgg':
        prototxt = '3_prototxt_solver/vgg16/train_val.prototxt'     
        caffemodel = '4_model_checkpoint/vgg16/vgg16_13x.caffemodel'
    elif net == 'lenet_300':
        prototxt = '3_prototxt_solver/lenet_300_100/train_val.prototxt'
        caffemodel = '4_model_checkpoint/lenet_300_100/lenet300_100_9x.caffemodel'
    net = caffe.Net(prototxt, caffemodel, caffe.TEST)
    net.forward()
    for layer in layers[net]:
        data_dir = 'data/%s_%s'%(net, layer)
        os.system('mkdir ' + data_dir)
        mat = net.params[layer][0].data
        dump2file(mat, data_dir + '/matrix.dat')



