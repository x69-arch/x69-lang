#pragma once

#include "util/format.h"

#include <ostream>

namespace jc
{
	/**
	 * @brief Provides a convenience interface for writing a formatted string to an output stream
	*/
	struct formatted_ostream
	{
	public:

		void write(std::string_view _fmt, const auto&... _val)
		{
			this->ostr_ << std::format(_fmt, _val...);
		};
		auto operator()(std::string_view _fmt, const auto&... _val)
		{
			return this->write(_fmt, _val...);
		};
		




		formatted_ostream() = default;
	
		formatted_ostream(std::streambuf* _buff) :
			ostr_{ _buff }
		{};
		formatted_ostream(std::ostream& _ostr) :
			formatted_ostream{ _ostr.rdbuf()  }
		{};

	private:
		std::ostream ostr_;
	};

};
