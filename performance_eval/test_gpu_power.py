
import subprocess
import os
import numpy as np
import time
import re

dirs = ['alexnet_fc6',
'alexnet_fc7',
'alexnet_fc8', 
'vgg_fc6',   
'vgg_fc7',   
'vgg_fc8',  
'lenet_300_ip1',  
'lenet_300_ip2',  
'lenet_300_ip3',  
'lenet5_ip1',  
'lenet5_ip2',  
'neutalk_We',  
'neutalk_Wd',  
'neutalk_WLSTM']  

programs = ['dense_mv',
'dense_mm',
'sparse_mv',
'sparse_mm']

def parse_nvi(file_str, idx):
    lines = open(file_str).read()
    match = re.findall(r'[0-9]+W\ \/\ [0-9]+W', lines)
    power = int(match[idx].split('W')[0])
    return power
    

dirs = map(lambda x: 'data/' + x, dirs)
gpu_id = 0

power = np.zeros(10)
f = open('log/log_power_gpu','w')
f.write(', dense_mv, dense_mm, sparse_mv, sparse_mm, \n')

for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    m,n = np.fromfile(file_t, np.int32, 2)
    f.write(dir_t + ', ')
    for program in programs:
        print file_t, program
        if 'sparse' in program:
            p = subprocess.Popen(['./' + program, file_t])
        else:
            p = subprocess.Popen(['./' + program, str(m), str(n)])

        for times in range(10):
            time.sleep(1)
            os.system('nvidia-smi > tmp_gpu_log')
            power[times] = parse_nvi('tmp_gpu_log', gpu_id)
        p.kill()
        f.write(str(max(power)) + ', ')
        f.flush()
        time.sleep(10)

    f.write('\n')
f.close()



