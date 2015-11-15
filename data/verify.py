import numpy as np
out = np.fromfile(open('output.dat'), dtype=np.float32)
out2 = np.fromfile(open('groundtruth.dat'), dtype=np.float32)
act_len = len(out2)
out[out<0] = 0
out2[out2<0] = 0
if act_len > 200:
    print out[:50]-out2[:50]
    print out[act_len-50:act_len] - out2[act_len-50:]
else:
    print out[:act_len] - out2

# print out[-1000:]-out2[-1000:]
