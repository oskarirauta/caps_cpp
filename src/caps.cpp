#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/capability.h>
#include <linux/securebits.h>
#include "cap-ng/cap-ng.hpp"
#include "capabilities/cap.hpp"
#include "capabilities/caps.hpp"

static bool contains_capset(const std::set<CAP::SET>& cs, const CAP::SET& c) {

	return std::find_if(cs.begin(), cs.end(), [c](const CAP::SET& _cs) { return _cs == c; }) != cs.end();
}

static bool contains_cap(const std::set<CAP>& cs, const CAP& c) {

	return std::find_if(cs.begin(), cs.end(), [c](const CAP& _cs) { return _cs == c; }) != cs.end();
}

CAPS& CAPS::operator =(const CAPS& other) {
	this -> _m = other._m;
	return *this;
}

CAPS& CAPS::operator =(const std::map<CAP::SET, std::set<CAP>>& m) {
	this -> _m = m;
	return *this;
}

CAPS& CAPS::operator =(const std::initializer_list<std::pair<CAP::SET, std::set<CAP>>>& values) {

	this -> _m.clear();

	for ( auto& p : values )
		this -> _m.emplace(p.first, p.second);

	return *this;
}

CAPS& CAPS::operator =(const CAPS::LIST& l) {

	this -> _m = {};

	for ( auto& p : l ) {

		if ( p.second.empty())
			continue;

		for ( auto& s: p.second ) {

			std::set<CAP> caps;

			if ( this -> contains(s))
				caps = this -> _m.at(s);

			caps.emplace(p.first);
			this -> _m.emplace(s, caps);
		}
	}

	return *this;
}

CAPS::operator CAPS::LIST() const {

	std::map<CAP::SET, std::set<CAP>> l = this -> _m;
	std::map<CAP, std::set<CAP::SET>> res = {};

	for ( const auto& e : CAPS::all()) {

		std::set<CAP::SET> sets = {};
		if ( contains_cap(l[CAP::SET::BOUNDING], CAP(e)))
			sets.emplace(CAP::SET::BOUNDING);
		if ( contains_cap(l[CAP::SET::PERMITTED], CAP(e)))
			sets.emplace(CAP::SET::PERMITTED);
		if ( contains_cap(l[CAP::SET::INHERITABLE], CAP(e)))
			sets.emplace(CAP::SET::INHERITABLE);
		if ( contains_cap(l[CAP::SET::EFFECTIVE], CAP(e)))
			sets.emplace(CAP::SET::EFFECTIVE);
		if ( contains_cap(l[CAP::SET::AMBIENT], CAP(e)))
			sets.emplace(CAP::SET::AMBIENT);

		res[CAP(e)] = sets;

	}

	return res;
}

CAPS::LIST CAPS::to_list() const {
	return this -> operator CAPS::LIST();
}

std::set<CAP>& CAPS::at(const CAP::SET& set) {
	return this -> _m.at(set);
}

const std::set<CAP>& CAPS::at(const CAP::SET& set) const {
	return this -> _m.at(set);
}

std::set<CAP>& CAPS::operator [](const CAP::SET& set) {
	return this -> _m.at(set);
}

const std::set<CAP>& CAPS::operator [](const CAP::SET& set) const {
	return this -> _m.at(set);
}

CAPS::iterator CAPS::begin() {
	return this -> _m.begin();
}

CAPS::const_iterator CAPS::begin() const {
	return this -> _m.begin();
}

CAPS::const_iterator CAPS::cbegin() const {
	return this -> _m.begin();
}

CAPS::iterator CAPS::end() {
	return this -> _m.end();
}

CAPS::const_iterator CAPS::end() const {
	return this -> _m.end();
}

CAPS::const_iterator CAPS::cend() const {
	return this -> _m.end();
}

bool CAPS::empty() const {
	return this -> _m.empty();
}

CAPS::size_type CAPS::size() const {
	return this -> _m.size();
}

CAPS::size_type CAPS::max_size() const {
	return 5;
}

void CAPS::clear() {
	this -> _m.clear();
}

void CAPS::emplace(const CAP::SET& s, const std::set<CAP>& c) {
	this -> _m.emplace(s, c);
}

void CAPS::erase(const CAP::SET& s) {

	this -> _m.erase(s);
}

CAPS::iterator CAPS::find(const CAP::SET& s) {

	return this -> _m.find(s);
}

CAPS::const_iterator CAPS::find(const CAP::SET& s) const {

	return this -> _m.find(s);
}

bool CAPS::contains(const CAP::SET& s) const {
	return this -> _m.find(s) != this -> _m.end();
}

CAPS::CAPS() {
	this -> _m = {};
}

CAPS::CAPS(const CAPS& other) {
	this -> _m = other._m;
}

CAPS::CAPS(const std::map<CAP::SET, std::set<CAP>>& m) {
	this -> _m = m;
}

CAPS::CAPS(const std::initializer_list<std::pair<CAP::SET, std::set<CAP>>>& values) {

	this -> _m = {};

	for ( auto& p : values )
		this -> _m.emplace(p.first, p.second);
}

CAPS::CAPS(const CAPS::LIST& l) {

	this -> _m = {};

	for ( auto& p : l ) {

		if ( p.second.empty())
			continue;

		for ( auto& s: p.second ) {

			std::set<CAP> caps;

			if ( this -> contains(s))
				caps = this -> _m.at(s);

			caps.emplace(p.first);
			this -> _m.emplace(s, caps);
		}
	}
}

bool CAPS::set(const CAPS& c) {

	std::map<CAP::SET, std::set<CAP>> l = c._m;
	std::set<CAP> all_types = {};

	std::set<CAP> bounding = l[CAP::SET::BOUNDING];
	std::set<CAP> permitted = l[CAP::SET::PERMITTED];
	std::set<CAP> inheritable = l[CAP::SET::INHERITABLE];
	std::set<CAP> effective = l[CAP::SET::EFFECTIVE];
	std::set<CAP> ambient = l[CAP::SET::AMBIENT];
	bool updated = false;

	for ( const auto& e : CAPS::all())
		all_types.emplace(CAP(e));

	capng_clear(CAPNG_SELECT_ALL);

	for ( const auto& e : all_types ) {

		if ( auto it = ambient.find(e); it != ambient.end() && !contains_cap(bounding, e) && !contains_cap(permitted, e) && !contains_cap(inheritable, e)) {
			std::cerr << "removed " << e.to_string() <<
				" from ambient set, it must also belong to bounding, permitted and inheritable sets if ambient is used" << std::endl;
			ambient.erase(it);
		}

		int sets = 0;

		if ( contains_cap(bounding, e))
			sets |= CAPNG_BOUNDING_SET;
		if ( contains_cap(permitted, e))
			sets |= CAPNG_PERMITTED;
		if ( contains_cap(inheritable, e))
			sets |= CAPNG_INHERITABLE;
		if ( contains_cap(effective, e))
			sets |= CAPNG_EFFECTIVE;
		if ( contains_cap(ambient, e))
			sets |= CAPNG_AMBIENT;

		if ( sets != 0 ) {
			updated = true;
			capng_update(CAPNG_ADD, sets, e.value());
		}
	}

	if ( updated && capng_apply(CAPNG_SELECT_ALL))
		return false;

	return true;
}

CAPS CAPS::get(pid_t pid) {

	capng_clear(CAPNG_SELECT_ALL);
	capng_setpid(pid < 1 ? ::getpid() : pid);

	if ( capng_get_caps_process()) {
		std::cerr << "failed to get capabilities for pid " << pid << std::endl;
		return {};
	}

	std::set<CAP> bounding = {};
	std::set<CAP> permitted = {};
	std::set<CAP> inheritable = {};
	std::set<CAP> effective = {};
	std::set<CAP> ambient = {};

	for ( const auto& e : CAPS::all()) {

		if ( capng_have_capability(CAPNG_BOUNDING_SET, e) != 0 )
			bounding.emplace(CAP(e));

		if ( capng_have_capability(CAPNG_PERMITTED, e) != 0 )
			permitted.emplace(CAP(e));

		if ( capng_have_capability(CAPNG_INHERITABLE, e) != 0 )
			inheritable.emplace(CAP(e));

		if ( capng_have_capability(CAPNG_EFFECTIVE, e) != 0 )
			effective.emplace(CAP(e));

		if ( capng_have_capability(CAPNG_AMBIENT, e) != 0 )
			ambient.emplace(CAP(e));
	}

	CAPS c = {
		{ CAP::SET::BOUNDING, bounding },
		{ CAP::SET::PERMITTED, permitted },
		{ CAP::SET::INHERITABLE, inheritable },
		{ CAP::SET::EFFECTIVE, effective },
		{ CAP::SET::AMBIENT, ambient }
	};

	return c;
}

bool CAPS::lock() {

	return capng_lock() == 0;
}

