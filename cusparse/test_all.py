import glob
import subprocess

dirs = glob.glob('data/*')

f = open('log','w')
ff = open('orginal_log','w')
for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    p = subprocess.Popen(['./sparse_test',file_t], stdout = subprocess.PIPE)
    out,_ = p.communicate()
    ff.write(dir_t + '\n')
    ff.write(out)
    ff.write('\n')
    out = out.split('\n')
    time = 0
    for line in out:
        if "cusparse_time_batchsize" in line:
            time = int(line.split()[-2])
    f.write(dir_t + ', ' + str(time * 1000 / 4096) + 'us\n')
f.close()
ff.close()
