#pragma once

#include "util/format.h"

#include <jclib/ranges.h>
#include <jclib/type_traits.h>

#include <string>
#include <string_view>
#include <functional>

namespace jc
{
	template <typename T, typename ValT>
	constexpr inline auto contains(T&& _range, ValT&& _value) ->
		jc::enable_if_t<jc::ranges::is_range<T>::value&&
		jc::has_operator<jc::equals_t, jc::ranges::value_t<T>, ValT>::value, bool>
	{
		return std::find(jc::begin(_range), jc::end(_range), std::forward<ValT>(_value)) != jc::end(_range);
	};
};