bool CAPS::set_user(uid_t uid, gid_t gid, const std::set<gid_t>& additional_gids, const CAPS& caps) {

	std::set<CAP> bounding = caps[CAP::SET::BOUNDING];
	std::set<CAP> permitted = caps[CAP::SET::PERMITTED];
	std::set<CAP> inheritable = caps[CAP::SET::INHERITABLE];
	std::set<CAP> effective = caps[CAP::SET::EFFECTIVE];
	std::set<CAP> ambient = caps[CAP::SET::AMBIENT];

	permitted.emplace(CAP::SETPCAP);
	permitted.emplace(CAP::SETGID);
	permitted.emplace(CAP::SETUID);
	effective.emplace(CAP::SETPCAP);
	effective.emplace(CAP::SETGID);
	effective.emplace(CAP::SETUID);

	if ( ::prctl(PR_SET_SECUREBITS, SECBIT_NO_SETUID_FIXUP) != 0 )
		return false;

	if ( ::prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0 )
		return false;

	CAPS _caps = {
		{ CAP::SET::BOUNDING, bounding },
		{ CAP::SET::PERMITTED, permitted },
		{ CAP::SET::INHERITABLE, inheritable },
		{ CAP::SET::EFFECTIVE, effective },
		{ CAP::SET::AMBIENT, ambient }
	};

	if ( !CAPS::set(_caps))
		return false;

	if ( passwd* pw = ::getpwuid(uid); pw != nullptr && ::initgroups(pw -> pw_name, gid) != 0 )
		return false;

	if ( ::setresgid(gid, gid, gid) != 0 )
		return false;

	if ( ::setresuid(uid, uid, uid) != 0 )
		return false;

	if ( !additional_gids.empty()) {

		std::vector<gid_t> gids;
		for ( auto& g : additional_gids )
			if ( g != gid )
				gids.push_back(g);

		if ( !gids.empty() && ::setgroups(gids.size(), &gids[0]) != 0 )
			return false;
	}

	if ( ::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) != 0 )
		return false;

	bool permitted_changed = false;
	bool effective_changed = false;

	if ( !contains_cap(caps[CAP::SET::PERMITTED], CAP::SETPCAP)) {
		permitted_changed = true;
		capng_update(CAPNG_DROP, CAPNG_PERMITTED, CAP::SETPCAP);
	}

	if ( !contains_cap(caps[CAP::SET::PERMITTED], CAP::SETGID)) {
		permitted_changed = true;
		capng_update(CAPNG_DROP, CAPNG_PERMITTED, CAP::SETGID);
	}

	if ( !contains_cap(caps[CAP::SET::PERMITTED], CAP::SETPCAP)) {
		permitted_changed = true;
		capng_update(CAPNG_DROP, CAPNG_PERMITTED, CAP::SETPCAP);
	}

	if ( !contains_cap(caps[CAP::SET::EFFECTIVE], CAP::SETPCAP)) {
		effective_changed = true;
		capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, CAP::SETPCAP);
	}

	if ( !contains_cap(caps[CAP::SET::EFFECTIVE], CAP::SETGID)) {
		effective_changed = true;
		capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, CAP::SETGID);
	}

	if ( !contains_cap(caps[CAP::SET::EFFECTIVE], CAP::SETPCAP)) {
		effective_changed = true;
		capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, CAP::SETPCAP);
	}

	if (( permitted_changed || effective_changed ) && capng_apply(CAPNG_SELECT_CAPS) != 0 )
		return false;

	return true;
}

std::ostream& operator <<(std::ostream& os, const CAPS& c) {

	std::string s;

	for ( auto& cap : c ) {

		if ( !s.empty())
			s += '\n';

		if ( cap.first == CAP::SET::BOUNDING )
			s += "bounding:    ";
		else if ( cap.first == CAP::SET::PERMITTED )
			s += "permitted:   ";
		else if ( cap.first == CAP::SET::INHERITABLE )
			s += "inheritable: ";
		else if ( cap.first == CAP::SET::EFFECTIVE )
			s += "effective:   ";
		else if ( cap.first == CAP::SET::AMBIENT )
			s += "ambient:     ";

		std::string l;

		for ( auto& n : cap.second ) {

			if ( !l.empty())
				l += ", ";

			l += n.to_string();
		}

		s += l;
	}

	os << s;
        return os;
}

std::ostream& operator <<(std::ostream& os, const CAPS::LIST& l) {

	std::string res;

	for ( const auto& e : l ) {

		CAP c = e.first;
		std::set<CAP::SET> s = e.second;

		if ( !res.empty())
			res += '\n';

		res += c.to_string();
		res += '\t';

		if ( c.type() != CAP::DAC_OVERRIDE && c.type() != CAP::DAC_READ_SEARCH && c.type() != CAP::LINUX_IMMUTABLE &&
			c.type() != CAP::NET_BIND_SERVICE && c.type() != CAP::NET_BROADCAST && c.type() != CAP::SYS_RESOURCE &&
			c.type() != CAP::SYS_TTY_CONFIG && c.type() != CAP::AUDIT_CONTROL && c.type() != CAP::MAC_OVERRIDE && c.type() != CAP::BLOCK_SUSPEND )
			res += '\t';

		res +=
			" bounding: " + std::string( contains_capset(s, CAP::SET::BOUNDING) ? "true " : "false" ) +
			" effective: " + std::string( contains_capset(s, CAP::SET::EFFECTIVE) ? "true " : "false" ) +
			" permitted: " + std::string( contains_capset(s, CAP::SET::PERMITTED) ? "true " : "false" ) +
			" inheritable: " + std::string( contains_capset(s, CAP::SET::INHERITABLE) ? "true " : "false" ) +
			" ambient: " + std::string( /*s.contains*/contains_capset(s, CAP::SET::AMBIENT) ? "true" : "false" );
	}

	os << res;
	return os;
}
