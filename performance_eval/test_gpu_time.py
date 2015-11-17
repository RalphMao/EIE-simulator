import glob
import subprocess
import numpy as np
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

dirs = map(lambda x: 'data/' + x, dirs)

f = open('log/log_gpu_time','w')

f.write(', dense_mv, dense_mm, sparse_mv, sparse_mm, \n')
for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    print file_t
    time = [0] * 4

    m,n = np.fromfile(file_t, np.int32, 2)

    p = subprocess.Popen(['./dense_mv', str(m), str(n)], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    out = out.split('\n')
    for line in out:
        if "-cublas_time_batchsize1" in line:
            time[0] = float(line.split()[-2])

    p = subprocess.Popen(['./dense_mm', str(m), str(n)], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    out = out.split('\n')
    for line in out:
        if "-cublas_time_batchsize64" in line:
            time[1] = float(line.split()[-2])

    p = subprocess.Popen(['./sparse_mv',file_t], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    out = out.split('\n')
    for line in out:
        if "-cusparse_time_batchsize1" in line:
            time[2] = float(line.split()[-2])

    p = subprocess.Popen(['./sparse_mm',file_t], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    out = out.split('\n')
    for line in out:
        if "-cusparse_time_batchsize64" in line:
            time[3] = float(line.split()[-2])


    type = file_t.split('/')[-2]
    f.write('%s, %.4f, %.4f, %.4f, %.4f\n'%((type,) +tuple(map(lambda x:x*1000/4096, time))))
    f.flush()
f.close()
