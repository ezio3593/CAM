#pragma once

#include <sstream>
#include <deque>
#include <memory>
#include <algorithm>
#include <vector>

namespace CAM
{
	class CodeTerm
	{
	protected:
		char _op;
	public:
		typedef std::shared_ptr<CodeTerm> term_ptr;
		typedef std::deque<term_ptr> code_t;

		CodeTerm(char op) : _op(op) {}

		char op() const { return _op; }

		virtual std::string to_string() const { return std::string(1, _op); };
		virtual size_t args_count() const { return 0; }
		virtual ~CodeTerm() {}

		static CodeTerm::term_ptr make(char op) {
			return std::make_shared<CodeTerm>(op);
		}

		static std::string to_string(const code_t &t);

		friend std::ostream& operator<<(std::ostream& os, const CodeTerm& t) {
			return os << t.to_string();
		}

		friend std::ostream& operator<<(std::ostream& os, const code_t& c) {
			return os << CodeTerm::to_string(c);
		}
	};

	class CodeTermWithArgs : public CodeTerm
	{
	public:
		typedef std::vector<code_t> args_t;
	private:
		args_t _args;
	public:
		typedef std::shared_ptr<CodeTermWithArgs> term_ptr;

		CodeTermWithArgs(char op, args_t &args) : CodeTerm(op), _args(args) {}

		code_t& get_arg(size_t i) { return _args[i]; }

		virtual size_t args_count() const { return _args.size(); }

		virtual std::string to_string() const;

		static CodeTerm::term_ptr make(char op, args_t &args) {
			return std::dynamic_pointer_cast<CodeTerm>(std::make_shared<CodeTermWithArgs>(op, args));
		}
	};

	class QuoteCodeTerm : public CodeTerm
	{
		std::string _arg;
	public:
		typedef std::shared_ptr<QuoteCodeTerm> term_ptr;

		QuoteCodeTerm(const std::string &s) : CodeTerm('\''), _arg(s) {}

		std::string& get_arg() { return _arg; }

		virtual size_t args_count() const { return 1; }

		virtual std::string to_string() const { return _op + _arg; }

		static CodeTerm::term_ptr make(const std::string &s) {
			return std::dynamic_pointer_cast<CodeTerm>(std::make_shared<QuoteCodeTerm>(s));
		}
	};
}