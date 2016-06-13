#include <iostream>

#include "code.h"
#include "term.h"
#include "CAM.h"
#include "history.h"

namespace CAM
{
	InvalidTermException::InvalidTermException(const std::string& term, const std::string& expected) :
		CAMException("Invalid term"), term(term), expected(expected) {
		std::ostringstream ossteam;
		ossteam << CAMException::what() << ": " << term << " "
			<< "(expected: " << expected << ")";
		msg = ossteam.str();
	}


	CAM::CAM() : _is_verbose(false) {
		init_transitions();
		init_parsers();
	}

	void CAM::run(const std::string &s) {
		size_t i = 0;

		time_point_t begin = std::chrono::steady_clock::now();

		History history;

		try {
			_term = Term::make();
			CodeTerm::code_t code = parse_code(s);

			while (!code.empty())
			{
				if (_is_verbose)
					history.add(_term->to_string(), CodeTerm::to_string(code), to_string(_stack));

				char op = code.front()->op();
				if (_transitions.find(op) == _transitions.end()) {
					std::cerr << "invalid op: " << op << std::endl;
					break;
				}

				auto tr = _transitions[op];

				tr(_term, code, _stack);
				++i;
			}

			if (_is_verbose)
				history.add(_term->to_string(), CodeTerm::to_string(code), to_string(_stack));
		}
		catch (CAMException &e) {
			const char *s = e.what();
			std::cerr << e.what() << std::endl;
		}

		time_point_t end = std::chrono::steady_clock::now();
		
		history.print();
		
		std::cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0 << std::endl;

		if (_is_verbose || _is_print_result)
			std::cout << "term: " << *_term << std::endl;
	}

	CodeTerm::code_t CAM::parse_code(const std::string &s) {
		std::string c = s;
		CodeTerm::code_t code;
		while (!c.empty())
		{
			if (_op_parsers.find(c[0]) == _op_parsers.end())
				throw InvalidCodeException(c);

			auto p = _op_parsers[c[0]];
			c = p(c, code);
		}

		return code;
	}

	void CAM::init_parsers() {
		auto default_parser = [](const std::string& s, CodeTerm::code_t &code) -> std::string {
			auto t = CodeTerm::make(s[0]);
			code.push_back(t);
			return s.substr(1);
		};

		auto un_op_parser = [this](const std::string &s, CodeTerm::code_t &code) -> std::string {
			auto res = get_op_arg(s);
			auto args = CodeTermWithArgs::args_t();
			args.push_back(parse_code(res.first));
			auto t = CodeTermWithArgs::make(s[0], args);
			code.push_back(t);
			return res.second;
		};

		auto quote_op_parser = [this](const std::string &s, CodeTerm::code_t &code) -> std::string {
			auto res = get_op_arg(s);
			auto t = QuoteCodeTerm::make(res.first);
			code.push_back(t);
			return res.second;
		};

		auto bin_op_parser = [this](const std::string &s, CodeTerm::code_t &code) -> std::string {
			auto res = get_op_arg(s);
			std::string rest = res.second;
			auto args = CodeTermWithArgs::args_t();

			res = get_op_arg(res.first, false);
			args.push_back(parse_code(res.first));
			res = get_op_arg(res.second);
			args.push_back(parse_code(res.first));

			auto t = CodeTermWithArgs::make(s[0], args);

			code.push_back(t);
			return rest;
		};

		_op_parsers.insert(std::pair<char, op_parser_t>('F', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('S', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('<', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>(',', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('>', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('e', default_parser));

		_op_parsers.insert(std::pair<char, op_parser_t>('+', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('-', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('=', default_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('*', default_parser));

		_op_parsers.insert(std::pair<char, op_parser_t>('\\', un_op_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('\'', quote_op_parser));
		_op_parsers.insert(std::pair<char, op_parser_t>('Y', un_op_parser));

		_op_parsers.insert(std::pair<char, op_parser_t>('b', bin_op_parser));
	}

	void CAM::init_transitions() {
		_transitions.insert(std::pair<char, transition_t>('F', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto pair = std::dynamic_pointer_cast<TermPair>(term);
			if (!pair.get())
				throw InvalidTermException(term->to_string(), "(s, t)");

			term = pair->first();
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('S', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto pair = std::dynamic_pointer_cast<TermPair>(term);
			if (!pair.get())
				throw InvalidTermException(term->to_string(), "(s, t)");

			term = pair->second();
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('<', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			stack.push_front(term);
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>(',', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			if (stack.empty())
				throw InvalidStackException("Empty stack");

			auto top = stack.front();
			stack.pop_front();
			stack.push_front(term);
			term = top;
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('>', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			if (stack.empty())
				throw InvalidStackException("Empty stack");

			auto top = stack.front();
			stack.pop_front();
			term = TermPair::make(top, term);
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('e', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto pair = std::dynamic_pointer_cast<TermPair>(term);
			if (!pair.get())
				throw InvalidTermException(term->to_string(), "(C: s, t)");

			auto app_term = std::dynamic_pointer_cast<AppTerm>(pair->first());
			if (!app_term.get())
				throw InvalidTermException(term->to_string(), "(C: s, t)");

			auto c = app_term->code();
			term = TermPair::make(app_term->term(), pair->second());
			code.pop_front();
			code.insert(code.begin(), c.begin(), c.end());
		}));

		_transitions.insert(std::pair<char, transition_t>('\\', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto c = std::dynamic_pointer_cast<CodeTermWithArgs>(code.front());
			if (!c.get())
				throw InvalidCodeException(code.front()->to_string());

			if (c->args_count() != 1)
				throw InvalidCodeException(c->to_string());

			term = AppTerm::make(c->get_arg(0), term);
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('\'', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto c = std::dynamic_pointer_cast<QuoteCodeTerm>(code.front());
			if (!c.get())
				throw InvalidCodeException(code.front()->to_string());

			term = QuoteTerm::make(c->get_arg());
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('b', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto t = std::dynamic_pointer_cast<QuoteTerm>(term);
			if (!t.get())
				throw InvalidTermException(term->to_string(), "numeric constant");

			if (stack.empty())
				throw InvalidStackException("Empty stack");

			auto top = stack.front();
			stack.pop_front();
			term = top;

			auto c = std::dynamic_pointer_cast<CodeTermWithArgs>(code.front());
			if (!c.get())
				throw InvalidCodeException(code.front()->to_string());

			if (c->args_count() != 2)
				throw InvalidCodeException(c->to_string());

			CodeTerm::code_t next;

			if (t->value() != 0)
				next = c->get_arg(0);
			else
				next = c->get_arg(1);
				
			code.pop_front();
			code.insert(code.begin(), next.begin(), next.end());
		}));

		_transitions.insert(std::pair<char, transition_t>('Y', [this](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			auto c = std::dynamic_pointer_cast<CodeTermWithArgs>(code.front());
			if (!c.get())
				throw InvalidCodeException(code.front()->to_string());

			if (c->args_count() != 1)
				throw InvalidCodeException(c->to_string());

			term = RecTerm::make(c->get_arg(0), term);
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('+', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			apply_operation(term, [](const mpz_class &a, const mpz_class &b) {
				return a + b;
			});
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('-', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			apply_operation(term, [](const mpz_class &a, const mpz_class &b) {
				return a - b;
			});
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('*', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			apply_operation(term, [](const mpz_class &a, const mpz_class &b) {
				return a * b;
			});
			code.pop_front();
		}));

		_transitions.insert(std::pair<char, transition_t>('=', [](Term::term_ptr &term, CodeTerm::code_t &code, stack_t &stack) {
			apply_operation(term, [](const mpz_class &a, const mpz_class &b) {
				return a == b;
			});
			code.pop_front();
		}));
	}

	void CAM::apply_operation(Term::term_ptr &term, binary_operation_t op) {
		auto pair = std::dynamic_pointer_cast<TermPair>(term);
		if (!pair.get())
			throw InvalidTermException(term->to_string(), "(s,t)");

		auto first = std::dynamic_pointer_cast<QuoteTerm>(pair->first());
		if (!first.get())
			throw InvalidTermException(term->to_string(), "(s,t) where s and t - numeric constants");

		auto second = std::dynamic_pointer_cast<QuoteTerm>(pair->second());
		if (!second.get())
			throw InvalidTermException(term->to_string(), "(s,t) where s and t - numeric constants");

		term = QuoteTerm::make(op(first->value(), second->value()));
	}

	std::pair<std::string, std::string> CAM::get_op_arg(const std::string &c, bool with_op) {
		int br_sum = 1;
		size_t start_idx = with_op ? 2 : 1;
		size_t i = start_idx;
		for (; i < c.length() && br_sum; i++) {
			if (c[i] == '(')
				++br_sum;
			else if (c[i] == ')')
				--br_sum;
		}

		if (c.length() <= start_idx)
			throw InvalidCodeException(c);

		return std::make_pair<>(c.substr(start_idx, i - start_idx - 1), c.substr(i));
	}

	std::string CAM::to_string(stack_t &stack) {
		std::ostringstream ossteam;
		ossteam << '[';
		for (size_t i = 0; i < stack.size(); i++) {
			ossteam << stack[i]->to_string();
			if (i != stack.size())
				ossteam << ", ";
		}
		ossteam << ']';

		return ossteam.str();
	}
}
