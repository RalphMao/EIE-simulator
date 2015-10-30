
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
    print "===============CSC Formatting===================="
    ptr = [np.array([0], dtype = np.uint32)] * bank_num
    spm = [np.array([], dtype = np.uint32)] * bank_num
    ind= [np.array([], dtype = np.uint32)] * bank_num

    for idx in range(bank_num):
        print "Bank:", idx
        tmp = np.take(weights, range(idx, weights.shape[0], bank_num), axis=0)
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

        ptr[idx] = np.append(ptr[idx], ptr_tmp[1:])
        spm[idx] = np.append(spm[idx], spm_tmp[:ptr_tmp[-1]])
        ind[idx] = np.append(ind[idx], ind_tmp[:ptr_tmp[-1]])

    return ptr, spm, ind


caffe.set_mode_cpu()   

nets = ['lenet5', 'alexnet', 'vgg', 'lenet_300'];

for option in nets:
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
    net.save(caffemodel + '.quantize')
    layers = filter(lambda x: 'ip' in x or 'fc' in x, net.params.keys())

    for layer in layers:
        w = net.params[layer][0].data
        b = net.params[layer][1].data

        maxw = max(abs(w));
        w /= maxw;
        b /= maxw;

        np.copyto(net.params[layer][0].data, w)
        np.copyto(net.params[layer][1].data, b)

    net.save(caffemodel + '.quantize.normalize')

