#pragma once

#include <string>
#include <ostream>

struct CAP {

	public:

		class LIST;

		enum SET {
			BOUNDING, PERMITTED, INHERITABLE, EFFECTIVE, AMBIENT
		};

		enum TYPE {
			CHOWN, DAC_OVERRIDE, DAC_READ_SEARCH, FOWNER, FSETID,
			KILL, SETGID, SETUID, SETPCAP, LINUX_IMMUTABLE,
			NET_BIND_SERVICE, NET_BROADCAST, NET_ADMIN, NET_RAW,
			IPC_LOCK, IPC_OWNER, SYS_MODULE, SYS_RAWIO, SYS_CHROOT,
			SYS_PTRACE, SYS_PACCT, SYS_ADMIN, SYS_BOOT, SYS_NICE,
			SYS_RESOURCE, SYS_TIME, SYS_TTY_CONFIG, MKNOD, LEASE,
			AUDIT_WRITE, AUDIT_CONTROL, SETFCAP, MAC_OVERRIDE,
			MAC_ADMIN, SYSLOG, WAKE_ALARM, BLOCK_SUSPEND, AUDIT_READ,
			PERFMON, BPF, CHECKPOINT_RESTORE
		};

		operator CAP::TYPE() const;
		operator std::string() const;
		operator int() const;

		bool operator <(const CAP& other) const;
		bool operator <(const CAP::TYPE& type) const;

		bool operator ==(const std::string& name) const;
		bool operator ==(const CAP::TYPE& type) const;
		bool operator ==(const CAP& other) const;
		bool operator ==(int value) const;

		bool operator !=(const std::string& name) const;
		bool operator !=(const CAP::TYPE& type) const;
		bool operator !=(const CAP& other) const;
		bool operator !=(int value) const;

		CAP& operator =(const std::string& name);
		CAP& operator =(const CAP::TYPE& type);
		CAP& operator =(const CAP& other);
		CAP& operator =(int value);

		CAP::TYPE type() const;
		std::string name() const;
		int value() const;

		std::string to_string() const;
		int to_int() const;

		CAP(const std::string& name);
		CAP(const CAP::TYPE& type) : _type(type) {}
		CAP(const CAP& other) : _type(other._type) {}
		CAP(int value);

		friend std::ostream& operator <<(std::ostream& os, const CAP& c);

	private:
		CAP::TYPE _type;
};

std::ostream& operator <<(std::ostream& os, const CAP::TYPE& type);
std::ostream& operator <<(std::ostream& os, const CAP& c);
