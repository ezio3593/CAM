#pragma once

#include <string>
#include <deque>
#include <map>
#include <functional>
#include <chrono>

#include "code.h"
#include "term.h"

namespace CAM
{
	typedef std::deque<Term::term_ptr> stack_t;
	typedef std::function<void(Term::term_ptr&, CodeTerm::code_t&, stack_t&)> transition_t;
	typedef std::function<mpz_class(const mpz_class&, const mpz_class&)> binary_operation_t;
	typedef std::function<std::string(const std::string&, CodeTerm::code_t&)> op_parser_t;
	typedef std::chrono::steady_clock::time_point time_point_t;

	class CAMException : public std::runtime_error
	{
	public:
		CAMException(const std::string& what_arg) : runtime_error(what_arg) {}
	};

	class InvalidTermException : public CAMException
	{
		std::string msg;

		std::string term;
		std::string expected;
	public:
		InvalidTermException(const std::string& term, const std::string& expected);

		virtual const char* what() const throw() { return msg.c_str(); }
	};

	class InvalidStackException : public CAMException
	{
	public:
		InvalidStackException(const std::string& what_arg) : CAMException("Invalid stack: " + what_arg) {}
	};

	class InvalidCodeException : public CAMException
	{
	public:
		InvalidCodeException(const std::string& what_arg) : CAMException("Invalid code: " + what_arg) {}
	};

	class CAM
	{
		stack_t _stack;
		Term::term_ptr _term;
		transition_t _transitions[256];
		op_parser_t _op_parsers[256];
		bool _is_verbose;
		bool _is_print_result;

	public:
		CAM();

		void run(const std::string &s);

		void set_verbose(bool is_verbose) { _is_verbose = is_verbose; }
		bool verbose() const { return _is_verbose; }

		void set_is_print_result(bool is_print_result) { _is_print_result = is_print_result; }
		bool print_result() const { return _is_print_result; }

	private:
		CodeTerm::code_t parse_code(const std::string &s);

		void init_parsers();

		void init_transitions();

		static void apply_operation(Term::term_ptr &term, binary_operation_t op);

		static std::pair<std::string, std::string> get_op_arg(const std::string &c, bool with_op=true);

		static std::string to_string(stack_t &stack);
	};
};