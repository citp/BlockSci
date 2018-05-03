Change Address Heuristics
--------------------------

BlockSci supports a number of different heuristics for determining the change address for a given transaction. The heuristics are sometimes contradictory and thus we provide users with the ability to choose which heuristics they wish to apply. Further, we support the composibility of different change address heuristics through the various composition operators of the :py:class:`blocksci.heuristics.change.ChangeHeuristic` class.

.. automodule:: blocksci.heuristics.change
	:special-members: __call__, __and__, __or__, __sub__
	:members: