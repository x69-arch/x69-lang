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
	public:
		struct arguement
		{
			std::optional<token_type> token;
			bool variadic = false;
		};

	private:
		using container_type = std::vector<arguement>;

	public:
		using iterator = container_type::iterator;
		using const_iterator = container_type::const_iterator;

		iterator begin() noexcept { return this->args_.begin(); };
		const_iterator begin() const noexcept { return this->args_.cbegin(); };
		const_iterator cbegin() const noexcept { return this->args_.cbegin(); };
		
		iterator end() noexcept { return this->args_.end(); };
		const_iterator end() const noexcept { return this->args_.cend(); };
		const_iterator cend() const noexcept { return this->args_.cend(); };


	
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

extern inline jc::formatted_ostream cout{ std::cout };

namespace x69
{

	// ... tk_symbol tk_assignment ... tk_eol

	using pattern_view = jc::ranges::iter_view<x69::token_pattern::const_iterator>;

	pattern_view next_pattern_sequence(pattern_view _pattern)
	{
		auto _outBegin = _pattern.begin();
		for (_outBegin; _outBegin != _pattern.end(); ++_outBegin)
		{
			if (!_outBegin->variadic)
			{
				break;
			};
		};

		auto _outEnd = _outBegin;
		for (_outEnd; _outEnd != _pattern.end(); ++_outEnd)
		{
			if (_outEnd->variadic)
			{
				break;
			};
		};

		return pattern_view{ _outBegin, _outEnd };
	};


	using statement_match = std::vector<x69::statement::const_iterator>;
	using statement_view = jc::ranges::iter_view<x69::statement::const_iterator>;

	constexpr auto find_pattern = [](const x69::statement& _st, const x69::token_pattern& _pattern)
	{
		const auto _patternView = pattern_view{ _pattern.begin(), _pattern.end() };
		


		statement_view _statement{ _st.begin(), _st.end() };
	
		// ... tk_symbol tk_assignment ... tk_eol

		statement_match _matched{};

		pattern_view _previousSeq = pattern_view{ _pattern.begin(), _pattern.end() };
		pattern_view _sequence = next_pattern_sequence(_patternView);

		while (_sequence.size() != 0)
		{
			auto it = _statement.begin();
			for (auto& _seq : _sequence)
			{

				for (it; it != _statement.end(); ++it)
				{

					if (!_seq.token || (_seq.token.value() == it->type()))
					{
						_matched.push_back(it);
						++it;
						if (it == _st.end())
						{
							break;
						};
					}
					else
					{
						return statement_match{};
					};
				};

				
			};

			_sequence = next_pattern_sequence(pattern_view{ _sequence.end(), _pattern.end() });
		};

		return _matched;
	};

	auto parse_statement(const statement& _statement, const std::ranges::range auto& _patterns)
	{
		auto& st = _statement;

		for (auto& pt : _patterns)
		{
			auto _matched = find_pattern(st, *pt);
			if (!_matched.empty())
			{
				for (auto& a : _matched)
				{
					if (a->type() != x69::token_type::tk_eol)
					{
						cout("{} ", a->str());
					};
				};
				cout("\n");
			};
		};

	};

};

int main()
{

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
				.token = std::nullopt,
				.variadic = true,
			},
			{
				.token = x69::token_type::tk_symbol
			},
			{
				.token = x69::token_type::tk_assignment
			},
			{
				.token = std::nullopt,
				.variadic = true
			},
			{
				.token = x69::token_type::tk_eol,
			}
		};
	};



	std::vector<jc::reference_ptr<x69::token_pattern>> _patterns
	{
		_vardeclPattern,
		_assignmentPattern
	};

	for (auto& st : _statements)
	{
		parse_statement(st, _patterns);
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

