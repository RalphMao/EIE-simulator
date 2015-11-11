
import os, sys
import subprocess
layers = {'lenet5':['ip1', 'ip2'],
          'lenet_300':['ip1', 'ip2', 'ip3'],
          'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

layers_rnn = {'neutalk':['We', 'Wd', 'WLSTM']}

f = open('cycles.log','w')
nets = layers.keys()
for net in nets:
    for layer in layers[net]:
        print net, layer
        flag = os.system('python script/layer_dump2.py --net=%s --layer=%s'%(net, layer))
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
    
        f.write('%s_%s, %d\n'%(net, layer, cycles))
    
nets=  layers_rnn.keys()
for net in nets:
    for layer in layers_rnn[net]:
        flag = os.system('python script/lstm_layer_dump.py '+layer)
        print net, layer
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
    
        f.write('%s_%s, %d\n'%(net, layer, cycles))
    
f.close()

