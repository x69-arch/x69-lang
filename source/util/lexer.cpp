#include "lexer.h"

#include <array>

namespace x69
{
	bool is_any_of(char _c, const std::vector<char>& _of)
	{
		return std::find(_of.begin(), _of.end(), _c) != _of.end();
	};



	std::vector<x69::token> lex_tokens(const lexer& _context, const std::string& _str)
	{
		std::vector<x69::token> _out{};
		size_t _lastStart = 0;
		bool _readingToken = false;

		const auto& _syntax = _context.syntax();

		for (auto i = 0; i < _str.size(); ++i)
		{
			if (!_readingToken)
			{
				if (!std::isspace(_str[i]))
				{
					_readingToken = true;
					_lastStart = i;
				}
				else
				{
					continue;
				};
			};

			if (_syntax.is_forcefull(_str[i]))
			{
				if (_lastStart != i)
				{
					auto _tstr = _str.substr(_lastStart, (i - _lastStart));
					auto _tokenOpt = _syntax.token(_tstr);
					_out.push_back(token{ std::move(_tstr), _tokenOpt.value_or(token_type::tk_symbol) });
				};

				auto _forceFull = std::string{ _str[i] };
				if (i + 1 < _str.size() && _syntax.is_forcefull(_str[i]))
				{
					_forceFull.push_back(_str[i + 1]);
					if (_syntax.token(_forceFull))
					{
						++i;
					}
					else
					{
						_forceFull.pop_back();
					};
				};
				auto _ffOpt = _syntax.token(_forceFull);
				_out.push_back(token{ std::move(_forceFull), _ffOpt.value_or(token_type::tk_symbol) });

				_readingToken = false;
			}
			else if (std::isspace(_str[i]))
			{
				auto _tstr = _str.substr(_lastStart, i - _lastStart);
				auto _tokenOpt = _syntax.token(_tstr);
				_out.push_back(token{ std::move(_tstr), _tokenOpt.value_or(token_type::tk_symbol) });
				_readingToken = false;
			};

		};

		if (_readingToken)
		{
			auto _tstr = _str.substr(_lastStart, _str.size() - _lastStart);
			auto _tokenOpt = _syntax.token(_tstr);
			_out.push_back(token{ std::move(_tstr),  _tokenOpt.value_or(token_type::tk_symbol) });
		};

		return _out;

	};

	void set_standard_tokens(lexer_syntax& _context)
	{
		constexpr auto forceful_tokens_v = std::array
		{
			'+', '=', '-', '<', '>', '!', '(', ')', '{', '}', ';', ',', '*', '@', '$'
		};

		_context.add_forcefull(forceful_tokens_v);

		_context.insert("let", token_type::tk_declaration);

		_context.insert("=", token_type::tk_assignment);



		_context.insert("+", token_type::tk_binary_operator);
		_context.insert("+=", token_type::tk_binary_operator);
		_context.insert("++", token_type::tk_unary_operator);

		_context.insert("-", token_type::tk_binary_operator);
		_context.insert("-=", token_type::tk_binary_operator);
		_context.insert("--", token_type::tk_unary_operator);

		_context.insert(">", token_type::tk_binary_operator);
		_context.insert(">=", token_type::tk_binary_operator);

		_context.insert("<", token_type::tk_binary_operator);
		_context.insert("<=", token_type::tk_binary_operator);

		_context.insert("!=", token_type::tk_binary_operator);
		_context.insert("==", token_type::tk_binary_operator);

		_context.insert("!", token_type::tk_unary_operator);

		_context.insert(",", token_type::tk_binary_operator);
		_context.insert("*", token_type::tk_binary_operator);

		_context.insert("@", token_type::tk_unary_operator);
		_context.insert("$", token_type::tk_unary_operator);

		_context.insert("(", token_type::tk_scope);
		_context.insert(")", token_type::tk_scope);

		_context.insert("{", token_type::tk_scope);
		_context.insert("}", token_type::tk_scope);
		
		_context.insert(";", token_type::tk_eol);

	};


	void strip_comments(std::string& _str)
	{
		while (true)
		{
			auto _at = _str.find('/');
			size_t _endPos = _at;
			if (_at >= _str.size() - 1)
			{
				break;
			};

			switch (_str[_at + 1])
			{
			case '/':
				// Single line comment
				_endPos = _str.find('\n', _at);
				_str.erase(_at, _endPos - _at);
				break;

			case '*':
				// Block comment
				_endPos = _str.find("*/", _at);
				_str.erase(_at, _endPos);
				break;

			default:
				break;
			};
		};
	};

};
