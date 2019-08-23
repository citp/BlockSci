//
//  doctor.hpp
//  blocksci_parser
//
//  Created by Malte Moeser on 8/22/19.
//

#ifndef doctor_hpp
#define doctor_hpp

#include "parser_configuration.hpp"

#include <wjfilesystem/path.h>
#include <nlohmann/json.hpp>

class BlockSciDoctor {

    int warnings = 0;
    int errors = 0;

    // Safe open files limit -- this is a guess and may need to be adjusted
    const rlim_t MIN_OPEN_FILES = 20000;

private:

    const filesystem::path configFilePath;
    const ParserConfigurationBase config;
    nlohmann::json jsonConf;

public:

    BlockSciDoctor(filesystem::path _configFilePath);

    /** Verify */
    void checkConfigFile();

    /** Scans the block files and builds a new index */
    void rebuildChainIndex();

    /** Checks if sufficient disk space is available for the parser */
    void checkDiskSpace();

    /** Checks if the process can open at least MIN_OPEN_FILES file descriptors */
    void checkOpenFilesLimit();

    /** Prints the results of running the checks */
    void printResults();

    /** Resets the warning and error counters to zero */
    void reset();
};

#endif /* doctor_hpp */
