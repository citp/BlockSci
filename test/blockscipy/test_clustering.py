import blocksci


def no_change_heuristic():
    return blocksci.heuristics.change.legacy - blocksci.heuristics.change.legacy


def test_clustering_no_change(chain, json_data, regtest, tmpdir_factory):
    cm = blocksci.cluster.ClusterManager.create_clustering(str(tmpdir_factory.mktemp("clustering")), chain,
                                                           heuristic=no_change_heuristic().unique_change)
    cluster = cm.cluster_with_address(chain.address_from_string(json_data['merge-addr-1']))

    assert 3 == len(cluster.addresses.to_list())

    assert chain.address_from_string(json_data['merge-addr-1']) in cluster.addresses.to_list()
    assert chain.address_from_string(json_data['merge-addr-2']) in cluster.addresses.to_list()
    assert chain.address_from_string(json_data['merge-addr-3']) in cluster.addresses.to_list()

    assert cluster.cluster_num >= 0
    assert cluster.index >= 0

    print(cluster.addresses.to_list(), file=regtest)
    print(cluster.balance(), file=regtest)
    print(cluster.balance(100), file=regtest)
    print(cluster.count_of_type(blocksci.address_type.pubkeyhash), file=regtest)
    print(cluster.count_of_type(blocksci.address_type.scripthash), file=regtest)
    print(cluster.count_of_type(blocksci.address_type.witness_pubkeyhash), file=regtest)
    print(cluster.count_of_type(blocksci.address_type.witness_scripthash), file=regtest)
    print(cluster.in_txes(), file=regtest)
    print(cluster.in_txes_count(), file=regtest)
    print(cluster.ins().to_list(), file=regtest)
    print(cluster.txes(), file=regtest)
    print(cluster.out_txes(), file=regtest)
    print(cluster.out_txes_count(), file=regtest)
    print(cluster.outs().to_list(), file=regtest)
    print(cluster.size(), file=regtest)
    print(cluster.txes(), file=regtest)
    print(cluster.type_equiv_size, file=regtest)

