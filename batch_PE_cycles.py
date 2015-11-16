
import os, sys
import subprocess
import numpy as np
layers = {'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

PEs = 2 ** np.arange(0,9)

f = open('cycles_PE.log','w')
nets = layers.keys()
for net in nets:
    for layer in layers[net]:
        f.write(net + '_' + layer + ', ')
        for num_pe in PEs:
            print net, layer
            flag = os.system('python script/layer_dump2.py --net=%s --layer=%s --bank-num=%d'%(net, layer, num_pe))
            os.system('make')
            p = subprocess.Popen(['./simulation'], stdout = subprocess.PIPE, stderr= subprocess.PIPE)
            out, err = p.communicate()
            out = err.split('\n')
            cycles = 0
            for line in out:
                if 'cycles' in line:
                    cycles = int(line.split(':')[-1])
            if cycles == 0:
                print "wrong!"
                sys.exit(1)
        
            f.write('%d, '%cycles)
        f.write('\n')

layers_rnn = {'neutalk':['We', 'Wd', 'WLSTM']}
    
nets=  layers_rnn.keys()
for net in nets:
    for layer in layers_rnn[net]:
        f.write('%s_%s, '%(net, layer))
        for num_pe in PEs:
            flag = os.system('python script/lstm_layer_dump.py --layer=%s --bank-num=%s'%(layer, num_pe))
            print net, layer, num_pe
            os.system('make')
            p = subprocess.Popen(['./simulation'], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
            out, err = p.communicate()
            out = err.split('\n')
            cycles = 0
            for line in out:
                if 'cycles' in line:
                    cycles = int(line.split(':')[-1])
            if cycles == 0:
                print "wrong!"
                sys.exit(1)
        
            f.write('%d, '%(cycles))
        f.write('\n')
    
f.close()

