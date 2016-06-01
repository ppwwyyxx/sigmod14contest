#!/usr/bin/python2

from math import *
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.font_manager as fontm
import argparse, sys
from itertools import chain

STDIN_FNAME = '-'

def get_args():
    description = "plot points into graph."
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('-i', '--input',
            help = 'input data file, "-" for stdin, default stdin',
            default='-')
    parser.add_argument('-o', '--output',
            help = 'output image', default = '')
    parser.add_argument('--show',
            help = 'show the figure after rendered',
            action = 'store_true')
    parser.add_argument('-c', '--column',
            help="describe each column in data, for example 'x,y,y'. \
            Default to 'y' for one column and 'x,y' for two columns. \
            Plot attributes can be appended after 'y', like 'ythick;cr' \
            ")
    parser.add_argument('-t', '--title',
            help = 'title of the graph',
            default = '')
    parser.add_argument('--xlabel',
            help = 'x label',
            default = 'x')
    parser.add_argument('--ylabel',
            help = 'y label',
            default = 'y')
    parser.add_argument('--annotate-maximum',
            help = 'annonate maximum value in graph',
            action = 'store_true')
    parser.add_argument('--annotate-minimum',
            help = 'annonate minimum value in graph',
            action = 'store_true')
    parser.add_argument('--xkcd',
            help = 'xkcd style',
            action = 'store_true')
    parser.add_argument('--decay',
            help='exponential decay rate to smooth Y',
            type=float, default=0)
    parser.add_argument('--legend',
            help='legend for each y')
    parser.add_argument('--ignore-difflen',
            help='ignore the length different in each column, take the minimum',
            action='store_true')

    global args
    args = parser.parse_args();

    if not args.show and not args.output:
        args.show = True

def filter_valid_range(points, rect):
    """rect = (min_x, max_x, min_y, max_y)"""
    ret = []
    for x, y in points:
        if x >= rect[0] and x <= rect[1] and y >= rect[2] and y <= rect[3]:
                ret.append((x, y))
    if len(ret) == 0:
        ret.append(points[0])
    return ret

def exponential_smooth(data, alpha):
    """ smooth data by alpha. returned a smoothed version"""
    ret = np.copy(data)
    now = data[0]
    for k in range(len(data)):
        ret[k] = now * alpha + data[k] * (1-alpha)
        now = ret[k]
    return ret

def annotate_min_max(data_x, data_y, ax):
    max_x, min_x = max(data_x), min(data_x)
    max_y, min_y = max(data_y), min(data_y)
    x_range = max_x - min_x
    y_range = max_y - min_y
    x_max, y_max = data_y[0], data_y[0]
    x_min, y_min = data_x[0], data_y[0]

    for i in xrange(1, len(data_x)):
        if data_y[i] > y_max:
            y_max = data_y[i]
            x_max = data_x[i]
        if data_y[i] < y_min:
            y_min = data_y[i]
            x_min = data_x[i]

    rect = ax.axis()
    if args.annotate_maximum:
        text_x, text_y = filter_valid_range([
            (x_max + 0.05 * x_range,
                y_max + 0.025 * y_range),
            (x_max - 0.05 * x_range,
                y_max + 0.025 * y_range),
            (x_max + 0.05 * x_range,
                y_max - 0.025 * y_range),
            (x_max - 0.05 * x_range,
                y_max - 0.025 * y_range)],
            rect)[0]
        ax.annotate('maximum ({:d},{:.3f})' . format(int(x_max), y_max),
                xy = (x_max, y_max),
                xytext = (text_x, text_y),
                arrowprops = dict(arrowstyle = '->'))
    if args.annotate_minimum:
        text_x, text_y = filter_valid_range([
            (x_min + 0.05 * x_range,
                y_min - 0.025 * y_range),
            (x_min - 0.05 * x_range,
                y_min - 0.025 * y_range),
            (x_min + 0.05 * x_range,
                y_min + 0.025 * y_range),
            (x_min - 0.05 * x_range,
                y_min + 0.025 * y_range)],
            rect)[0]
        #ax.annotate('minimum ({:d},{:.3f})' . format(int(x_min), y_min),
                #xy = (x_min, y_min),
                #xytext = (text_x, text_y),
                #arrowprops = dict(arrowstyle = '->'))
        ax.annotate('{:.3f}' . format(y_min),
                xy = (x_min, y_min),
                xytext = (text_x, text_y),
                arrowprops = dict(arrowstyle = '->'))

