#pragma once

#include "util/format.h"
#include "util/config.h"

#include <filesystem>
#include <fstream>

namespace x69
{
	using path = std::filesystem::path;

	inline std::filesystem::path current_path()
	{
		if constexpr (use_source_root_as_working_path_v)
		{
			return PROJECT_ROOT "/";
		}
		else
		{
			return std::filesystem::current_path();
		};
	};
};

namespace jc
{
	template <typename CharT>
	struct formatter<x69::path, CharT>
	{
		auto format(const x69::path& _path, auto& _ctx)
		{
			return std::format(_ctx.locale(), "{}", _path.string());
		};
	};
};

