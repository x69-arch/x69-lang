#pragma once

#include <jclib/type_traits.h>
#include <jclib/ranges.h>

#include <format>
#include <ranges>
#include <type_traits>
#include <array>
#include <vector>

namespace jc
{
	/**
	 * @brief Custom formatter redirection, specialize to implement
	 * @tparam T Type to format
	 * @tparam CharT Character type
	*/
	template <typename T, typename CharT>
	struct formatter;

	namespace impl
	{
		/**
		 * @brief Fufilled if the ccap::formatter for T has a parse function
		*/
		template <typename T, typename CharT>
		concept parsing_formatter = requires (formatter<T, CharT> f, std::basic_format_parse_context<CharT> ctx)
		{
			f.parse(ctx);
		};
	};

	/**
	 * @brief Fufilled if T has a ccap::formatter specialization
	*/
	template <typename T, typename CharT>
	concept formattable = requires(T val)
	{
		std::format("{}", val);
		//std::formatter<std::remove_cvref_t<T>, CharT>{};
	};

};

namespace std
{
	/**
	 * @brief Formatter redirection for implementing custom formatters
	 * @tparam T Type with ccap::formatter specialization
	 * @tparam CharT Character type
	*/
	template <typename T, typename CharT>
	requires requires
	{
		jc::formatter<jc::remove_cvref_t<T>, CharT>{};
	}
	struct formatter<T, CharT> : public std::formatter<std::string_view, CharT>
	{
		using custom_type = jc::formatter<jc::remove_cvref_t<T>, CharT>;

		template <typename ContextT>
		auto parse(ContextT& _ctx)
		{
			if constexpr (jc::impl::parsing_formatter<jc::remove_cvref_t<T>, CharT>)
			{
				const auto it = _ctx.begin();
				if (it != _ctx.end())
				{
					return this->custom_.parse(_ctx);
				}
				else
				{
					return it;
				};
			}
			else
			{
				return std::formatter<std::string_view, CharT>::parse(_ctx);
			};
		};

		template <typename ContextT>
		auto format(const jc::remove_cvref_t<T>& _p, ContextT& _ctx)
		{
			return std::formatter<std::string_view, CharT>::format(this->custom_.format(_p, _ctx), _ctx);
		};

	private:
		custom_type custom_{};
	};
}

namespace jc
{
	/**
	 * @brief Applies a formatting string to each value in a range and concats the results
	 * @tparam RangeT Range type
	 * @param _fmt Formatting string
	 * @param _vals Range value to fold
	 * @return Formatted string
	*/
	template <jc::cx_range RangeT>
	requires formattable<jc::ranges::value_t<RangeT>, char>
		constexpr auto format_fold(std::string_view _fmt, const RangeT& _vals)
	{
		std::string _out{};
		for (auto& v : _vals)
		{
			_out.append(std::format(_fmt, v));
		};
		return _out;
	};

};

