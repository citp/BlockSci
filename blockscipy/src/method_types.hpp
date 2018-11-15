//
//  method_types.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/27/18.
//

#ifndef method_types_h
#define method_types_h

#include <typeindex>
#include <string>
#include <unordered_map>

std::string getTypeName(const char *text, const std::type_info *const *types);

struct TypenameLookup {
    std::unordered_map<std::type_index, std::string> typeNames;
    std::unordered_map<std::type_index, std::string> docstringTypeNames;

    TypenameLookup();
    
    template <typename T>
    std::string getName() const {
    	return typeNames.at(std::type_index(typeid(T)));
    }

    std::string getDocstringName(const std::type_info &info) const {
    	auto it = docstringTypeNames.find(std::type_index(info));
    	if (it != docstringTypeNames.end()) {
    		return it->second;
    	}
    	return typeNames.at(std::type_index(info)); 
    }
};

const TypenameLookup &typenameLookup();

#endif // method_types_h