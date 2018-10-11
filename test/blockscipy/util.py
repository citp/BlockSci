FEE = 0.0001


class Coin(object):
    COIN = 1e8

    def __init__(self, val):
        self.val = val

    @classmethod
    def from_satoshi(cls, value):
        return cls(value / cls.COIN)

    def satoshi(self):
        return int(round(self.val * self.COIN, 0))

    def bitcoin(self):
        return round(self.val, 8)

    def __eq__(self, other):
        return self.satoshi() == other

    def __ne__(self, other):
        return self.satoshi() != other

    def __le__(self, other):
        return self.satoshi() <= other

    def __ge__(self, other):
        return self.satoshi() >= other

    def __lt__(self, other):
        return self.satoshi() < other

    def __gt__(self, other):
        return self.satoshi() > other
