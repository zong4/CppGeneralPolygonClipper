# polygon 类

import contour

class Polygon:
    holes = []
    contours = []

    def __init__(self, contours : list[contour.Contour], holes : list[bool]):
        self.contours = contours
        self.holes = holes