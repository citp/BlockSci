from blocksci.blocksci_interface import *
from multiprocess import Pool
import psutil
import tempfile
import importlib
import subprocess
import sys
import os
import inspect

def map_blocks_general(chain, start, end, mapFunc):
    cpu_count = psutil.cpu_count()
    segments = chain.segment(start, end, cpu_count)
    with Pool(cpu_count - 1) as p:
        results_future = p.map_async(mapFunc, segments[1:])
        last = mapFunc(segments[0])
        results = results_future.get()
        results.insert(0, last)
    return [x for y in results for x in y]

def map_blocks(self, start, end, blockFunc):
    def mapFunc(blocks):
        return [blockFunc(block) for block in blocks]
    
    return map_blocks_general(self, start, end, mapFunc)

def filter_txes(self, start, end, filterFunc):
    def mapFunc(blocks):
        return [tx for block in blocks for tx in block if filterFunc(tx)]
    return map_blocks_general(self, start, end, mapFunc)

old_init = Blockchain.__init__
def new_init(self, loc = "/data2/bitcoin-data/"):
    old_init(self, loc)

Blockchain.__init__ = new_init
Blockchain.map_blocks = map_blocks
Blockchain.filter_txes = filter_txes
  
moduleDirectory = tempfile.TemporaryDirectory()
sys.path.append(moduleDirectory.name)
print(moduleDirectory.name)
dynamicFunctionCounter = 0

class DummyClass:
    pass 

loaderDirectory = os.path.dirname(os.path.abspath(inspect.getsourcefile(DummyClass)))

def function_loader(code, function_name):
    global dynamicFunctionCounter
    global moduleDirectory
    def generate_code(func_name, module_name, func_def):
        from string import Template
        filein = open(loaderDirectory + '/templateExtension.cpp')
        template = Template(filein.read())
        return template.safe_substitute({"func_name" : func_name, "module_name":module_name, "func_def" : func_def})
        
    def create_makefile(module_name):
        from string import Template
        import os
        filein = open(loaderDirectory + '/templateMakefile')
        template = Template(filein.read())
        subs = {"module_name" : module_name, "install_location" : moduleDirectory.name, "srcname" : module_name + ".cpp", "loaderDirectory":loaderDirectory}
        return template.safe_substitute(subs)
    module_name = "dynamicCode" + str(dynamicFunctionCounter)
    dynamicFunctionCounter += 1
    full_code = generate_code(function_name, module_name, code)
    makefile = create_makefile(module_name)
    builddir = tempfile.TemporaryDirectory()
    print(builddir)
    with open(builddir.name + '/' + module_name + ".cpp", 'w') as f:
        f.write(full_code)
    with open(builddir.name + '/CMakeLists.txt', 'w') as f:
        f.write(makefile)
    process = subprocess.Popen(["cmake", "."], cwd=builddir.name, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    print(err.decode('utf8'))
    process = subprocess.Popen(["make"], cwd=builddir.name, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    print(err.decode('utf8'))
    process = subprocess.Popen(["make", "install"], cwd=builddir.name, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    print(err.decode('utf8'))
    mod = importlib.import_module(module_name)
    return getattr(mod, "func")    
