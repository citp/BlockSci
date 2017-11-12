//
//  parser_fwd.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/27/17.
//

#ifndef parser_fwd_hpp
#define parser_fwd_hpp

struct FileTag{};
struct RPCTag{};

template <typename ParseType>
struct BlockInfo;

struct RawTransaction;
struct BlockInfoBase;
class UTXOState;
class UTXOAddressState;
class AddressState;
class AddressWriter;

struct RawTransaction;
struct RawInput;
struct RawOutput;
struct WitnessStackItem;

template<auto>
struct ScriptOutput;

class AnyScriptOutput;

template<auto>
struct ScriptInput;

class AnyScriptInput;

template<auto>
struct ScriptData;

template<auto>
struct ScriptInputData;

template<auto>
struct SpendData;

class AnySpendData;

struct ParserConfigurationBase;
template <typename ParseType>
struct ParserConfiguration;

#endif /* parser_fwd_hpp */
