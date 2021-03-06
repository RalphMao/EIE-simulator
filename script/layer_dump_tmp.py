
import pickle
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
parser.add_argument('--ind-bits', type = int, default = 4)
parser.add_argument('--c-simulation', action = 'store_true')
parser.add_argument('--binary', action = 'store_true')


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
options = parser.parse_args()
option = options.net
layer = options.layer
bank_num = options.bank_num
C_simulation = options.c_simulation
binary = options.binary
max_jump = 2 **  options.ind_bits

simulator_root = os.environ['SIMULATOR_PATH']
data_dir = simulator_root + '/converted_data/%s_%s_%d'%(option, layer, bank_num)
if options.ind_bits != 4:
    data_dir += '_%d'%options.ind_bits

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

'''
codebook = kmeans(net, [layer])
codes_W, codes_b = get_codes(net, codebook)
'''
codebook, codes_W, codes_b = pickle.load(open(caffemodel + '.codes'))
weights = codes_W[layer]

ptr, spm, ind = get_csc_single_nobias(weights, bank_num = bank_num, max_jump = max_jump)


os.system("rm -rf " + data_dir)
os.system("mkdir " + data_dir)
os.system("mkdir " + data_dir + '/ptr')
os.system("mkdir " + data_dir + '/spm')

max_memsize = 0
mem_a = [0] * bank_num
total_nonzeros = 0
for idx in range(bank_num):

    '''
    with open("%s/spm/spm%d.dat"%(data_dir, idx), 'wb') as f:
        # Spm and Ind are stored with 8-bit or with 32-bit * 2
        # Spm is stored in low 4-bit and Ind in high 4-bit
        if C_simulation:
            mem = np.transpose(np.array([spm[idx],ind[idx]])).flatten()
        else:
            mem = spm[idx].astype(np.uint8) + (ind[idx].astype(np.uint8) << 4)

        if (mem.size > max_memsize):
            max_memsize = mem.size
        total_nonzeros += np.count_nonzero(spm[idx])
        mem_a[idx] = mem.size
        mem.tofile(f)
    '''
    if C_simulation:
        with open("%s/ptr/ptr%d.dat"%(data_dir, idx), 'wb') as f:
            ptr[idx].tofile(f) # Ptr is stored by 32-bit 
        with open("%s/spm/spm%d.dat"%(data_dir, idx), 'w') as f:
            mem = np.transpose(np.array([spm[idx],ind[idx]])).flatten()
            mem.tofile(f)

    else:
        f = open("%s/ptr_even_%d.txt"%(data_dir, idx), 'wb') 
        g = open("%s/ptr_odd_%d.txt"%(data_dir, idx), 'wb') 
        for idt in range(ptr[idx].size):
            if idt % 2 == 1:
                g.write('{:016b}\n'.format(ptr[idx][idt]))
            else:
                f.write('{:016b}\n'.format(ptr[idx][idt]))
        f.close()
        g.close()

        with open("%s/spmat%d.txt"%(data_dir, idx), 'w') as f:
            mem = np.transpose(np.array([spm[idx],ind[idx]])).flatten()
            if binary:
                mem.astype(np.uint8).tofile(f)
            else:
                # f.write('%d\n'%len(spm[idx]))
                for idt in range(len(spm[idx])):
                    upper = (idt / 16) * 16 + 15
                    if upper >= len(spm[idx]):
                        upper = len(spm[idx]) - 1
                    ids = upper - idt % 16
                    f.write('{:04b}'.format(spm[idx][ids]))
                    f.write('{:04b}'.format(ind[idx][ids]))
                    if idt % 16 == 15:
                        f.write('\n')

    if (spm[idx].size > max_memsize):
        max_memsize = spm[idx].size
    total_nonzeros += np.count_nonzero(spm[idx])
    mem_a[idx] = spm[idx].size


total_memsize = sum(mem_a)
print "Max memory size of one bank:", max_memsize
print "Total memory size :", total_memsize
print "Total Nonzeros:",total_nonzeros 
