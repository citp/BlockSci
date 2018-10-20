//
//  parser_fwd.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/27/17.
//

#ifndef parser_fwd_hpp
#define parser_fwd_hpp

#include "config.hpp"

#include <blocksci/core/address_types.hpp>

#ifdef BLOCKSCI_FILE_PARSER
struct FileTag{};
struct ChainDiskConfiguration;
#endif

#ifdef BLOCKSCI_RPC_PARSER
struct RPCTag{};
#endif

template <typename ParseType>
struct BlockInfo;

struct RawOutputPointer;
struct UTXO;

template<typename Key, typename Value>
class SerializableMap;

class UTXOState;
class UTXOScriptState;

struct RawTransaction;
struct BlockInfoBase;
class UTXOAddressState;
class AddressState;
class AddressWriter;

struct RawTransaction;
struct RawInput;
struct RawOutput;
struct WitnessStackItem;

template<blocksci::AddressType::Enum>
struct ScriptOutput;

class AnyScriptOutput;

template<blocksci::AddressType::Enum>
struct ScriptInput;

class AnyScriptInput;

template<blocksci::AddressType::Enum>
struct ScriptOutputData;

template<blocksci::AddressType::Enum>
struct ScriptInputData;

template<blocksci::AddressType::Enum>
struct SpendData;

class AnySpendData;

struct ParserConfigurationBase;
template <typename ParseType>
struct ParserConfiguration;

#endif /* parser_fwd_hpp */
