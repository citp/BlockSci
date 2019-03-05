import blocksci


def test_clustering_no_change(chain, json_data, regtest, tmpdir_factory):
    cm = blocksci.cluster.ClusterManager.create_clustering(str(tmpdir_factory.mktemp("clustering")), chain,
                                                           heuristic=blocksci.heuristics.change.none.unique_change)
    cluster = cm.cluster_with_address(chain.address_from_string(json_data['merge-addr-1']))

    assert 3 == len(cluster.addresses.to_list())
    assert 3 == cluster.address_count()

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
    print(cluster.address_count(), file=regtest)
    print(cluster.txes(), file=regtest)
    print(cluster.type_equiv_size, file=regtest)

    for tx in chain.blocks.txes:
        if tx.input_count > 1 and not blocksci.heuristics.is_coinjoin(tx):
            cluster = cm.cluster_with_address(tx.inputs[0].address)
            addresses = cluster.addresses.to_list()
            for i in range(tx.input_count):
                assert tx.inputs[i].address in addresses


def test_clustering_with_change(chain, json_data, tmpdir_factory):
    heuristics = [
        blocksci.heuristics.change.peeling_chain.unique_change,
        blocksci.heuristics.change.optimal_change.unique_change,
        blocksci.heuristics.change.address_type.unique_change,
        blocksci.heuristics.change.locktime.unique_change,
        blocksci.heuristics.change.address_reuse.unique_change,
        blocksci.heuristics.change.client_change_address_behavior.unique_change,
        blocksci.heuristics.change.legacy.unique_change,
        blocksci.heuristics.change.none.unique_change
    ]

    for f in heuristics:
        cm = blocksci.cluster.ClusterManager.create_clustering(str(tmpdir_factory.mktemp("clustering")), chain,
                                                               heuristic=f)
        cluster = cm.cluster_with_address(chain.address_from_string(json_data['merge-addr-1']))

        assert 3 <= len(cluster.addresses.to_list())

        assert chain.address_from_string(json_data['merge-addr-1']) in cluster.addresses.to_list()
        assert chain.address_from_string(json_data['merge-addr-2']) in cluster.addresses.to_list()
        assert chain.address_from_string(json_data['merge-addr-3']) in cluster.addresses.to_list()

        assert cluster.cluster_num >= 0
        assert cluster.index >= 0


def test_clustering_ignore_coinjoin(chain, json_data, tmpdir_factory):
    addresses = chain.tx_with_hash(json_data['simple-coinjoin-tx']).inputs.map(lambda i: i.address).to_list()

    cm = blocksci.cluster.ClusterManager.create_clustering(str(tmpdir_factory.mktemp("clustering")), chain,
                                                           heuristic=blocksci.heuristics.change.none.unique_change,
                                                           ignore_coinjoin=True)
    cluster = cm.cluster_with_address(addresses[0])
    cluster_addresses = cluster.addresses.to_list()
    assert 1 == len(cluster)

    for addr in addresses[1:]:
        assert addr not in cluster_addresses

    # Normal clustering should still work as expected
    cluster = cm.cluster_with_address(chain.address_from_string(json_data['merge-addr-1']))
    assert 3 <= len(cluster.addresses.to_list())

    assert chain.address_from_string(json_data['merge-addr-1']) in cluster.addresses.to_list()
    assert chain.address_from_string(json_data['merge-addr-2']) in cluster.addresses.to_list()
    assert chain.address_from_string(json_data['merge-addr-3']) in cluster.addresses.to_list()


def test_clustering_cluster_coinjoin(chain, json_data, tmpdir_factory):
    addresses = chain.tx_with_hash(json_data['simple-coinjoin-tx']).inputs.map(lambda i: i.address).to_list()

    cm = blocksci.cluster.ClusterManager.create_clustering(str(tmpdir_factory.mktemp("clustering")), chain,
                                                           heuristic=blocksci.heuristics.change.none.unique_change,
                                                           ignore_coinjoin=False)
    cluster = cm.cluster_with_address(addresses[0])
    cluster_addresses = cluster.addresses.to_list()
    assert 1 < len(cluster)

    for addr in addresses:
        assert addr in cluster_addresses

    # Normal clustering should still work as expected
    cluster = cm.cluster_with_address(chain.address_from_string(json_data['merge-addr-1']))
    assert 3 <= len(cluster.addresses.to_list())

    assert chain.address_from_string(json_data['merge-addr-1']) in cluster.addresses.to_list()
    assert chain.address_from_string(json_data['merge-addr-2']) in cluster.addresses.to_list()
    assert chain.address_from_string(json_data['merge-addr-3']) in cluster.addresses.to_list()
