//
//  method_tags.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/27/18.
//

#ifndef method_tags_h
#define method_tags_h

struct property_tag_type {};
struct method_tag_type {};

static constexpr property_tag_type property_tag = property_tag_type{};
static constexpr method_tag_type method_tag = method_tag_type{};


#endif // method_tags_h