def plot_args_from_column_desc(desc):
    if not desc:
        return {}
    ret = {}
    desc = desc.split(';')
    if 'thick' in desc:
        ret['lw'] = 5
    if 'dash' in desc:
        ret['ls'] = '--'
    for v in desc:
        if v.startswith('c'):
            ret['color'] = v[1:]
    return ret

def do_plot(data_x, data_ys):
    fig = plt.figure(figsize = (16.18/1.2, 10/1.2))
    ax = fig.add_axes((0.1, 0.2, 0.8, 0.7))
    nr_y = len(data_ys[0])
    y_column = args.y_column
    assert len(y_column) == nr_y
    if args.legend:
        legends = args.legend.split(',')
        assert len(legends) == nr_y
    else:
        legends = None
    for yidx in range(nr_y):
        plotargs = plot_args_from_column_desc(y_column[yidx][1:])
        data_y = data_ys[:,yidx]
        leg = legends[yidx] if legends else None
        p = plt.plot(data_x, data_y, label=leg, **plotargs)

        c = p[0].get_color()
        plt.fill_between(data_x, data_y, alpha=0.1, facecolor=c)

        #ax.set_aspect('equal', 'datalim')
        #ax.spines['right'].set_color('none')
        #ax.spines['left'].set_color('none')
        #plt.xticks([])
        #plt.yticks([])

        if args.annotate_maximum or args.annotate_minimum:
            annotate_min_max(data_x, data_y, ax)

    plt.xlabel(args.xlabel, fontsize='xx-large')
    plt.ylabel(args.ylabel, fontsize='xx-large')
    plt.legend(loc='best', fontsize='xx-large')

    for label in chain.from_iterable(
            [ax.get_xticklabels(), ax.get_yticklabels()]):
        label.set_fontproperties(fontm.FontProperties(size=15))

    ax.grid(color = 'gray', linestyle = 'dashed')

    plt.title(args.title, fontdict={'fontsize': '20'})

    if args.output != '':
        plt.savefig(args.output)
    if args.show:
        plt.show()

def main():
    get_args()
    if args.input == STDIN_FNAME:
        fin = sys.stdin
    else:
        fin = open(args.input)
    all_inputs = fin.readlines()
    if args.input != STDIN_FNAME:
        fin.close()

    nr_column = len(all_inputs[0].rstrip().split())
    if args.column is None:
        if nr_column == 1:
            column = ['y']
        elif nr_column == 2:
            column = ['x', 'y']
        else:
           raise Exception("Please specify column by '-c/--column'.")
    else:
        column = args.column.strip().split(',')
    for k in column: assert k[0] in ['x', 'y']
    assert nr_column == len(column), "Column and data doesn't have same length. {}!={}".format(nr_column, len(column))
    args.y_column = [v for v in column if v[0] == 'y']
    nr_x_column = column.count('x')
    assert nr_x_column <= 1, "At most one 'x' is allowed"

    data_x = []
    data_ys = []
    data_format = -1
    for lineno, line in enumerate(all_inputs):
        line = [float(i) for i in line.rstrip().split()]
        if not args.ignore_difflen:
            assert len(line) == nr_column, \
                    "Data doesn't have same length! Use --ignore-difflen to ignore."
        else:
            if len(line) != nr_column:
                continue

        data_ys.append([])
        for val, tp in zip(line, column):
            if tp == 'x':
                data_x.append(val)
            else:
                data_ys[-1].append(val)
        if nr_x_column == 0:
            data_x.append(lineno + 1)
    print "Data size: ", len(data_x)
    assert len(data_x) == len(data_ys)

    if len(data_x) == 1:
        return

    data_ys = np.array(data_ys) # nr x dim
    if args.decay != 0:
        data_ys = exponential_smooth(data_ys, args.decay)

    if args.xkcd:
        with plt.xkcd():
            do_plot(data_x, data_ys)
    else:
        do_plot(data_x, data_ys)

if __name__ == '__main__':
    main()
