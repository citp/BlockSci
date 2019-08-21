//
//  doctor.hpp
//  blocksci_parser
//
//  Created by Malte Moeser on 8/22/19.
//

#ifndef doctor_hpp
#define doctor_hpp

#include "parser_configuration.hpp"
#include "chain_index.hpp"

#include <iostream>
#include <sys/resource.h>
#include <sys/statvfs.h>
#include <wjfilesystem/path.h>
#include <nlohmann/json.hpp>

class BlockSciDoctor {

    const filesystem::path configFilePath;
    const ParserConfigurationBase config;
    nlohmann::json jsonConf;

    int warnings = 0;
    int errors = 0;

    // Safe open files limit -- this is a guess and may need to be adjusted
    const rlim_t MIN_OPEN_FILES = 20000;

private:

    ParserConfigurationBase getBaseConfig(const filesystem::path &configPath) {
        if (!configPath.exists()) {
            throw std::runtime_error("Config path does not exist");
        }
        return {blocksci::loadBlockchainConfig(configPath.str(), true, 0)};
    }

    void printInfo(std::string str) {
        std::cout << str << std::endl;
    }

    void printWarning(std::string str) {
        std::stringstream output;
        output << "Warning: " << str << std::endl;
        std::cout << output.str();
    }

    void printError(std::string str) {
        std::stringstream output;
        output << "Error: " << str << std::endl;
        std::cout << output.str();
    }

    void printOk(std::string str) {
        std::stringstream output;
        output << "OK: " << str << std::endl;
        std::cout << output.str();
    }

public:

    BlockSciDoctor(filesystem::path _configFilePath) : configFilePath(_configFilePath), config(getBaseConfig(_configFilePath)), jsonConf(blocksci::loadConfig(configFilePath.str())) {
        blocksci::checkVersion(jsonConf);
    }

    void checkConfigFile() {
        printInfo("Checking configuration for issues.");

        blocksci::ChainConfiguration chainConfig = jsonConf.at("chainConfig");

        auto dataDirectory = chainConfig.dataDirectory;
        if(!dataDirectory.exists()) {
            printWarning("Data directory does not exist.");
            warnings += 1;

            if(!filesystem::create_directory(dataDirectory)) {
                std::stringstream ss;
                ss << "Cannot create directory for path " << dataDirectory << ". Check file permissions.";
                printError(ss.str());
                errors += 1;
            }
        } else {
            printOk("Found data directory on disk.");
        }

        // Parser
        auto parserConf = jsonConf.at("parser");
        bool hasRPCConfig = parserConf.find("rpc") != parserConf.end();
        bool hasDiskConfig = parserConf.find("disk") != parserConf.end();

        if (!hasRPCConfig && !hasDiskConfig) {
            std::cout << "Error: config contains neither disk nor RPC parsing settings." << std::endl;
            errors += 1;
        }
        if (hasDiskConfig){
            printOk("Found disk parser settings in config.");
        }
        if (hasRPCConfig) {
            printOk("Found RPC parser settings in config.");
        }

        // Disk parser
        if (hasDiskConfig) {
            ChainDiskConfiguration diskConfig = parserConf.at("disk");

            auto coinDirectory = diskConfig.coinDirectory;
            auto blockDirectory = coinDirectory/"blocks";

            if(!coinDirectory.exists()) {
                printError("Coin directory does not exist.");
                errors += 1;
            } else if(coinDirectory.exists() && !blockDirectory.exists()) {
                printError("Coin directory does not contain blocks subdirectory.");
                errors += 1;
            }
        }
    }

    void rebuildChainIndex() {
        blocksci::ChainConfiguration chainConfig = jsonConf.at("chainConfig");
        auto parserConf = jsonConf.at("parser");

        blocksci::BlockHeight maxBlock = parserConf.at("maxBlockNum");
        ChainDiskConfiguration diskConfig = parserConf.at("disk");

        blocksci::DataConfiguration dataConfig{configFilePath.str(), chainConfig, true, 0};
        ParserConfiguration<FileTag> config{dataConfig, diskConfig};

        ChainIndex<FileTag> index = ChainIndex<FileTag>{};

        printInfo("Constructing chain index from scratch. This may take a few minutes.");

        index.update(config, maxBlock);
        auto blocks = index.generateChain(maxBlock);
        if(blocks.size() > 0) {
            auto lastBlock = blocks.back();

            std::stringstream ss;
            ss << "Most recent block found in coin directory has height " << lastBlock.height << " (in file " << lastBlock.nFile << ")";
            printOk(ss.str());

            if(maxBlock > 0 && (unsigned int) maxBlock > blocks.size()) {
                printWarning("MaxBlock in config is larger than number of blocks found on disk.");
                warnings += 1;
            }
        } else {
            printError("No blocks found in block files.");
            errors += 1;
        }
    }

    void checkDiskSpace() {
        blocksci::ChainConfiguration chainConfig = jsonConf.at("chainConfig");
        auto dataDirectory = chainConfig.dataDirectory;

        struct statvfs stats;
        statvfs(dataDirectory.str().c_str(), &stats);
        auto diskSize = stats.f_frsize * stats.f_bavail / (1024*1024*1024);

        if(diskSize < 20) {
            std::stringstream ss;
            ss << "Available disk space is getting low (" << diskSize << "GB).";
            printWarning(ss.str());
            warnings += 1;
        } else {
            std::stringstream ss;
            ss << diskSize << "GB of free disk space available.";
            printInfo(ss.str());
        }
    }

    void checkOpenFilesLimit() {
        struct rlimit rlim;
        getrlimit(RLIMIT_NOFILE, &rlim);
        rlim_t openFileLimit = rlim.rlim_cur;
        if(openFileLimit < MIN_OPEN_FILES) {
            std::stringstream ss;
            ss << "Open files limit of " << openFileLimit << " is potentially too low. This could lead to data corruption.";
            printWarning(ss.str());
            warnings += 1;
        } else {
            std::stringstream ss;
            ss << "Open files limit of " << openFileLimit << ".";
            printOk(ss.str());
        }
    }

    void printResults() {
        if(warnings + errors > 0) {
            std::cout << "Found " << warnings << " warnings and " << errors << " errors." << std::endl;
            std::cout << "Warnings may be bening, errors must be resolved for BlockSci to run." << std::endl;
        } else {
            std::cout << "No significant issues detected." << std::endl;
        }
    }

    void reset() {
        warnings = 0;
        errors = 0;
    }
};

#endif /* doctor_hpp */
