
import sys
import os
import numpy as np
import scipy.cluster.vq_maohz as scv
from jinja2 import Template

# os.system("cd $CAFFE_ROOT")
caffe_root = os.environ["CAFFE_ROOT"]
sys.path.insert(0, caffe_root + 'python')
os.chdir(caffe_root)
import caffe

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--net', type = str, default = 'alexnet')
parser.add_argument('--layer', type = str, default = 'fc6')
parser.add_argument('--bank-num', type = int, default = 8)
parser.add_argument('--c-simulation', action = 'store_true')


def kmeans(net, layers, num_c=16, initials=None, snapshot=False, alpha=0.0):            
    codebook = {}                                                                           
    if type(num_c) == type(1):                                                              
        num_c = [num_c] * len(layers)                                                       
    else:                                                                                   
        assert len(num_c) == len(layers)                                                    
                                                                                            
    print "==============Perform K-means============="                                      
    for idx, layer in enumerate(layers):                                                    
        print "Eval layer:", layer                                                          
        W = net.params[layer][0].data.flatten()                                             
        W = W[np.where(W != 0)]                                                             
        if initials is None:  # Default: uniform sample                                     
            min_W = np.min(W)                                                               
            max_W = np.max(W)                                                               
            initial_uni = np.linspace(min_W, max_W, num_c[idx] - 1)                         
                                                                                            
            codebook[layer], _= scv.kmeans(W, initial_uni, compress=False, alpha=alpha) 
                                                                                            
        elif type(initials) == type(np.array([])):                                          
            codebook[layer], _ = scv.kmeans(W, initials)                                    
        elif initials == 'random':                                                          
            codebook[layer], _ = scv.kmeans(W, num_c[idx]-1)                                
                                                                                            
        codebook[layer] = np.append(0, codebook[layer])                                   
        print "codebook size:", len(codebook[layer])                                        

    return codebook

def quantize_net(net, codebook):
    layers = codebook.keys()
    print "================Perform quantization=============="
    for layer in layers:
        print "Quantize layer:", layer
        W = net.params[layer][0].data
        codes, _ = scv.vq(W.flatten(), codebook[layer])
        W_q = np.reshape(codebook[layer][codes], W.shape)
        np.copyto(net.params[layer][0].data, W_q)


def get_codes(net, codebook):
    layers = codebook.keys()                                          
    codes_W = {}
    codes_b = {}
    print "================Perform quantization=============="        
    for layer in layers:                                              
        print "Quantize layer:", layer                                
        W = net.params[layer][0].data                                 
        b = net.params[layer][1].data   
        codes, _ = scv.vq(W.flatten(), codebook[layer])           
        codes = np.reshape(codes, W.shape)             
        codes_W[layer] = np.array(codes, dtype=np.uint32)
        W_q = np.reshape(codebook[layer][codes], W.shape)
        np.copyto(net.params[layer][0].data, W_q)

        codes, _ = scv.vq(b.flatten(), codebook[layer])           
        codes = np.reshape(codes, b.shape)             
        codes_b[layer] = np.array(codes, dtype=np.uint32)
        b_q = np.reshape(codebook[layer][codes], b.shape)
        np.copyto(net.params[layer][1].data, b_q)

    return codes_W, codes_b

def get_csc_single_nobias(weights, bank_num = 64, max_jump = 16):
    ptr = [np.array([0], dtype = np.uint32)] * bank_num
    spm = [np.array([], dtype = np.uint32)] * bank_num
    ind= [np.array([], dtype = np.uint32)] * bank_num

    for idx in range(bank_num):
        tmp = np.take(weights, range(idx, weights.shape[0], bank_num), axis=0)
        # bank_weights[idx,:tmp.shape[0]] = tmp
        # tmp_id = np.where(tmp == 0)
        # x_id = tmp_id[0]
        # y_id = tmp_id[1]
        ptr_tmp = np.zeros(tmp.shape[1]+1, dtype = np.uint32) # take bias into consideration
        
        # weights    
        spm_tmp = np.zeros(weights.size, dtype = np.uint32) # large enough
        ind_tmp = np.ones(weights.size, dtype = np.uint32) * (max_jump-1)# large enough
        for col in range(tmp.shape[1]):
            loc = np.where(tmp[:,col] != 0)[0]
            if len(loc) > 0:
                distance_loc = np.append(loc[0], np.diff(loc)-1)  #jump 1 encode to 0
                zeros = distance_loc/max_jump
                idx_vec = np.cumsum(zeros+1)-1  #add the element itself. first one need -1
                ptr_tmp[col+1] = idx_vec[-1]+1 + ptr_tmp[col]             #ptr
                spm_tmp[ptr_tmp[col] + idx_vec] = tmp[loc, col]           #code
                ind_tmp[ptr_tmp[col] + idx_vec] = distance_loc % max_jump #index
            else:
                ptr_tmp[col+1] = ptr_tmp[col]

        ptr[idx] = np.append(ptr[idx], ptr_tmp)
        spm[idx] = np.append(spm[idx], spm_tmp[:ptr_tmp[-1]])
        ind[idx] = np.append(ind[idx], ind_tmp[:ptr_tmp[-1]])

    return ptr, spm, ind


