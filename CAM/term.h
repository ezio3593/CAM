#pragma once

#include "code.h"
#include "gmpxx.h"

namespace CAM
{
	class Term
	{
	public:
		typedef std::shared_ptr<Term> term_ptr;

		static term_ptr make() { return std::make_shared<Term>(); }

		virtual std::string to_string() const { return "()"; }

		virtual ~Term() {}

		friend std::ostream& operator<<(std::ostream& os, const Term& t) {
			return os << t.to_string();
		}
	};

	class TermPair : public Term
	{
		Term::term_ptr _term1;
		Term::term_ptr _term2;
	public:
		typedef std::shared_ptr<TermPair> term_ptr;

		TermPair(const Term::term_ptr &t1, const Term::term_ptr &t2) : _term1(t1), _term2(t2) {}

		Term::term_ptr first() const { return _term1; }
		Term::term_ptr second() const { return _term2; }

		static Term::term_ptr make(const Term::term_ptr &t1, const Term::term_ptr &t2) {
			return std::dynamic_pointer_cast<Term>(std::make_shared<TermPair>(t1, t2));
		}

		virtual std::string to_string() const { return '(' + _term1->to_string() + ", " + _term2->to_string() + ')'; }
	};

	class QuoteTerm : public Term
	{
		mpz_class _number;
	public:
		typedef std::shared_ptr<QuoteTerm> term_ptr;

		QuoteTerm(const std::string &v) {
			_number = v;
		}

		QuoteTerm(const mpz_class &v) {
			_number = v;
		}

		mpz_class const& value() const { return _number; }

		static Term::term_ptr make(const std::string &v) {
			return std::dynamic_pointer_cast<Term>(std::make_shared<QuoteTerm>(v));
		}

		static Term::term_ptr make(const mpz_class &v) {
			return std::dynamic_pointer_cast<Term>(std::make_shared<QuoteTerm>(v));
		}

		virtual std::string to_string() const { return _number.get_str(); }
	};

	class AppTerm : public Term
	{
	protected:
		CodeTerm::code_t _code;
		Term::term_ptr _term;
	public:
		typedef std::shared_ptr<AppTerm> term_ptr;

		AppTerm(const CodeTerm::code_t &c, const Term::term_ptr &t) : _code(c), _term(t) {}

		CodeTerm::code_t code() const { return _code; }
		virtual Term::term_ptr term() const { return _term; }

		static Term::term_ptr make(const CodeTerm::code_t &code, const Term::term_ptr &term) {
			return std::dynamic_pointer_cast<Term>(std::make_shared<AppTerm>(code, term));
		}

		virtual std::string to_string() const { return CodeTerm::to_string(_code) + ": " + _term->to_string(); }
	};

	class RecTerm : public AppTerm, public std::enable_shared_from_this<RecTerm>
	{
	public:
		RecTerm(const CodeTerm::code_t &c, const Term::term_ptr &t) : AppTerm(c, t) {}

		virtual Term::term_ptr term() const {
			auto pair = std::make_shared<TermPair>(_term, std::const_pointer_cast<RecTerm>(shared_from_this()));
			return std::dynamic_pointer_cast<Term>(pair);
		}

		static Term::term_ptr make(const CodeTerm::code_t &code, const Term::term_ptr &term) {
			return std::dynamic_pointer_cast<Term>(std::make_shared<RecTerm>(code, term));
		}

		virtual std::string to_string() const { return CodeTerm::to_string(_code) + ": (" + _term->to_string() + ", rec)"; }
	};
}