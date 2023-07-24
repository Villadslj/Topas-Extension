import matplotlib.pyplot as plt
import numpy as np
import os
from topas2numpy import BinnedResult
from os import listdir
from os.path import isfile, join


# find all .csv files in directory
mypath = os.getcwd()
onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f)) and f[len(f)-3:len(f)] == "csv"]
print(onlyfiles)
for dose in onlyfiles:
    print('{0} [{1}]'.format(dose.quantity, dose.unit))
    print('Statistics: {0}'.format(dose.statistics))
    for dim in dose.dimensions:
        print('{0} [{1}]: {2} bins'.format(dim.name, dim.unit, dim.n_bins))

    ax = plt.subplot(111)
    x = dose.dimensions[2].get_bin_centers()
    y = np.squeeze(dose.data['Sum'])
    plt.plot(np.flip(x), y)
    plt.xlabel('Depth [cm]')
    plt.ylabel(dose.quantity)
    plt.grid()
    plt.show()
    plt.clf()


# plt.legend(['Antiprotons', 'Protons'])

# plt.vlines(x =[0.5, 1.5], ymin = 0, ymax =max(z), color = 'black', label = 'Cellflask 1')
# plt.vlines(x =[8.5, 9.5], ymin = 0, ymax =max(np.squeeze(dose.data['Sum'])), color = 'blue', label = 'Cellflask 2')
# plt.vlines(x =[11.8, 12.8], ymin = 0, ymax =max(np.squeeze(dose.data['Sum'])), color = 'red', label = 'Cellflask 3')
# plt.xlabel('Depth [cm]')
# plt.ylabel('Dose [Gy]')
# plt.show()