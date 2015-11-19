
import os,sys
import numpy as np
import scipy.sparse as sp
caffe_root = os.environ["CAFFE_ROOT"]
sys.path.insert(0, caffe_root + 'python')
os.chdir(caffe_root)
import caffe


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


layers = {'lenet5':['ip1', 'ip2'],
          'lenet_300':['ip1', 'ip2', 'ip3'],
          'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

nets = layers.keys()
for net_name in nets:
    if net_name == 'lenet5':                                             
        prototxt = '3_prototxt_solver/lenet5/train_val.prototxt'       
        caffemodel = '4_model_checkpoint/lenet5/lenet5.caffemodel'     
    elif net_name == 'alexnet':                                          
        prototxt = '3_prototxt_solver/L2/train_val.prototxt'           
        caffemodel = '4_model_checkpoint/alexnet/alexnet9x.caffemodel' 
    elif net_name == 'vgg':
        prototxt = '3_prototxt_solver/vgg16/train_val.prototxt'     
        caffemodel = '4_model_checkpoint/vgg16/vgg16_13x.caffemodel'
    elif net_name == 'lenet_300':
        prototxt = '3_prototxt_solver/lenet_300_100/train_val.prototxt'
        caffemodel = '4_model_checkpoint/lenet_300_100/lenet300_100_9x.caffemodel'
    net = caffe.Net(prototxt, caffemodel, caffe.TEST)
    net.forward()
    for layer in layers[net_name]:
        data_dir = '/home/maohz12/dnn_simulator/cusparse/data/%s_%s'%(net_name, layer)
        os.system('mkdir ' + data_dir)
        mat = net.params[layer][0].data
        mat.tofile(data_dir + '/dense.dat')
        dump2file(mat, data_dir + '/matrix.dat')

        all_layers = net.blobs.keys()
        layer_previous = all_layers[all_layers.index(layer)-1]
        if len(net.blobs[layer_previous].data.shape) == 1: # In case of lenet300-100
            layer_previous = 'data'
        act = net.blobs[layer_previous].data.flatten()
        act_relu = (act + abs(act)) / 2

        act_relu.tofile(data_dir + '/act.dat')
        


