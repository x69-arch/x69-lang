#include "util/fmtstream.h"
#include "util/filesystem.h"

#include "util/lexer.h"
#include "util/parse.h"

#include <jclib/algorithm.h>

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <stack>
#include <optional>
#include <list>



size_t target_word_size()
{

	return 2;
};






namespace x69
{
	struct type_spec
	{
		size_t size = 0;
	};

	struct variable_spec
	{
		jc::reference_ptr<type_spec> type;
		size_t address = 0;
	};



	enum symbol_type
	{
		sm_name,
		sm_declaration,
		sm_typename,
		sm_assignment,
		sm_literal,
		sm_unary_op,
		sm_binary_op
	};





	struct symbol
	{
		jc::reference_ptr<const x69::token> token;
		symbol_type type;
		void* spec = nullptr;
	};



	using statement = std::vector<x69::token>;






	// 
	//  tk_declaration tk_symbol tk_symbol tk_eol;
	//  tk_declaration tk_symbol (tk_symbol tk_assignment ...)
	//

	struct token_pattern
	{
		struct arguement
		{
			std::optional<token_type> token;
			bool variadic = false;
		};

		std::vector<arguement> args_;
	};
















	struct compiler_context
	{
		size_t memory_top_ = 0;

		std::unordered_map<std::string, variable_spec> variables_
		{};
		std::unordered_map<std::string, type_spec> types_
		{
			{ "int", type_spec{ target_word_size() } },

			{ "i8", type_spec{ 1 } },
			{ "i16", type_spec{ 2 } },
			{ "i32", type_spec{ 4 } },

			{ "uint", type_spec{ target_word_size() } },

			{ "u8", type_spec{ 1 } },
			{ "u16", type_spec{ 2 } },
			{ "u32", type_spec{ 4 } }
		};
	};


};

namespace x69
{

	constexpr auto find_pattern = [](const x69::statement& _st, const x69::token_pattern& _pattern)
	{
		auto _ptIter = _pattern.args_.begin();
		auto _stIter = _st.begin();

		for (; _ptIter != _pattern.args_.end() && _stIter != _st.end(); ++_ptIter, ++_stIter)
		{
			if (_ptIter->token)
			{
				if (_ptIter->token.value() != _stIter->type())
				{
					return false;
				};
			}
			else
			{
				if (_ptIter->variadic)
				{
					break;
				};
			};
		};
		return true;
	};
	constexpr auto match_pattern = [](const x69::statement& _st, const x69::token_pattern& _pattern)
	{
		auto& _args = _pattern.args_;
		auto _varargIter = std::ranges::find_if(_args, [](const auto& _arg) -> bool
			{
				return _arg.variadic;
			});

		if (_varargIter == std::ranges::end(_args))
		{
			return std::ranges::end(_st);
		};

		const auto _dist = std::distance(std::ranges::begin(_args), _varargIter);
		if (_dist > _st.size())
		{
			return _st.end();
		};
		return std::next(_st.begin(), _dist);
	};



	auto parse_statement(const statement& _statement, const std::ranges::range auto& _patterns)
	{
		auto& st = _statement;

		for (auto& pt : _patterns)
		{
			if (find_pattern(st, pt))
			{
				const auto _varargIter = match_pattern(st, *pt);
				for (auto& a : jc::views::iter(st.cbegin(), _varargIter))
				{
					if (a.type() != x69::token_type::tk_eol)
					{
						cout("{} ", a.str());
					};
				};
				cout("\n");

				if (_varargIter != st.end())
				{
					// process varaidic arguements

				};
			};
		};

	};

};

int main()
{
	jc::formatted_ostream cout{ std::cout };

	std::vector<x69::token> _tokens{};
	{
		std::string _source{};
		{
			char _buffer[512]{ 0 };
			std::ranges::fill(_buffer, 0);

			std::ifstream _sourceFile{ PROJECT_ROOT "/source/test.cb" };
			while (_sourceFile)
			{
				_sourceFile.read(_buffer, 512);
				_source.append(_buffer, _sourceFile.gcount());
			};
		};

		{
			x69::lexer_syntax _syntax{};
			x69::lexer _lexer{ _syntax };

			x69::set_standard_tokens(_syntax);

			x69::strip_comments(_source);
			_tokens = x69::lex_tokens(_lexer, _source);
		};
	};

	std::vector<x69::statement> _statements{};
	{
		x69::statement _thisStatement{};
		for (auto it = _tokens.begin(); it != _tokens.end(); ++it)
		{
			_thisStatement.push_back(*it);
			if (it->type() == x69::token_type::tk_eol)
			{
				_statements.push_back(_thisStatement);
				_thisStatement.clear();
			};
		};
	};


	x69::token_pattern _vardeclPattern{};
	x69::token_pattern _assignmentPattern{};

	{
		using arg_t = x69::token_pattern::arguement;
		_vardeclPattern.args_ =
		{
			{
				.token = x69::token_type::tk_declaration
			},
			{
				.token = x69::token_type::tk_symbol
			},
			{
				.token = x69::token_type::tk_symbol
			},
			{
				.token = std::nullopt,
				.variadic = true
			}
		};
		_assignmentPattern.args_ =
		{
			{
				.token = x69::token_type::tk_assignment
			},
			{
				.token = std::nullopt,
				.variadic = true
			}
		};
	};



	std::vector<jc::reference_ptr<x69::token_pattern>> _patterns
	{
		_vardeclPattern,
		_assignmentPattern
	};




	x69::compiler_context _context{};

	std::ofstream _ostr{ PROJECT_ROOT "/slime.x69" };
	jc::formatted_ostream fout{ _ostr.rdbuf() };










	/*


	for (auto& v : _statements)
	{
		using enum x69::symbol_type;

		if (v.empty())
		{
			continue;
		};

		if (v.size() == 2)
		{
			// boom variable declaration
			if (v[0].type == sm_typename && v[1].type == sm_name)
			{
				auto& _typeSpec = *static_cast<x69::type_spec*>(v[0].spec);
				x69::variable_spec _var
				{
					_typeSpec,
					_context.memory_top_
				};

				_context.memory_top_ += _typeSpec.size;
				_context.variables_.insert({ v[1].token->str_, _var });
			};

			if (v[0].type == sm_unary_assignment_op && v[1].type == sm_name)
			{
				auto& _variable = _context.variables_.at(v[1].token->str_);
				auto _registerNumber = _variable.address / target_word_size();

				if (v[0].token->str_ == "++")
				{
					fout("inc r{}\n", _registerNumber);
				}
				else if (v[0].token->str_ == "--")
				{
					fout("dec r{}\n", _registerNumber);
				};
			};
		};

		if (v.size() >= 3)
		{
			if (v[0].type == sm_name && v[1].type == sm_assignment)
			{
				auto& _variable = _context.variables_.at(v[0].token->str_);
				auto _registerNumber = _variable.address / target_word_size();

				if (v.size() == 3)
				{
					if (v[2].type == sm_literal)
					{
						fout("set r{}, {}\n", _registerNumber, v[2].token->str());
					}
					else if (v[2].type == sm_name)
					{
						auto _sourcevar = _context.variables_.at(v[2].token->str_);
						auto _sourcereg = _sourcevar.address / target_word_size();
						fout("mov r{}, r{}\n", _registerNumber, _sourcereg);
					};
				};

				if (v.size() == 4)
				{

				};

			};

		};


	};
	*/




	return 0;
};