caffe.set_mode_cpu()   
options = argparse.parse_args()
option = options.net
layer = options.layer
bank_num = options.bank_num
C_simulation = options.c_simulation

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
    print "Unknown net type:", option
    sys.exit(1)

net = caffe.Net(prototxt, caffemodel, caffe.TEST)

codebook = kmeans(net, [layer])
codes_W, codes_b = get_codes(net, codebook)
weights = codes_W[layer]

ptr, spm, ind = get_csc_single_nobias(weights, bank_num = bank_num)

simulator_root = os.environ['SIMULATOR_PATH']
data_dir = simulator_root + '/data/%s_%s_%d'%(net, layer, bank_num)

os.system("rm -rf " + data_dir)
os.system("mkdir " + data_dir)
os.system("mkdir " + data_dir + '/ptr')
os.system("mkdir " + data_dir + '/spm')

max_memsize = 0
mem_a = [0] * bank_num
for idx in range(bank_num):
    with open("%s/ptr/ptr%d.dat"%(data_dir, idx), 'wb') as f:
        ptr[idx].tofile(f) # Ptr is stored by 32-bit 

    with open("%s/spm/spm%d.dat"%(data_dir, idx), 'wb') as f:
        # Spm and Ind are stored with 8-bit or with 32-bit * 2
        # Spm is stored in low 4-bit and Ind in high 4-bit
        if C_simulation:
            mem = np.transpose(np.array([spm[idx],ind[idx]])).flatten()
        else:
            mem = spm[idx].astype(np.uint8) + ind[idx].astype(np.uint8) << 4

        if (mem.size > max_memsize):
            max_memsize = mem.size
        mem_a[idx] = mem
        mem.tofile(f)

with open("%s/data/arithm.dat"%simulator_root, 'wb') as f:
    if C_simulation:
        codebook_t = np.array(codebook[layer], dtype=np.float32)
    else: # 16-bit fixed-point, range in [-1,1]
        codebook_t = np.array(codebook[layer], dtype=np.float64)
        codebook_t = np.array(codebook_t / np.max(abs(codebook_t)) * (2 ** 15), dtype = np.int16)

    codebook_t.tofile(f)
    
# Render config header file
template = r'''
// Auto generated by script/layer_dump.py

#ifndef PARAMS
#define PARAMS

// Config harware
const int NUM_PE = {{ bank_num }};
const int ACTRW_maxcapacity = {{ max_size }};
const int NZFETCH_buffersize = {{ buffer_size }};  
const int PTRVEC_num_lines = {{ ptr_lines }};  
const int SPMAT_unit_line   =  {{ spm_unitsize }};  // Nzeros per line
const int SPMAT_num_lines   =  {{ spm_lines }}; 
const int SPMAT_index_bits  =  4;  
const int SPMAT_weights_bits=  4;  
const int ARITHM_codebooksize = 16;

// Config input data
const int ACT_length = {{act_length}};
#endif
'''
###################################################
# Configuration
spm_unitsize = 16  # 16 code + 16 index
buffer_size = 4

##################################################
batch_size = net.blobs['conv1'].data.shape[0]
for i in range(idx / batch_size+1):
    net.forward()

one_act = 1 # For debug

if option == 'lenet5':
    if one_act:
        act = np.array([2.0, 0.0, 1.0], dtype=np.float32)
        ground_truth = 2.0 * net.params['ip1'][0].data[:,0] + 1.0 * net.params['ip1'][0].data[:,2]
    else:
        act = net.blobs['pool2'].data[idx % batch_size]
        ground_truth = net.blobs['ip1'].data[idx % batch_size]
else:
    act = net.blobs['fc6'].data[idx % batch_size]
    ground_truth = net.blobs['fc7'].data[idx % batch_size]

if option == "lenet5":
    max_inputsize = 1024
else:
    max_inputsize = 4096
act_length = act.size

with open("%s/act.dat"%data_dir, 'wb') as f:
    if C_simulation:
        act.tofile(f)
    else:
        act.astype(np.int16).tofile(f)

with open("%s/groundtruth.dat"%data_dir, 'wb') as f:
    ground_truth.tofile(f)
 

if C_simulation:
    jtem = Template(template)
    config_file = jtem.render(bank_num = bank_num, ptr_lines = ptr[0].size, 
        spm_unitsize = spm_unitsize, spm_lines = (max_memsize - 1) / spm_unitsize / 2 + 1, 
        max_size = max_inputsize, act_length = act_length, buffer_size = buffer_size)

    with open("%s/src/params.h"%(simulator_root), 'w') as f:
        f.write(config_file)

