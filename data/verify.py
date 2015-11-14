import numpy as np
out = np.fromfile(open('output.dat'), dtype=np.float32)
out2 = np.fromfile(open('groundtruth.dat'), dtype=np.float32)
print out[:2000]-out2[:2000]

print out[-1000:]-out2[-1000:]
