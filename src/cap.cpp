#include <string>
#include <iostream>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <sys/capability.h>
#include "cap-ng/cap-ng.hpp"
#include "capabilities.hpp"

static std::map<CAP::TYPE, std::pair<int, std::string>> names = {
	{ CAP::TYPE::CHOWN, { CAP_CHOWN, "cap_chown" }},
	{ CAP::TYPE::DAC_OVERRIDE, { CAP_DAC_OVERRIDE, "cap_dac_override" }},
	{ CAP::TYPE::DAC_READ_SEARCH, { CAP_DAC_READ_SEARCH, "cap_dac_read_search" }},
	{ CAP::TYPE::FOWNER, { CAP_FOWNER, "cap_fowner" }},
	{ CAP::TYPE::FSETID, { CAP_FSETID, "cap_fsetid" }},
	{ CAP::TYPE::KILL, { CAP_KILL, "cap_kill" }},
	{ CAP::TYPE::SETGID, { CAP_SETGID, "cap_setgid" }},
	{ CAP::TYPE::SETUID, { CAP_SETUID, "cap_setuid" }},
	{ CAP::TYPE::SETPCAP, { CAP_SETPCAP, "cap_setpcap" }},
	{ CAP::TYPE::LINUX_IMMUTABLE, { CAP_LINUX_IMMUTABLE, "cap_linux_immutable" }},
	{ CAP::TYPE::NET_BIND_SERVICE, { CAP_NET_BIND_SERVICE, "cap_net_bind_service" }},
	{ CAP::TYPE::NET_BROADCAST, { CAP_NET_BROADCAST, "cap_net_broadcast" }},
	{ CAP::TYPE::NET_ADMIN, { CAP_NET_ADMIN, "cap_net_admin" }},
	{ CAP::TYPE::NET_RAW, { CAP_NET_RAW, "cap_net_raw" }},
	{ CAP::TYPE::IPC_LOCK, { CAP_IPC_LOCK, "cap_ipc_lock" }},
	{ CAP::TYPE::IPC_OWNER, { CAP_IPC_OWNER, "cap_ipc_owner" }},
	{ CAP::TYPE::SYS_MODULE, { CAP_SYS_MODULE, "cap_sys_module" }},
	{ CAP::TYPE::SYS_RAWIO, { CAP_SYS_RAWIO, "cap_sys_rawio" }},
	{ CAP::TYPE::SYS_CHROOT, { CAP_SYS_CHROOT, "cap_sys_chroot" }},
	{ CAP::TYPE::SYS_PTRACE, { CAP_SYS_PTRACE, "cap_sys_ptrace" }},
	{ CAP::TYPE::SYS_PACCT, { CAP_SYS_PACCT, "cap_sys_pacct" }},
	{ CAP::TYPE::SYS_ADMIN, { CAP_SYS_ADMIN, "cap_sys_admin" }},
	{ CAP::TYPE::SYS_BOOT, { CAP_SYS_BOOT, "cap_sys_boot" }},
	{ CAP::TYPE::SYS_NICE, { CAP_SYS_NICE, "cap_sys_nice" }},
	{ CAP::TYPE::SYS_RESOURCE, { CAP_SYS_RESOURCE, "cap_sys_resource" }},
	{ CAP::TYPE::SYS_TIME, { CAP_SYS_TIME, "cap_sys_time" }},
	{ CAP::TYPE::SYS_TTY_CONFIG, { CAP_SYS_TTY_CONFIG, "cap_sys_tty_config" }},
	{ CAP::TYPE::MKNOD, { CAP_MKNOD, "cap_mknod" }},
	{ CAP::TYPE::LEASE, { CAP_LEASE, "cap_lease" }},
	{ CAP::TYPE::AUDIT_WRITE, { CAP_AUDIT_WRITE, "cap_audit_write" }},
	{ CAP::TYPE::AUDIT_CONTROL, { CAP_AUDIT_CONTROL, "cap_audit_control" }},
	{ CAP::TYPE::SETFCAP, { CAP_SETFCAP, "cap_setfcap" }},
	{ CAP::TYPE::MAC_OVERRIDE, { CAP_MAC_OVERRIDE, "cap_mac_override" }},
	{ CAP::TYPE::MAC_ADMIN, { CAP_MAC_ADMIN, "cap_mac_admin" }},
	{ CAP::TYPE::SYSLOG, { CAP_SYSLOG, "cap_syslog" }},
	{ CAP::TYPE::WAKE_ALARM, { CAP_WAKE_ALARM, "cap_wake_alarm" }},
	{ CAP::TYPE::BLOCK_SUSPEND, { CAP_BLOCK_SUSPEND, "cap_block_suspend" }},
	{ CAP::TYPE::AUDIT_READ, { CAP_AUDIT_READ, "cap_audit_read" }},
	{ CAP::TYPE::PERFMON, { CAP_PERFMON, "cap_perfmon" }},
	{ CAP::TYPE::BPF, { CAP_BPF, "cap_bpf" }},
	{ CAP::TYPE::CHECKPOINT_RESTORE, { CAP_CHECKPOINT_RESTORE, "cap_checkpoint_restore" }},
};

