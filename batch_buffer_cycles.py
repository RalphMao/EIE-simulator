
import os, sys
import subprocess
import numpy as np
layers = {'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

buffersizes = 2 ** np.arange(1,10)


keyword = 'Efficiency'
f = open('%s_buffer.log'%keyword,'w')

nets = layers.keys()
for net in nets:
    for layer in layers[net]:
        f.write(net + '_' + layer + ', ')
        for buffersize in buffersizes:
            print net, layer
            flag = os.system('python script/layer_dump2.py --net=%s --layer=%s --buffersize=%d'%(net, layer, buffersize))
            os.system('make')
            p = subprocess.Popen(['./simulation'], stdout = subprocess.PIPE, stderr= subprocess.PIPE)
            out, err = p.communicate()
            out = err.split('\n')
            cycles = 0
            for line in out:
                if keyword in line:
                    cycles = float(line.split(':')[-1])
            if cycles == 0:
                print "wrong!"
                sys.exit(1)
        
            f.write('%.4f, '%cycles)
        f.write('\n')

layers_rnn = {'neutalk':['We', 'Wd', 'WLSTM']}
    
nets=  layers_rnn.keys()
for net in nets:
    for layer in layers_rnn[net]:
        f.write('%s_%s, '%(net, layer))
        for buffersize in buffersizes:
            flag = os.system('python script/lstm_layer_dump.py --layer=%s --buffersize=%s'%(layer, buffersize))
            print net, layer, buffersize
            os.system('make')
            p = subprocess.Popen(['./simulation'], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
            out, err = p.communicate()
            out = err.split('\n')
            cycles = 0
            for line in out:
                if keyword in line:
                    cycles = float(line.split(':')[-1])
            if cycles == 0:
                print "wrong!"
                sys.exit(1)
        
            f.write('%.4f, '%(cycles))
        f.write('\n')
    
f.close()

