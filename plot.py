#! /usr/bin/env python

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy
import matplotlib.cm as cm
import argparse
import os


stride_label = "Stride (x8 Bytes)"
logsize_label = "log2(size) (Bytes)"
perf_label = "MB/s"

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=str, help="Input file")
    parser.add_argument("-o", "--out", type=str, \
                            default="mountain.png", help="Output file")
    parser.add_argument("-s", "--sections", action="store_true", \
                            default=False, help="Show sections")
    args = parser.parse_args()

    x, y, z = numpy.loadtxt(args.file, unpack=True)

    # Mountain
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    ax.invert_yaxis()
    ax.set_xlabel(stride_label)
    ax.set_ylabel(logsize_label)
    ax.set_zlabel(perf_label)
    #plt.tight_layout()

    ax.plot_trisurf(x, y, z, cmap="terrain")
    plt.savefig(args.out, dpi=300)

    if not args.sections:
        exit()

    maxz = max(z)

    # Varying stride (fixed y)
    stride_dir = "stride"
    if not os.path.isdir(stride_dir):
        os.mkdir(stride_dir)
    for logsize in set(y):
        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.set_title("Constant logsize of {}".format(logsize))
        ax.set_ylim(bottom=0, top=maxz)
        ax.set_xlabel(stride_label)
        ax.set_ylabel(perf_label)
        ax.plot(x[y == logsize], z[y == logsize])
        plt.savefig("{}/stride-{}.png".format(stride_dir, logsize), dpi=300)
        plt.close()

    # Varying logsize
    logsize_dir = "logsize"
    if not os.path.isdir(logsize_dir):
        os.mkdir(logsize_dir)
    for stride in set(x):
        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.set_title("Constant stride of {}".format(stride))
        ax.set_xlabel(logsize_label)
        ax.set_ylabel(perf_label)
        ax.set_ylim(bottom=0, top=maxz)
        ax.plot(y[x == stride], z[x == stride])
        plt.savefig("{}/logsize-{}.png".format(logsize_dir, stride), dpi=300)
        plt.close()
