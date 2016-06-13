#pragma once

#include <vector>

namespace CAM {
	class History
	{
		std::vector<std::string> _term;
		std::vector<std::string> _code;
		std::vector<std::string> _stack;

		std::vector<std::string> _header;

		size_t _term_w;
		size_t _code_w;
		size_t _stack_w;
		size_t _idx_w;
	public:
		History();

		void add(const std::string &t, const std::string &c, const std::string &s);

		void print() const;

	private:
		void print_entry(size_t idx, const std::string &t, const std::string &c, const std::string &s) const;

		void print_header() const;

		void print_line(size_t w) const;
	};
}
