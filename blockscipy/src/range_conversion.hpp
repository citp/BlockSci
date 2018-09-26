//
//  range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_conversion_h
#define range_conversion_h

#include "simplify_range.hpp"
#include "python_range_conversion.hpp"

template <typename T>
auto convertRangeToPython(T && t) {
    return convertPythonRange(simplifyRange(std::move(t)));
}

#endif /* range_conversion_h */
