//
//  gtest_main.cpp
//  blocksci_unittest
//
//  Created by Malte Möser on 4/28/20.
//

#include "gtest/gtest.h"
#include <clipp.h>

std::string configFilePath;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    auto cli = clipp::group(
        clipp::value("config file location", configFilePath) % "Path to config file"
    );

    auto res = parse(argc, argv, cli);
    if (res.any_error()) {
        std::cout << "Invalid command line parameter\n" << clipp::make_man_page(cli, argv[0]);
        return 0;
    }
    return RUN_ALL_TESTS();
}
