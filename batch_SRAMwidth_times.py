
import os, sys
import subprocess
import numpy as np
layers = {'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

widths = [16,32,64,128,256,512]

f = open('times_SRAMwidth.log','w')
nets = layers.keys()
for net in nets:
    for layer in layers[net]:
        f.write(net + '_' + layer + ', ')
        for width in widths:
            print net, layer
            flag = os.system('python script/layer_dump2.py --net=%s --layer=%s --sram-line=%d'%(net, layer, width))
            os.system('make')
            p = subprocess.Popen(['./simulation'], stdout = subprocess.PIPE, stderr= subprocess.PIPE)
            out, err = p.communicate()
            out = err.split('\n')
            times = 0
            for line in out:
                if 'total_spm' in line:
                    times = int(line.split(':')[-1])
            if times == 0:
                print "wrong!"
                sys.exit(1)
        
            f.write('%d, '%times)
        f.write('\n')

layers_rnn = {'neutalk':['We', 'Wd', 'WLSTM']}
    
nets=  layers_rnn.keys()
for net in nets:
    for layer in layers_rnn[net]:
        f.write('%s_%s, '%(net, layer))
        for width in widths:
            flag = os.system('python script/lstm_layer_dump.py --layer=%s --sram-line=%s'%(layer, width))
            print net, layer, width
            os.system('make')
            p = subprocess.Popen(['./simulation'], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
            out, err = p.communicate()
            out = err.split('\n')
            times = 0
            for line in out:
                if 'total_spm' in line:
                    times = int(line.split(':')[-1])
            if times == 0:
                print "wrong!"
                sys.exit(1)
        
            f.write('%d, '%(times))
        f.write('\n')
    
f.close()

