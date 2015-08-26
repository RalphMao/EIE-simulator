
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
    spm = [np.array([0], dtype = np.int32)] * bank_num

    for layer in layers:
        weights = codes_W[layer]
        bias = codes_b[layer]

        for idx in range(bank_num):
            tmp = np.take(weights, range(idx, weights.shape[0], bank_num), axis=0)
            # bank_weights[idx,:tmp.shape[0]] = tmp
            # tmp_id = np.where(tmp == 0)
            # x_id = tmp_id[0]
            # y_id = tmp_id[1]
            num_nonzero_col = np.sum(tmp != 0, axis = 0)
            ptr_tmp = np.zeros(((tmp.shape[1]-1)/bank_num+1)*bank_num+1, dtype = np.int32) # take bias into consideration
            spm_tmp = np.zeros(weights.size, dtype = np.int32) # large enough
            for col in range(tmp.shape[1]):
                loc = np.where(tmp[col] != 0)
                distance_loc = np.append(loc[0], np.diff(loc))
                zeros = distance_loc / max_jump
                idx = np.cumsum(zeros+1)-1
                ptr_tmp[col+1] = num_nonzero_col[col] + idx[-1]+1 + ptr_tmp[col]
                spm_tmp[ptr_tmp[col] + idx] = tmp[col][loc]

            
            ptr_tmp[tmp.shape[1]:-1] = ptr_tmp[tmp.shape[1]]

            loc = np.where(bias != 0)
            distance_loc = np.append(loc[0], np.diff(loc))
            zeros = distance_loc / max_jump
            idx = np.cumsum(zeros+1)-1
            ptr_tmp[-1] = num_nonzero_col[col] + idx[-1]+1 + ptr_tmp[-2]
            spm_tmp[ptr_tmp[-2] + idx] = bias[loc]

            ptr[idx] = np.append(ptr[idx], ptr_tmp[1:] + ptr[idx][-1])
            spm[idx] = np.append(spm[idx], spm_tmp[:ptr_tmp[col+1]])

    return ptr, spm
                


    

net = caffe.Net(prototxt, caffemodel, caffe.TEST)
layers = ['ip2']
codebook = kmeans(net, layers)
codes_W, codes_b = get_codes(net, codebook)
ptr, spm = get_csc(codes_W, codes_b)

