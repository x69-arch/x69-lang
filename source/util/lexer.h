#pragma once

#include <unordered_map>
#include <string>
#include <cassert>
#include <map>
#include <optional>

namespace x69
{
	enum class token_type
	{
		tk_symbol,
		tk_operator,
		tk_expression,
		tk_scope,
		tk_eol
	};

	struct token
	{
		using enum token_type;

		std::string_view str() const noexcept
		{
			return this->str_;
		};
		token_type type() const noexcept
		{
			return this->type_;
		};



		explicit token(const std::string& _str, token_type _type) :
			str_{ _str }, type_{ _type }
		{};

		std::string str_;
		token_type type_ = token_type::tk_symbol;
	};

	struct ParseTree
	{
	private:
		struct Node
		{
			friend inline bool operator==(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator!=(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator<(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator>(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator<=(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator>=(const Node& _lhs, const Node& _rhs) = default;

			friend inline bool operator==(const Node& _lhs, char _rhs)
			{
				return _lhs.c == _rhs;
			};
			friend inline bool operator!=(const Node& _lhs, char _rhs)
			{
				return _lhs.c != _rhs;
			};
			friend inline bool operator==(char _lhs, const Node& _rhs)
			{
				return _rhs == _lhs;
			};
			friend inline bool operator!=(char _lhs, const Node& _rhs)
			{
				return _rhs != _lhs;
			};

			char c = 0;
			std::optional<token_type> type;
			std::map<char, Node> children;

		};

		Node* find_node(const std::string& _str)
		{
			auto _at = &this->root_;
			assert(_at);

			for (size_t i = 0; i < _str.size(); ++i)
			{
				if (_at->children.contains(_str[i]))
				{
					_at = &_at->children.at(_str[i]);
				}
				else
				{
					return nullptr;
				};
			};

			return _at;
		};
		const Node* find_node(const std::string& _str) const
		{
			auto _at = &this->root_;
			assert(_at);

			for (size_t i = 0; i < _str.size(); ++i)
			{
				if (_at->children.contains(_str[i]))
				{
					_at = &_at->children.at(_str[i]);
				}
				else
				{
					return nullptr;
				};
			};

			return _at;
		};

	public:

		bool insert(const std::string& _str, token_type _token)
		{
			Node* _at = &this->root_;

			for (size_t i = 0; i < _str.size(); ++i)
			{
				assert(_at);

				if (!_at->children.contains(_str[i]))
				{
					// Branch ends, extend it
					Node _newNode{};
					_newNode.c = _str[i];
					_at->children.insert({ _str[i], std::move(_newNode) });
				};

				_at = &_at->children.at(_str[i]);

			};

			assert(_at);
			if (_at->type)
			{
				return false;
			}
			else
			{
				_at->type = _token;
				return true;
			};

		};

		bool contains(const std::string& _str) const
		{
			return this->find_node(_str) != nullptr;
		};

		std::optional<token_type> find(const std::string& _str) const
		{
			auto _ptr = find_node(_str);
			if (_ptr)
			{
				return _ptr->type;
			}
			else
			{
				return std::nullopt;
			};
		};

		void clear()
		{
			this->root_.children.clear();
			this->root_.type = std::nullopt;
		};

	private:
		Node root_;
	};

	static inline const std::vector<char> FORCEFUL_TOKENS
	{
		'+', '=', '-', '<', '>', '!', '(', ')', '{', '}', ';', ',', '*', '@', '$'
	};

	struct lexer_context
	{
	public:
		auto parse_token(std::string& _str) const
		{
			return this->ptree_.find(_str);
		};

		ParseTree ptree_;
	};

	std::vector<x69::token> lex_tokens(const lexer_context& _context, const std::string& _str);

	void set_standard_tokens(lexer_context& _context);

	void strip_comments(std::string& _str);


};
