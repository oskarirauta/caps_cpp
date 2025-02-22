#include <iostream>
#include "capabilities.hpp"

int main(int argc, char** argv) {

	std::cout << "capability test\n" << std::endl;

	pid_t pid = -1;
	bool need_help = false;

	if ( argc != 2 ) {
		std::cout << "current capabilities:" << std::endl;
		std::cout << CAPS::get().to_list() << "\n" << std::endl;
		return 0;
	} else {

		std::string s(argv[1]);
		if ( s == "-h" || s == "--h" || s == "-help" || s == "--help" || s == "-usage" || s == "--usage" )
			need_help = true;
		else {

			try {
				pid = std::stoi(s);
			} catch (...) {
				std::cerr << "failed to convert " << argv[1] << " to number" << std::endl;
				need_help = true;
				pid = -1;
			}
		}
	}

	if ( need_help ) {
		std::cout << "usage:\n " << argv[0] << " <pid>" << std::endl;
		exit(0);
	}

	std::cout << "capabilities of PID " << pid << ":" << std::endl;
	std::cout << CAPS::get(pid).to_list() << "\n" << std::endl;

	return 0;
}