static std::string str_to_lower(const std::string& str) {

	std::string s(str);

	for ( auto& ch : s )
		if ( std::isupper(ch))
			ch ^= 32;
	return s;
}

size_t CAP::caps_max() {
	return names.size();
}

CAP::operator CAP::TYPE() const {
	return this -> _type;
}

CAP::operator std::string() const {
	return names.at(this -> _type).second;
}

CAP::operator int() const {
	return names.at(this -> _type).first;
}

bool CAP::operator <(const CAP& other) const {
	return this -> value() < other.value();
}

bool CAP::operator <(const CAP::TYPE& type) const {
	return this -> value() < CAP(type).value();
}

bool CAP::operator ==(const std::string& name) const {
	return name == names.at(this -> _type).second;
}

bool CAP::operator ==(const CAP::TYPE& type) const {
	return this -> _type == type;
}

bool CAP::operator ==(const CAP& other) const {
	return this -> _type == other._type;
}

bool CAP::operator ==(int value) const {
	return value == names.at(this -> _type).first;
}

bool CAP::operator !=(const std::string& name) const {
	return !(this -> operator ==(name));
}

bool CAP::operator !=(const CAP::TYPE& type) const {
	return !(this -> operator ==(type));
}

bool CAP::operator !=(const CAP& other) const {
	return !(this -> operator ==(other));
}

bool CAP::operator !=(int value) const {
	return !(this -> operator ==(value));
}

CAP& CAP::operator =(const std::string& name) {

	std::string value = str_to_lower(name);

	if ( auto it = std::find_if(names.begin(), names.end(),
		[value](const std::pair<CAP::TYPE, std::pair<int, std::string>>& p) {
			return value == p.second.second;
		}); it != names.end()) {
		this -> _type = it -> first;
		return *this;
	}

	throw std::runtime_error("capability " + value + " is not supported");
}

CAP& CAP::operator =(const CAP::TYPE& type) {

	this -> _type = type;
	return *this;
}

CAP& CAP::operator =(const CAP& other) {

	this -> _type = other._type;
	return *this;
}

CAP& CAP::operator =(int value) {

	if ( auto it = std::find_if(names.begin(), names.end(),
		[value](const std::pair<CAP::TYPE, std::pair<int, std::string>>& p) {
			return value == p.second.first;
		}); it != names.end()) {
		this -> _type = it -> first;
		return *this;
	}

	throw std::runtime_error("capability value " + std::to_string(value) + " is not supported");
}

CAP::TYPE CAP::type() const {
	return this -> _type;
}

std::string CAP::name() const {
	return this -> operator std::string();
}

int CAP::value() const {
	return this -> operator int();
}

std::string CAP::to_string() const {
	return this -> name();
}

int CAP::to_int() const {
	return this -> value();
}

CAP::CAP(const std::string& name) {

	std::string value = str_to_lower(name);

	if ( auto it = std::find_if(names.begin(), names.end(),
		[value](const std::pair<CAP::TYPE, std::pair<int, std::string>>& p) {
			return value == p.second.second;
		}); it != names.end()) {
		this -> _type = it -> first;
	} else throw std::runtime_error("capability " + value + " is not supported");
}

CAP::CAP(int value) {

	if ( auto it = std::find_if(names.begin(), names.end(),
		[value](const std::pair<CAP::TYPE, std::pair<int, std::string>>& p) {
			return value == p.second.first;
		}); it != names.end()) {
		this -> _type = it -> first;
	} else throw std::runtime_error("capability value " + std::to_string(value) + " is not supported");
}

CAPS::SET CAPS::all() {

	CAPS::SET types;

	for ( const auto& e : names )
		types += CAP(e.first);

	return types;
}

std::ostream& operator <<(std::ostream& os, const CAP::TYPE& ct) {
	os << CAP(ct).to_string();
        return os;
}

std::ostream& operator <<(std::ostream& os, const CAP& c) {
	os << c.to_string();
	return os;
}
