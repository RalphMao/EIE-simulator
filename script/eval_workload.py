
import sys,os
import pickle
import numpy as np
caffe_root = os.environ["CAFFE_ROOT"]
sys.path.insert(0, caffe_root + 'python')
os.chdir(caffe_root)
import caffe

class blob(object):
    def __init__(self, W):
        self.data = W.astype(np.float32)
class fakenet(object):
    def __init__(self, W):
        self.params = {}
        for key in W:
            self.params[key] = [blob(W[key].transpose())]
            self.params[key].append(blob(np.zeros(W[key].shape[1])))


layers = {'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8'],
          'neutalk':['We', 'Wd', 'WLSTM']}

PEs = 2 ** np.arange(0,11)

simulator_root = os.environ['SIMULATOR_PATH']
f_std = open(simulator_root + '/workloads_std.log', 'w')
f_mean = open(simulator_root + '/workloads_mean.log', 'w' )

for option in layers.keys():
    if option == 'lenet5':                                             
        prototxt = '3_prototxt_solver/lenet5/train_val.prototxt'       
        caffemodel = '4_model_checkpoint/lenet5/lenet5.caffemodel'     
    elif option == 'alexnet':                                          
        prototxt = '3_prototxt_solver/L2/train_val.prototxt'           
        caffemodel = '4_model_checkpoint/alexnet/alexnet9x.caffemodel' 
    elif option == 'vgg':
        prototxt = '3_prototxt_solver/vgg16/train_val.prototxt'     
        caffemodel = '4_model_checkpoint/vgg16/vgg16_13x.caffemodel'
    elif option == 'lenet_300':
        prototxt = '3_prototxt_solver/lenet_300_100/train_val.prototxt'
        caffemodel = '4_model_checkpoint/lenet_300_100/lenet300_100_9x.caffemodel'
    else:
        W_ = pickle.load(open('/home/maohz12/dnn_simulator/data_rnn/coco_1.6.pkl'))

    if option == 'neutalk':
        net = fakenet(W_)
    else:
        net = caffe.Net(prototxt, caffemodel, caffe.TEST)

    for layer in layers[option]:
        f_std.write(option + '_' + layer + ',')
        f_mean.write(option + '_' + layer + ',')

        weights = net.params[layer][0].data
        for bank_num in PEs:
            workloads = np.zeros((bank_num, weights.shape[1]))
            for idx in range(bank_num):
                tmp = np.take(weights, range(idx, weights.shape[0], bank_num), axis=0)
                workloads[idx] = np.sum(tmp != 0, axis = 0)
            f_std.write('%.4f, '%np.std(workloads))
            f_mean.write('%.4f, '%np.mean(workloads))
        f_std.write('\n')
        f_mean.write('\n')
        f_std.flush()
        f_mean.flush()

