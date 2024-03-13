import vertex

class Contour:
    vertexs = []

    # std::vector[vertex.Vertex] vertexs
    def __init__(self, vertexs : list[vertex.Vertex]):
        self.vertexs = vertexs