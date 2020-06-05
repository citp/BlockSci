Clustering
====================================

.. automodule:: blocksci.cluster

The BlockSci clustering module provides users with the ability to apply heuristic based clustering techniques to a Blockchain instance. The :py:class:`ClusterManager` is the main entrace into this module. Using it you can open an already produced clustering or create a new clustering based on a change address heuristic of your choice.

BlockSci's clustering module is a simple effort to explore heuristic based clustering techniques. Users should not assume that it's results will be correct in practice. Providing a more accurate clustering mechanism is an ongoing research project.

A clustering consists of a list of :py:class:`blocksci.cluster.Cluster` objects each containing a list of addresses which have been marked as members of the cluster. Users can efficiently find which cluster an address is in as well as find which addresses a cluster contains.

The clustering module also supports a mechanism for applying externally provided address tags in order to label clusters as likely belonging to given users.

.. toctree::
   :maxdepth: 2
   :caption: Clustering:
   
   cluster_manager
   cluster
   tagging

   ranges/ranges
   iterators/iterators
