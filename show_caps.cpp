#include <iostream>
#include "capabilities.hpp"

int main(int argc, char** argv) {


	std::cout << "\nthis demonstrates printing of caps in different forms:" << std::endl;

	CAPS c = {
		{ CAP::SET::PERMITTED, { CAP::KILL, CAP::CHOWN }},
		{ CAP::SET::EFFECTIVE, { CAP::KILL, CAP::AUDIT_WRITE }},
		{ CAP::SET::AMBIENT, { CAP::KILL }},
	};

	std::cout << "\ncaps:\n" << c << std::endl;

	CAPS::LIST l = c.to_list();

	std::cout << "\nlist:\n" << l << std::endl;

	return 0;
}
