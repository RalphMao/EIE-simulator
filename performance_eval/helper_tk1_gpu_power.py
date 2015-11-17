
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

for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    m,n = np.fromfile(file_t, np.int32, 2)
    for program in programs:
        print file_t, program
        if 'sparse' in program:
            p = subprocess.Popen(['./' + program, file_t])
        else:
            p = subprocess.Popen(['./' + program, str(m), str(n)])

        time.sleep(10)
        p.kill()
        print "Stop Now!"
        stop = False
        while not stop:
            time.sleep(1)
            os.system('ps -A | grep %s > .tmp'%program)
            content = open('.tmp').readlines()
            stop = True
            for line in content:
                if len(line) > 5 and 'defunct' not in line:
                    stop = False
        time.sleep(5)


