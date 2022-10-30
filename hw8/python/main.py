# coding=utf-8
# This is a sample Python script.

# Press ⌃R to execute it or replace it with your code.
# Press Double ⇧ to search everywhere for classes, files, tool windows, actions, and settings.


import numpy as np
import matplotlib.pyplot as plt
import platform
from scipy.spatial import Delaunay, Voronoi, voronoi_plot_2d
from scipy import argmin, inner


def cvt(samples, times, rnum):
    # rnum = 100 s_num = 10000
    s_num = rnum * rnum
    # 随机生成20个点，'2'表示是2纬的坐标
    p = np.random.random((samples, 2))
    # 10000个点均匀的分摊到0~1之间
    interval = np.linspace(0.0, 1.0, rnum)
    sx = np.zeros(s_num)
    sy = np.zeros(s_num)
    s = np.zeros([s_num, 2])

    # 将100 * 100个离散的点均匀的分布在 1 * 1的区域内
    k = 0
    for j in range(rnum):
        for i in range(rnum):
            sx[k], sy[k] = interval[i], interval[j]
            s[k, 0], s[k, 1] = interval[i], interval[j]
            k += 1

    # 迭代20次
    for it in range(times):
        # 源代码是每迭代3次，生成一张图，我这里为了生成完整的迭代过程，每一帧都会生成图片
        # if it % 3 == 0:
        #     vor = Voronoi(p)
        #     voronoi_plot_2d(vor)
        #     plt.title('Iteration Time: ' + str(it))
        #     plt.show()
        #

        # 生成voronoi图
        vor = Voronoi(p)
        # 绘制voronoi图
        voronoi_plot_2d(vor)
        plt.title('Iteration Time: ' + str(it))
        # 图片保存到本地
        plt.savefig('./Iteration Time: ' + str(it))
        # 展示图片
        plt.show()

        # 遍历100 * 100个点，计算每个点和之前随机成的采样点(20个)的距离
        # 生成一个100 * 100的数组，存储索引，索引表示离哪个采样点最近
        k = [argmin([inner(gg - ss, gg - ss) for gg in p]) for ss in s]

        # w是权重数组，长度为100 * 100，值均为1
        w = np.ones(s_num)
        # 计算每个点对应有多少个离它最近的点，m长度为20
        # 可以查下python bincount的用法，否则不好理解这个计算的意义.python 确实方便，一行代码放到别的语言可能得写十几行
        m = np.bincount(k, weights=w)

        #  用bincount的方式，计算Voronoi图中每个网格的中心，每个点等于周边像素的坐标和
        #  new_x、new_y为网格的中心，
        #  sx、sy 是像素点的坐标，这里作为权重，很巧妙，
        new_x = np.bincount(k, weights=sx)
        new_y = np.bincount(k, weights=sy)

        #  更新采样点的位置
        for i in range(samples):
            if m[i] > 0:
                new_x[i] = new_x[i] / float(m[i])
                new_y[i] = new_y[i] / float(m[i])

        p[:, 0], p[:, 1] = new_x[:], new_y[:]


# python的内置环境变量，判断是从当前文件执行，非import
if __name__ == '__main__':
    # 采样20个点，迭代20次，分辨率为100 * 100
    cvt(20, 20, 100)
