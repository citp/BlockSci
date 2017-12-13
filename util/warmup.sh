fio --filename=/home/ubuntu/bitcoin/chain/block.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/chain/tx_index.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/chain/tx_data.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/chain/sequence_index.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/chain/sequence_data.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/chain/tx_hashes.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/chain/coinbases.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize

touch /home/ubuntu/BlockSci/TX_DATA_HEATED

fio --filename=/home/ubuntu/bitcoin/scripts/pubkey_script.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/multisig_script_data.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/multisig_script_index.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/nonstandard_script_data.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/nonstandard_script_index.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/null_data_script_data.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/null_data_script_index.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/scripts/scripthash_script.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize

touch /home/ubuntu/BlockSci/SCRIPT_DATA_HEATED

fio --filename=/home/ubuntu/bitcoin/hashIndex.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize
fio --filename=/home/ubuntu/bitcoin/addressesDb.dat --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize

touch /home/ubuntu/BlockSci/INDEX_DATA_HEATED
