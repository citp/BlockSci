from ._blocksci import *
import pickle
import sqlite3
from collections import namedtuple

# Simple class representing a record in our database.
MemoRecord = namedtuple("MemoRecord", "key, task")

class Pickler(pickle.Pickler):
    """Custom Pickler for BlockSci objects"""

    def persistent_id(self, obj):
        # Instead of pickling MemoRecord as a regular class instance, we emit a
        # persistent ID.
        if isinstance(obj, Block):
            # Here, our persistent ID is simply a tuple, containing a tag and a
            # key, which refers to a specific record in the database.
            return ("Block", obj.height)
        elif isinstance(obj, Tx):
            return ("Tx", obj.index)
        elif isinstance(obj, Output):
            return ("Output", (obj.tx_index, obj.index))
        elif isinstance(obj, Input):
            return ("Input", (obj.tx_index, obj.index))
        elif isinstance(obj, Address):
            return ("Address", (obj.address_num, obj.type))
        elif isinstance(obj, EquivAddress):
            sample_addr = next(iter(obj))
            return ("EquivAddress", (sample_addr.address_num, sample_addr.type, obj.is_script_equiv))
        else:
            # If obj does not have a persistent ID, return None. This means obj
            # needs to be pickled as usual.
            return None


class Unpickler(pickle.Unpickler):
    """Custom Unpickler for BlockSci objects"""

    def __init__(self, file, chain):
        super().__init__(file)
        self.chain = chain

    def persistent_load(self, pid):
        # This method is invoked whenever a persistent ID is encountered.
        # Here, pid is the tuple returned by DBPickler.
        type_tag, key_id = pid
        if type_tag == "Block":
            return self.chain[key_id]
        elif type_tag == "Tx":
            return self.chain.tx_with_index(key_id)
        elif type_tag == "Output":
            return self.chain.tx_with_index(key_id[0]).outputs[key_id[1]]
        elif type_tag == "Input":
            return self.chain.tx_with_index(key_id[0]).inputs[key_id[1]]
        elif type_tag == "Address":
            return self.chain.address_from_index(key_id[0], key_id[1])
        elif type_tag == "EquivAddress":
            addr = self.chain.address_from_index(key_id[0], key_id[1])
            return addr.equiv(key_id[2])
        else:
            # Always raises an error if you cannot return the correct object.
            # Otherwise, the unpickler will think None is the object referenced
            # by the persistent ID.
            raise pickle.UnpicklingError("unsupported persistent object")
