# read file and draw polygon

# data format:
# contour nums
# is hole
# point nums
# x y

import matplotlib.pyplot as plt
import numpy as np
import sys

import polygon.Polygon as Polygon

def read_file(file_name):
    with open(file_name, 'r') as f:
        polygons = []

        contour_nums = int(f.readline())
        for i in range(contour_nums):
            is_hole = int(f.readline())
            
            vertexs = []
            point_nums = int(f.readline())
            for j in range(point_nums):
                vertex = []
                point = f.readline().split()
                vertex.append(float(point[0]))
                vertex.append(float(point[1]))
                vertexs.append(vertex)
            
            polygons.append(Polygon(vertexs, is_hole))

    return polygons

def draw_polygons(polygons):
    for polygon in polygons:
        for contour in polygon.contours:
            x = []
            y = []
            for vertex in contour.vertexs:
                x.append(vertex.x)
                y.append(vertex.y)
            x.append(contour.vertexs[0].x)
            y.append(contour.vertexs[0].y)
            plt.plot(x, y)
    plt.show()

file_name = "..//build//windows//x64//debug//result.txt" # sys.argv[1]
polygons = read_file(file_name)
draw_polygons(polygons)
