#! /usr/bin/python
import os,sys

layers = {'lenet_5':['ip1', 'ip2'],
          'lenet_300':['ip1', 'ip2'],
          'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

nets = layers.keys()
banks = [8, 16, 32, 64, 128]

for net in nets:
    for layer in layers[net]:
        for bank_num in banks:
            flag = os.system('python layer_dump_tmp.py --net=%s --layer=%s --bank-num=%d'%(net, layer, bank_num))
            if flag != 0:
                sys.exit(flag)
