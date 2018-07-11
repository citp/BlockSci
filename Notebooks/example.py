import blocksci
import os

chain = blocksci.Blockchain("../../../zcash-data") # Or your custom directory of your parsed zcash data

in_z_addresses = 0
num_of_txes = 0
num_of_jstxes = 0
num_of_joinsplits = 0
num_of_ins = 0
num_of_outs = 0
val_of_vpubolds = 0
val_of_vpubnews = 0

for blk in chain:
	for tx in blk:
		num_of_txes += 1
		num_of_joinsplits += tx.joinsplits
		if tx.is_joinsplit:
			num_of_jstxes += 1
			in_z_addresses += tx.sum_vpubold - tx.sum_vpubnew
		for ins in tx.ins:
			num_of_ins += 1
		for outs in tx.outs:
			num_of_outs += 1
		for olds in tx.vpubold:
			val_of_vpubolds += olds
		for news in tx.vpubnew:
			val_of_vpubnews += news
			

print('Number of transactions: ' + str(num_of_txes))
print('Of that hidden transactions: ' + str(num_of_jstxes))
print('Number of joinsplits: ' + str(num_of_joinsplits))
print('Number of transaction inputs: ' + str(num_of_ins))
print('Number of transaction outputs: ' + str(num_of_outs))
print('amount of Zatoshis in Z addresses: ' + str(in_z_addresses))


os._exit(0) # The os exit is used, because otherwise the python3 crashes on exit
