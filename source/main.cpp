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
		sm_typename,
		sm_assignment,
		sm_literal,
		sm_unary_op,
		sm_binary_op,
		sm_unary_assignment_op,
		sm_binary_assignment_op
	};

	struct symbol
	{
		jc::reference_ptr<x69::token> token;
		symbol_type type;
		void* spec = nullptr;
	};

	using statement = std::vector<symbol>;







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
			x69::lexer_context _context{};
			x69::set_standard_tokens(_context);

			x69::strip_comments(_source);
			_tokens = x69::lex_tokens(_context, _source);
		};
	};


	x69::compiler_context _context{};

	std::vector<x69::statement> _statements{ x69::statement{} };
	for (auto it = _tokens.begin(); it != _tokens.end(); ++it)
	{
		auto& v = *it;

		using enum x69::token_type;
		switch (v.type())
		{
		case tk_symbol:
		{
			x69::symbol_type _type = x69::symbol_type::sm_name;

			auto& _types = _context.types_;
			auto _typeIter = _types.find(v.str_);

			void* _spec = nullptr;

			if (_typeIter != _types.end())
			{
				_type = x69::symbol_type::sm_typename;
				_spec = &_typeIter->second;
			}
			else
			{
				auto _tokenStr = v.str();
				if (std::isdigit(_tokenStr.front()))
				{
					_type = x69::symbol_type::sm_literal;
				}
				else
				{
					_type = x69::symbol_type::sm_name;
				};
			};

			_statements.back().push_back(x69::symbol{ v, _type, _spec });
		};
			break;
		case tk_operator:
		{
			using enum x69::symbol_type;

			x69::symbol_type _type = sm_unary_op;
			if (v.str() == "=")
			{
				_type = sm_assignment;
			}
			else if (v.str() == "+")
			{
				_type = sm_binary_op;
			}
			else if (v.str() == "-")
			{
				_type = sm_binary_op;
			}
			else if (v.str() == "+=")
			{
				_type = sm_binary_assignment_op;
			}
			else if (v.str() == "-=")
			{
				_type = sm_binary_assignment_op;
			}
			else if (v.str() == "++")
			{
				_type = sm_unary_assignment_op;
			}
			else if (v.str() == "--")
			{
				_type = sm_unary_assignment_op;
			};

			x69::symbol _symbol{ v, _type };
			_statements.back().push_back(std::move(_symbol));
		};
			break;
		case tk_eol:
			_statements.push_back(x69::statement{});
			break;
		default:
			break;
		};
		
	};


	std::ofstream _ostr{ PROJECT_ROOT "/slime.x69" };
	jc::formatted_ostream fout{ _ostr.rdbuf() };

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




	return 0;
};

