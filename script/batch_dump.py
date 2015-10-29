#! /usr/bin/python
import os,sys

layers = {'lenet5':['ip1', 'ip2'],
          'lenet_300':['ip1', 'ip2', 'ip3'],
          'alexnet':['fc6', 'fc7', 'fc8'],
          'vgg': ['fc6', 'fc7', 'fc8']}

nets = layers.keys()
banks_large = [8, 16, 32, 64, 128]
banks_lenet = [2, 4, 8, 16, 32]

for net in nets:
    if 'lenet' in net:
        banks = banks_lenet
    else:
        banks = banks_large
    for layer in layers[net]:
        for bank_num in banks:
            flag = os.system('python layer_dump_tmp.py --net=%s --layer=%s --bank-num=%d'%(net, layer, bank_num))
            if flag != 0:
                sys.exit(flag)
