FEE = 0.0001


def tz_offset(ts):
    local_time = ts.astimezone().tzinfo
    return local_time.utcoffset(ts)


def reward(height):
    return Coin(50) if height < 150 else Coin(25 - 10)


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
        if type(other) == Coin:
            return self.satoshi() == other.satoshi()
        else:
            return self.satoshi() == other

    def __ne__(self, other):
        if type(other) == Coin:
            return self.satoshi() != other.satoshi()
        else:
            return self.satoshi() != other

    def __le__(self, other):
        if type(other) == Coin:
            return self.satoshi() <= other.satoshi()
        else:
            return self.satoshi() <= other

    def __ge__(self, other):
        if type(other) == Coin:
            return self.satoshi() >= other.satoshi()
        else:
            return self.satoshi() >= other

    def __lt__(self, other):
        if type(other) == Coin:
            return self.satoshi() < other.satoshi()
        else:
            return self.satoshi() < other

    def __gt__(self, other):
        if type(other) == Coin:
            return self.satoshi() > other.satoshi()
        else:
            return self.satoshi() > other

    def __add__(self, other):
        if type(other) == Coin:
            return self.satoshi() + other.satoshi()
        else:
            return self.satoshi() + other

    def __sub__(self, other):
        if type(other) == Coin:
            return self.satoshi() - other.satoshi()
        else:
            return self.satoshi() - other

    def __mul__(self, other):
        if type(other) == Coin:
            return self.satoshi() * other.satoshi()
        else:
            return self.satoshi() * other

    def __div__(self, other):
        if type(other) == Coin:
            return self.satoshi() / other.satoshi()
        else:
            return self.satoshi() / other

    def __str__(self):
        return "Coin[{}]".format(self.satoshi())

    def __repr__(self):
        return "Coin[{}]".format(self.satoshi())