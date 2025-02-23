#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>

#include "capabilities.hpp"

void sleep(int ms) {
	::poll(nullptr, 0, ms);
}

bool update_caps() {

	CAPS caps = {
		{ CAP::SET::BOUNDING, { CAP::KILL, CAP::SYS_CHROOT, CAP::NET_BIND_SERVICE, CAP::AUDIT_WRITE }},
		{ CAP::SET::PERMITTED, { CAP::AUDIT_WRITE, CAP::KILL, CAP::NET_BIND_SERVICE, CAP::SYS_CHROOT }},
		{ CAP::SET::INHERITABLE, { CAP::KILL, CAP::SYS_CHROOT }},
		{ CAP::SET::EFFECTIVE, { CAP::SYS_CHROOT, CAP::AUDIT_WRITE, CAP::KILL }},
		{ CAP::SET::AMBIENT, { CAP::SYS_CHROOT }}
	};

	try {
		caps.set();
	} catch ( const std::runtime_error& e ) {
		std::cerr << e.what() << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char** argv) {

	if ( ::getuid() != 0 ) {
		std::cerr <<"error, this program needs root privileges" << std::endl;
		exit(1);
	}

	std::cout << "parent pid " << ::getpid() << std::endl;

	pid_t pid = fork();
	if ( pid < 0 ) {
		std::cerr << "error, fork failed: " << ::strerror(errno) << std::endl;
		exit(1);
	}

	if ( pid == 0 ) {

		pid = ::getpid();
		std::cout << "child process started with pid " << pid << std::endl;

		if ( !update_caps())
			exit(1);

		sleep(200);
		std::cout << "shell starts now" << std::endl;

		char* args[2] = { (char*)"/bin/sh", nullptr };
		execvp(args[0], args);
		exit(0);
	}

	std::cout << "waiting for child to finish" << std::endl;
	::waitpid(pid, nullptr, 0);

	return 0;
}
