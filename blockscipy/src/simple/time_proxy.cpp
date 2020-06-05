//
//  time_proxy.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "simple_proxies.hpp"
#include "proxy/range.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/basic.hpp"
#include "proxy/optional.hpp"
#include "method_tags.hpp"
#include "proxy_apply_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

#include <date/date.h>

struct AddTimeMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        namespace py = pybind11;
        
        func(property_tag, "year", +[](std::chrono::system_clock::time_point time) -> int64_t {
            auto dp = date::floor<date::days>(time);
    		auto ymd = date::year_month_day{dp};
            return static_cast<int>(ymd.year());
        }, "The year of the date");
        
        func(property_tag, "month", +[](std::chrono::system_clock::time_point time) -> int64_t {
            auto dp = date::floor<date::days>(time);
    		auto ymd = date::year_month_day{dp};
            return static_cast<unsigned>(ymd.month());
        }, "The month of the date");

        func(property_tag, "day", +[](std::chrono::system_clock::time_point time) -> int64_t {
            auto dp = date::floor<date::days>(time);
    		auto ymd = date::year_month_day{dp};
            return static_cast<unsigned>(ymd.day());
        }, "The day of the date");

        func(property_tag, "hour", +[](std::chrono::system_clock::time_point tp) -> int64_t {
            auto dp = date::floor<date::days>(tp);
    		auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp - dp));
            return time.hours().count();
        }, "The hour of the date");

        func(property_tag, "minute", +[](std::chrono::system_clock::time_point tp) -> int64_t {
            auto dp = date::floor<date::days>(tp);
    		auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp - dp));
            return time.minutes().count();
        }, "The minute of the date");

        func(property_tag, "second", +[](std::chrono::system_clock::time_point tp) -> int64_t {
            auto dp = date::floor<date::days>(tp);
    		auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp - dp));
            return time.seconds().count();
        }, "The second of the date");
    }
};

void addTimeProxyMethods(AllProxyClasses<std::chrono::system_clock::time_point> &cls) {
	cls.applyToAll(AddProxyMethods{});
	addProxyEqualityMethods(cls.base);
	addProxyComparisonMethods(cls.base);
	addProxyOptionalMethods(cls.optional);

	applyMethodsToProxy(cls.base, AddTimeMethods{});
}