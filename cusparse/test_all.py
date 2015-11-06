import glob
import subprocess
import numpy as np
dirs = glob.glob('data/*')

f = open('log','w')
for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    print file_t
    time = [0] * 4

    p = subprocess.Popen(['./sparse_test',file_t], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    out = out.split('\n')
    for line in out:
        if "-cusparse_time_batchsize1" in line:
            time[2] = int(float(line.split()[-2]))
        if "-cusparse_time_batchsize64" in line:
            time[3] = int(float(line.split()[-2]))

    m,n = np.fromfile(file_t, np.int32, 2)
    p = subprocess.Popen(['./dense_test', str(m), str(n)], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    out = out.split('\n')
    for line in out:
        if "-cublas_time_batchsize1" in line:
            time[0] = int(float(line.split()[-2]))
        if "-cublas_time_batchsize64" in line:
            time[1] = int(float(line.split()[-2]))

    type = file_t.split('/')[-2]
    f.write('%s, %d, %d, %d, %d\n'%((type,) +tuple(map(lambda x:x*1000/4096, time))))
f.close()
ff.close()
