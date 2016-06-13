#include "code.h"

namespace CAM
{
	std::string CodeTerm::to_string(const code_t &t) {
		std::ostringstream ossteam;
		std::for_each(t.begin(), t.end(), [&ossteam](const CodeTerm::term_ptr &c) {
			ossteam << *c;
		});
		return ossteam.str();
	}

	std::string CodeTermWithArgs::to_string() const {
		std::ostringstream ossteam;
		ossteam << _op;
		if (_args.size() != 0) {
			ossteam << '(';
			for (size_t i = 0; i < _args.size(); i++) {
				ossteam << CodeTerm::to_string(_args[i]);
				if (i != _args.size() - 1)
					ossteam << ", ";
			}
			ossteam << ')';
		}

		return ossteam.str();
	}
}