#pragma once

#include <jclib/memory.h>

#include <ranges>
#include <unordered_map>
#include <string>
#include <cassert>
#include <map>
#include <set>
#include <optional>
#include <string_view>

namespace x69
{


	enum class token_type
	{
		tk_symbol,
		tk_declaration,
		tk_assignment,
		tk_unary_operator,
		tk_binary_operator,
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

		Node* find_node(std::string_view _str)
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
		const Node* find_node(std::string_view _str) const
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

		bool insert(std::string_view _str, token_type _token)
		{
			Node* _at = &this->root_;

			for (auto& c : _str)
			{
				assert(_at);

				if (!_at->children.contains(c))
				{
					// Branch ends, extend it
					Node _newNode{};
					_newNode.c = c;
					_at->children.insert({ c, std::move(_newNode) });
				};

				_at = &_at->children.at(c);
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

		std::optional<token_type> find(std::string_view _str) const
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





	template <typename T>
	struct basic_lexer_syntax
	{
	public:
		using character_type = T;
		using string_view_type = std::basic_string_view<character_type>;


		/**
		 * @brief Checks if a character is a forceful stop token
		 * @param _char 
		 * @return 
		*/
		bool is_forcefull(character_type _char) const noexcept
		{
			return this->forceful_characters_.contains(_char);
		};

		/**
		 * @brief Sets a character as a forceful stop point when lexing
		 * @param _char 
		*/
		void add_forcefull(character_type _char)
		{
			this->forceful_characters_.insert(_char);
		};


		template <std::ranges::range _T>
		requires std::convertible_to<std::ranges::range_value_t<_T>, character_type>
		void add_forcefull(const _T& _chars)
		{
			for (auto& c : _chars)
			{
				this->add_forcefull(c);
			};
		};



		void insert(string_view_type _token, token_type _type)
		{
			this->ptree_.insert(_token, _type);
		};


		std::optional<token_type> token(std::string_view _tk) const
		{
			return this->ptree_.find(_tk);
		};









	private:
		ParseTree ptree_;
		std::set<character_type> forceful_characters_{};
	};

	using lexer_syntax = basic_lexer_syntax<char>;




	// 	'+', '=', '-', '<', '>', '!', '(', ')', '{', '}', ';', ',', '*', '@', '$'

	struct lexer
	{
	public:
		const lexer_syntax& syntax() const noexcept
		{
			return *this->syntax_;
		};

	


		lexer(jc::reference_ptr<const lexer_syntax> _syntax) :
			syntax_{ _syntax }
		{};

	private:
		jc::reference_ptr<const lexer_syntax> syntax_;
	};


	std::vector<x69::token> lex_tokens(const lexer& _context, const std::string& _str);

	void set_standard_tokens(lexer_syntax& _context);

	void strip_comments(std::string& _str);


};
