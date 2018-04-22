//
//  script_access.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include <blocksci/core/script_access.hpp>
#include <blocksci/address/dedup_address_info.hpp>
#include <blocksci/util/data_configuration.hpp>

#include <boost/filesystem/path.hpp>

namespace blocksci {
    ScriptAccess::ScriptAccess(const DataConfiguration &config_) :
        scriptFiles(blocksci::apply(DedupAddressType::all(), [&] (auto tag) {
            return std::make_unique<ScriptFile<tag.value>>((boost::filesystem::path{config_.scriptsDirectory()}/std::string{dedupAddressName(tag)}).native());
        })) {}
} // namespace blocksci
