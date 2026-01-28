class Entity:
    def __init__(self, type, offset, length):
        self.type = type
        self.offset = offset
        self.length = length

    def __repr__(self):
        return f"Entity({self.type}, {self.offset}, {self.length})"
