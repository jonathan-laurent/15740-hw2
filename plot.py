#! /usr/bin/env python

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy
import matplotlib.cm as cm
import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=str, help="Input file")
    parser.add_argument("-o", "--out", type=str, \
                            default="mountain.png", help="Output file")
    args = parser.parse_args()

    x, y, z = numpy.loadtxt(args.file, unpack=True)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    ax.invert_yaxis();
    ax.set_xlabel("Stride")
    ax.set_ylabel("log2(size) (Bytes)")
    ax.set_zlabel("MB/s")
    #plt.tight_layout()

    ax.plot_trisurf(x, y, z, cmap=cm.Blues_r)
    plt.savefig(args.out, dpi=300)
