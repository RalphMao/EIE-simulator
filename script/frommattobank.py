
import sys, os
import numpy as np
import scipy.cluster.vq_maohz as scv

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



W = np.load('W.npy').astype('f')

act = np.zeros(W.shape[1], dtype = 'int16')
act[0] = 1
act[1] = 2
act[5] = 1

groundtruth = np.dot(W, act)
groundtruth = (groundtruth * 32).astype('int16')


codebook = np.arange(16).astype('f') / 32

W_codes, _ = scv.vq(W.flatten(), codebook)
W_codes = np.reshape(W_codes, W.shape)

ptr, spm, ind   = get_csc_single_nobias(W_codes, bank_num = 2, max_jump = 16)

data_dir = 'test_data'
os.system("rm -rf " + data_dir)
os.system("mkdir " + data_dir)
os.system("mkdir " + data_dir + '/ptr')
os.system("mkdir " + data_dir + '/spm')

for idx in range(2):
    with open("%s/ptr/ptr%d.dat"%(data_dir, idx), 'wb') as f:
        f.write('%d\n'%len(ptr[idx]))
        for number in ptr[idx]:
            f.write('{:016b} '.format(number))
    with open("%s/spm/weights%d.dat"%(data_dir, idx), 'w') as f:
        f.write('%d\n'%len(spm[idx]))
        for number in spm[idx]:
            f.write('{:04b} '.format(number))

    with open("%s/spm/index%d.dat"%(data_dir, idx), 'w') as f:
        f.write('%d\n'%len(ind[idx]))
        for number in ind[idx]:
            f.write(('{:0%db} '%(4)).format(number))

with open("%s/act.dat"%data_dir, 'w') as f:
    f.write('%d\n'%len(act))
    for number in act:
        f.write('{:016b} '.format(number))

with open("%s/groundtruth.dat"%(data_dir), 'w') as f:
    f.write('%d\n'%len(groundtruth))
    for number in groundtruth:
        f.write('{:016b} '.format(number))
