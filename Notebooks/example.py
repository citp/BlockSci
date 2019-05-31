import blocksci

chain = blocksci.Blockchain("../zcash-devdata")

in_z_addresses = 0
num_of_txes = 0
num_of_jstxes = 0
num_of_ins = 0
num_of_outs = 0


for blk in chain:
	for tx in blk:
		num_of_txes += 1
		if tx.is_joinsplit:
			num_of_jstxes += 1
			in_z_addresses += tx.sum_vpubold - tx.sum_vpubnew
		for ins in tx.ins:
			num_of_ins += 1
		for outs in tx.outs:
			num_of_outs += 1

print('Number of transactions: ' + str(num_of_txes))
print('Of that hidden transactions: ' + str(num_of_jstxes))
print('Number of transaction inputs: ' + str(num_of_ins))
print('Number of transaction outputs: ' + str(num_of_outs))
print('amount of Zatoshis in Z addresses: ' + str(in_z_addresses))