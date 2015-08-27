
import sys
import os
import numpy as np

# os.system("cd $CAFFE_ROOT")
caffe_root = os.environ["CAFFE_ROOT"]
sys.path.insert(0, caffe_root + 'python')
os.chdir(caffe_root)
import caffe

caffe.set_mode_gpu()                                               
caffe.set_device(0)                                                
option = 'lenet5'
if option == 'lenet5':                                             
    prototxt = '3_prototxt_solver/lenet5/train_val.prototxt'       
    caffemodel = '4_model_checkpoint/lenet5/lenet5.caffemodel'     
elif option == 'alexnet':                                          
    prototxt = '3_prototxt_solver/L2/train_val.prototxt'           
    caffemodel = '4_model_checkpoint/alexnet/alexnet9x.caffemodel' 

if len(sys.argv) > 1:
    idx = int(sys.argv[1])
else:
    idx = 0

net = caffe.Net(prototxt, caffemodel, caffe.TEST)
batch_size = net.blobs['conv1'].data.shape[0]
for i in range(idx / batch_size+1):
    net.forward()

if option == 'lenet5':
    act = net.blobs['pool2'].data[idx % batch_size]
    ground_truth = net.blobs['ip1'].data[idx % batch_size]
else:
    act = net.blobs['pool5'].data[idx % batch_size]
    ground_truth = net.blobs['fc6'].data[idx % batch_size]


simulator_root = os.environ['SIMULATOR_PATH']

with open("%s/data/act.dat"%simulator_root, 'wb') as f:
    act.tofile(f)

with open("%s/data/groundtruth.dat"%simulator_root, 'wb') as f:
    ground_truth.tofile(f)
    
