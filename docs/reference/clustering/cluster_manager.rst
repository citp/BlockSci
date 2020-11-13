ClusterManager
====================================

The ClusterManager can be used to create new or load existing clusterings.
The following example will create a new clustering using the multi-input heuristic and attempting to skip Coinjoin transactions.
To overwrite an existing clustering, set ``should_overwrite=True``.

..  code-block:: python

    cm = blocksci.cluster.ClusterManager.create_clustering(<cluster_directory>, chain)

Instead of creating a new clustering, you can also load a previously created clustering.

..  code-block:: python

    cm = blocksci.cluster.ClusterManager(<cluster_directory>, chain)

From the cluster manager you can retrieve all clusters using :py:meth:`~blocksci.cluster.ClusterManager.clusters` or retrieve a specific cluster based on an address using :py:meth:`~blocksci.cluster.ClusterManager.cluster_with_address`.

Due to the risk of cluster collapse, BlockSci does not cluster change addresses by default.
A few change address detection heuristics are available in :py:mod:`blocksci.heuristics.change` and can be passed to the clusterer using the ``heuristic`` keyword, though we do not recommend using them for clustering without further refinement.

.. autoclass:: blocksci.cluster.ClusterManager
   :members:
