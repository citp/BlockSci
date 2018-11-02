//
//  generic_sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef generic_sequence_hpp
#define generic_sequence_hpp

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/transform.hpp>

#include <any>

constexpr ranges::category random_access_sized = ranges::category::random_access | ranges::category::sized;

template <ranges::category range_cat>
struct GenericSequence {
	virtual ranges::any_view<std::any, range_cat> getAnySequence() = 0;
	virtual ~GenericSequence() = default;
};

template <typename T>
struct Range : public GenericSequence<random_access_sized>, public ranges::any_view<T, random_access_sized> {
	using ranges::any_view<T, random_access_sized>::any_view;

	Range(ranges::any_view<T, random_access_sized> && r) : ranges::any_view<T, random_access_sized>(std::move(r)) {}

	Range(ranges::any_view<T, random_access_sized> & r) : ranges::any_view<T, random_access_sized>(r) {}

	ranges::any_view<std::any, random_access_sized> getAnySequence() override {
		return ranges::view::transform(*this, [](T && t) -> std::any {
			return std::move(t);
		});
	}
};

template <typename T>
struct Iterator : public GenericSequence<ranges::category::input>, public ranges::any_view<T> {
	using ranges::any_view<T>::any_view;

	Iterator(ranges::any_view<T> && r) : ranges::any_view<T>(std::move(r)) {}

	Iterator(ranges::any_view<T> &r) : ranges::any_view<T>(r) {}

	Iterator(ranges::any_view<T, random_access_sized> && r) : ranges::any_view<T>(std::move(r)) {}

	Iterator(ranges::any_view<T, random_access_sized> &r) : ranges::any_view<T>(r) {}

	ranges::any_view<std::any> getAnySequence() override {
		return ranges::view::transform(*this, [](T && t) -> std::any {
			return std::move(t);
		});
	}
};

template <typename T, ranges::category range_cat>
struct AnyViewType;

template <typename T>
struct AnyViewType<T, ranges::category::input> {
	using type = Iterator<T>;
};

template <typename T>
struct AnyViewType<T, random_access_sized> {
	using type = Range<T>;
};

template <typename T, ranges::category range_cat>
using any_view = typename AnyViewType<T, range_cat>::type;

#endif /* generic_sequence_hpp */
