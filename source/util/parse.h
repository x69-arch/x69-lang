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

namespace x69
{

		constexpr auto make_formatter = [](std::string_view _fmt)
		{
			const std::string _fmtStr{ _fmt };
			return [_fmtStr](const auto&... _vals)
			{
				return std::format(_fmtStr, _vals...);
			};
		};

		template <typename T>
		concept cx_match_fn = requires (T a, char c)
		{
			{ std::invoke(a, c) } -> std::same_as<bool>;
		};

		struct match_class_t
		{
			constexpr auto operator()(std::string_view _in, cx_match_fn auto _charclass) const
			{
				// find first character that is not in char class, inverse if needed

				auto it = _in.begin();
				while (it != _in.end() && std::invoke(_charclass, *it))
				{
					++it;
				};

				const auto _endPos = it - _in.begin();

				return _in.substr(0, _endPos);
			};
			constexpr auto operator()(cx_match_fn auto _charclass) const
			{
				return [_charclass](const std::string_view _str)
				{
					return match_class_t{}(_str, _charclass);
				};
			};
		};

		constexpr inline match_class_t match_class{};

		struct ignore_t
		{
			constexpr auto operator()(std::string_view _in, std::string_view _ignoreChars) const
			{
				const auto _pos = std::min(_in.size(), _in.find_first_not_of(_ignoreChars));
				const auto _len = _in.size() - _pos;
				return _in.substr(_pos, _len);
			};
			auto operator()(std::string _chars) const
			{
				return [_chars](const std::string_view _str)
				{
					return ignore_t{}(_str, _chars);
				};
			};
		};

		constexpr inline ignore_t ignore{};

};