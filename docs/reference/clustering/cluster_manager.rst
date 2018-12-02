ClusterManager
====================================

The ClusterManager can be used to create new or load existing clusterings.
The following example will create a new clustering using the legacy heuristic.
To overwrite an existing clustering, set ``should_overwrite=True``.

..  code-block:: python

    heuristic = blocksci.heuristics.change.legacy.unique_change
    cm = blocksci.cluster.ClusterManager.create_clustering(<cluster_directory>, chain, heuristic)

Instead of creating a new clustering, you can also load a previously created clustering.

..  code-block:: python

    cm = blocksci.cluster.ClusterManager(<cluster_directory>, chain)

From the cluster manager you can retrieve all clusters using :py:meth:`~blocksci.cluster.ClusterManager.clusters` or retrieve a specific cluster based on an address using :py:meth:`~blocksci.cluster.ClusterManager.cluster_with_address`.

Various change address detection heuristics are available in :py:mod:`blocksci.heuristics.change`. If you don't want to include change addresses in your clustering, choose :py:func:`blocksci.heuristics.change.none.unique_change` as your heuristic.

.. autoclass:: blocksci.cluster.ClusterManager
   :members:
