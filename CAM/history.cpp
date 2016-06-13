#include <iostream>
#include <iomanip>
#include <string>

#include "history.h"

namespace CAM {
	History::History() {
		_header.push_back("¹");
		_header.push_back("Term");
		_header.push_back("Code");
		_header.push_back("Stack");

		_idx_w = _header[0].length();
		_term_w = _header[1].length();
		_code_w = _header[2].length();
		_stack_w = _header[3].length();
	}

	void History::add(const std::string &t, const std::string &c, const std::string &s) {
		_term.push_back(t);
		_code.push_back(c);
		_stack.push_back(s);

		size_t idx_w = log10(_term.size()) + 1;
		if (idx_w > _idx_w)
			_idx_w = idx_w;
		if (t.length() > _term_w)
			_term_w = t.length();
		if (c.length() > _code_w)
			_code_w = c.length();
		if (s.length() > _stack_w)
			_stack_w = s.length();
	}

	void History::print() const {
		const char separator = ' ';

		size_t max_idx = _term.size();
		size_t line_w = _idx_w + _term_w + _code_w + _stack_w + 13;

		if (max_idx == 0)
			return;

		print_line(line_w);
		print_header();
		print_line(line_w);

		for (size_t i = 0; i < max_idx; i++) {
			print_entry(i, _term[i], _code[i], _stack[i]);
			print_line(line_w);
		}
	}

	void History::print_entry(size_t idx, const std::string &t, const std::string &c, const std::string &s) const {
		const char separator = ' ';

		std::cout << "| " << std::left << std::setw(_idx_w) << std::setfill(separator) << idx << ' ';
		std::cout << "| " << std::left << std::setw(_term_w) << std::setfill(separator) << t << ' ';
		std::cout << "| " << std::left << std::setw(_code_w) << std::setfill(separator) << c << ' ';
		std::cout << "| " << std::left << std::setw(_stack_w) << std::setfill(separator) << s << " |";
		std::cout << std::endl;
	}

	void History::print_header() const {
		const char separator = ' ';

		std::cout << "| " << std::left << std::setw(_idx_w) << std::setfill(separator) << _header[0] << ' ';
		std::cout << "| " << std::left << std::setw(_term_w) << std::setfill(separator) << _header[1] << ' ';
		std::cout << "| " << std::left << std::setw(_code_w) << std::setfill(separator) << _header[2] << ' ';
		std::cout << "| " << std::left << std::setw(_stack_w) << std::setfill(separator) << _header[3] << " |";
		std::cout << std::endl;
	}

	void History::print_line(size_t w) const {
		std::cout << '+' << std::right << std::setw(w-1) << std::setfill('-') << '+' << std::endl;
	}
}
