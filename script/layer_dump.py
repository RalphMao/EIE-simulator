
import sys
import os
import numpy as np
import scipy.cluster.vq_maohz as scv

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
        codes_W[layer] = np.array(codes, dtype=np.int32)
        codes, _ = scv.vq(b.flatten(), codebook[layer])           
        codes = np.reshape(codes, b.shape)             
        codes_b[layer] = np.array(codes, dtype=np.int32)

    return codes_W, codes_b

def get_csc(codes_W, codes_b, bank_num=64, max_jump = 16):
    layers = codes_W.keys()
    ptr = [np.array([0], dtype = np.int32)] * bank_num
    spm = [np.array([], dtype = np.int32)] * bank_num
    ind= [np.array([], dtype = np.int32)] * bank_num
    layer_shift = np.zeros(len(layers) + 1, dtype=np.int32)

    has_bias = [False] * bank_num

    for layer_id, layer in enumerate(layers):
        weights = codes_W[layer]
        bias = codes_b[layer]

        for idx in range(bank_num):
            tmp = np.take(weights, range(idx, weights.shape[0], bank_num), axis=0)
            tmp_bias = np.take(bias, range(idx, bias.shape[0], bank_num), axis=0)
            # bank_weights[idx,:tmp.shape[0]] = tmp
            # tmp_id = np.where(tmp == 0)
            # x_id = tmp_id[0]
            # y_id = tmp_id[1]
            if not has_bias[idx]:
                ptr_tmp = np.zeros(((tmp.shape[1]-1)/bank_num+1)*bank_num+2, dtype = np.int32) # take bias into consideration
                has_bias[idx] = True
            else:
                ptr_tmp = np.zeros(((tmp.shape[1])/bank_num+1)*bank_num+2, dtype = np.int32) # take bias into consideration
                
            spm_tmp = np.zeros(weights.size, dtype = np.int32) # large enough
            ind_tmp = np.ones(weights.size, dtype = np.int32) * (max_jump-1)# large enough
            for col in range(tmp.shape[1]):
                loc = np.where(tmp[:,col] != 0)[0]
                if len(loc) > 0:
                    distance_loc = np.append(loc[0], np.diff(loc))
                    zeros = distance_loc / max_jump
                    idx_vec = np.cumsum(zeros+1)-1
                    ptr_tmp[col+1] = idx_vec[-1]+1 + ptr_tmp[col]
                    spm_tmp[ptr_tmp[col] + idx_vec] = tmp[loc, col]
                    ind_tmp[ptr_tmp[col] + idx_vec] = distance_loc % 16
                else:
                    ptr_tmp[col+1] = ptr_tmp[col]
            
            ptr_tmp[tmp.shape[1]:-1] = ptr_tmp[tmp.shape[1]]

            loc = np.where(tmp_bias != 0)[0]
            if len(loc) > 0:
                distance_loc = np.append(loc[0], np.diff(loc))
                zeros = distance_loc / max_jump
                idx_vec = np.cumsum(zeros+1)-1
                ptr_tmp[-1] = idx_vec[-1]+1 + ptr_tmp[-2]
                spm_tmp[ptr_tmp[-2] + idx_vec] = tmp_bias[loc]
                ind_tmp[ptr_tmp[-2] + idx_vec] = distance_loc % 16
            else:
                ptr_tmp[-1] = ptr_tmp[-2]

            ptr[idx] = np.append(ptr[idx], ptr_tmp[1:] + ptr[idx][-1])
            spm[idx] = np.append(spm[idx], spm_tmp[:ptr_tmp[-1]])
            ind[idx] = np.append(ind[idx], ind_tmp[:ptr_tmp[-1]])

            print len(ptr[idx])
        layer_shift[layer_id+1] = ptr[0].size - 1


    return ptr, spm, ind, layer_shift[:-1]


net = caffe.Net(prototxt, caffemodel, caffe.TEST)
if option == 'lenet5':
    layers = ['ip1']
    bank_num = 4
else:
    layers = ['fc6']
    bank_num = 32

codebook = kmeans(net, layers)
codes_W, codes_b = get_codes(net, codebook)
ptr, spm, ind, layer_shift= get_csc(codes_W, codes_b, bank_num = bank_num)

simulator_root = os.environ['SIMULATOR_PATH']
os.system("rm -rf %s/data/ptr"%simulator_root)
os.system("rm -rf %s/data/spm"%simulator_root)
os.system("mkdir %s/data/ptr"%simulator_root)
os.system("mkdir %s/data/spm"%simulator_root)

for idx in range(bank_num):
    with open("%s/data/ptr/ptr%d.dat"%(simulator_root, idx), 'wb') as f:
        ptr[idx].tofile(f)

    with open("%s/data/spm/spm%d.dat"%(simulator_root, idx), 'wb') as f:
        mem = np.transpose(np.array([spm[idx],ind[idx]])).flatten()
        mem.tofile(f)

with open("%s/data/arithm.dat"%simulator_root, 'wb') as f:
    for key in codebook:
        codebook[key].tofile(f)
    

