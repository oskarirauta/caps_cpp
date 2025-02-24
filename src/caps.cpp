#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
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

static std::string err_desc() {

	std::string str(::strerror(errno));

	if ( !str.empty())
		str[0] = static_cast<std::string::value_type>(::tolower(str[0]));

	return str;
}

CAPS& CAPS::operator =(const CAPS& other) {
	this -> bounding = other.bounding;
	this -> permitted = other.permitted;
	this -> inheritable = other.inheritable;
	this -> effective = other.effective;
	this -> ambient = other.ambient;
	return *this;
}

CAPS& CAPS::operator =(const std::initializer_list<std::pair<CAP::SET, CAPS::SET>>& values) {

	this -> clear();

	for ( auto& p : values ) {

		if ( p.first == CAP::SET::BOUNDING )
			this -> bounding = p.second;
		else if ( p.first == CAP::SET::PERMITTED )
			this -> permitted = p.second;
		else if ( p.first == CAP::SET::INHERITABLE )
			this -> inheritable = p.second;
		else if ( p.first == CAP::SET::EFFECTIVE )
			this -> effective = p.second;
		else if ( p.first == CAP::SET::AMBIENT )
			this -> ambient = p.second;
	}

	return *this;
}

CAPS& CAPS::operator =(const CAPS::LIST& l) {

	this -> clear();

	for ( auto& p : l ) {

		if ( p.second.empty())
			continue;

		for ( auto& s: p.second ) {

			if ( s == CAP::SET::BOUNDING )
				this -> bounding += p.first;
			else if ( s == CAP::SET::PERMITTED )
				this -> permitted += p.first;
			else if ( s == CAP::SET::INHERITABLE )
				this -> inheritable += p.first;
			else if ( s == CAP::SET::EFFECTIVE )
				this -> effective += p.first;
			else if ( s == CAP::SET::AMBIENT )
				this -> ambient += p.first;
 		}
	}

	return *this;
}

CAPS::operator CAPS::LIST() const {
	return this -> to_list(false);
}

CAPS::LIST CAPS::to_list(bool all) const {

	std::map<CAP, std::set<CAP::SET>> res = {};

	for ( const auto& e : CAPS::all()) {

		if ( !all && !this -> bounding.contains(e) && !this -> permitted.contains(e) &&
			!this -> inheritable.contains(e) && !this -> effective.contains(e) &&
			!this -> ambient.contains(e))
			continue;

		std::set<CAP::SET> sets = {};
		if ( this -> bounding.contains(CAP(e)))
			sets.emplace(CAP::SET::BOUNDING);
		if ( this -> permitted.contains(CAP(e)))
			sets.emplace(CAP::SET::PERMITTED);
		if ( this -> inheritable.contains(CAP(e)))
			sets.emplace(CAP::SET::INHERITABLE);
		if ( this -> effective.contains(CAP(e)))
			sets.emplace(CAP::SET::EFFECTIVE);
		if ( this -> ambient.contains(CAP(e)))
			sets.emplace(CAP::SET::AMBIENT);

		res.emplace(CAP(e), sets);
	}

	return res;
}

CAPS::SET& CAPS::at(const CAP::SET& set) {

	if ( set == CAP::SET::BOUNDING )
		return this -> bounding;
	else if ( set == CAP::SET::PERMITTED )
		return this -> permitted;
	else if ( set == CAP::SET::INHERITABLE )
		return this -> inheritable;
	else if ( set == CAP::SET::EFFECTIVE )
		return this -> effective;

	return this -> ambient;
}

const CAPS::SET& CAPS::at(const CAP::SET& set) const {

	if ( set == CAP::SET::BOUNDING )
		return std::as_const(this -> bounding);
	else if ( set == CAP::SET::PERMITTED )
		return std::as_const(this -> permitted);
	else if ( set == CAP::SET::INHERITABLE )
		return std::as_const(this -> inheritable);
	else if ( set == CAP::SET::EFFECTIVE )
		return std::as_const(this -> effective);

	return std::as_const(this -> ambient);
}

CAPS::SET& CAPS::operator [](const CAP::SET& set) {
	return this -> at(set);
}

const CAPS::SET& CAPS::operator [](const CAP::SET& set) const {
	return this -> at(set);
}

CAPS::iterator CAPS::begin() {
	return CAPS::iterator(*this, 0);
}

CAPS::const_iterator CAPS::begin() const {
	return CAPS::const_iterator(*this, 0);
}

CAPS::const_iterator CAPS::cbegin() const {
	return CAPS::const_iterator(*this, 0);
}

CAPS::iterator CAPS::end() {
	return CAPS::iterator(*this, 5);
}

CAPS::const_iterator CAPS::end() const {
	return CAPS::const_iterator(*this, 5);
}

CAPS::const_iterator CAPS::cend() const {
	return CAPS::const_iterator(*this, 5);
}

bool CAPS::empty() const {
	return this -> bounding.empty() && this -> permitted.empty() &&
		this -> inheritable.empty() && this -> effective.empty() &&
		this -> ambient.empty();
}

CAPS::size_type CAPS::size() const {
	return CAP::sets_max();
}

CAPS::size_type CAPS::max_size() const {
	return CAP::sets_max();
}

void CAPS::clear() {
	this -> bounding.clear();
	this -> permitted.clear();
	this -> inheritable.clear();
	this -> effective.empty();
	this -> ambient.empty();
}

void CAPS::emplace(const CAP::SET& set, const CAPS::SET& cs) {

	if ( set == CAP::SET::BOUNDING )
		this -> bounding = cs;
	else if ( set == CAP::SET::PERMITTED )
		this -> permitted = cs;
	else if ( set == CAP::SET::INHERITABLE )
		this -> inheritable = cs;
	else if ( set == CAP::SET::EFFECTIVE )
		this -> effective = cs;
	else if ( set == CAP::SET::AMBIENT )
		this -> ambient = cs;
}

void CAPS::emplace(const CAP::SET& set, const CAP& c) {

	if ( set == CAP::SET::BOUNDING )
		this -> bounding += c;
	else if ( set == CAP::SET::PERMITTED )
		this -> permitted += c;
	else if ( set == CAP::SET::INHERITABLE )
		this -> inheritable += c;
	else if ( set == CAP::SET::EFFECTIVE )
		this -> effective += c;
	else if ( set == CAP::SET::AMBIENT )
		this -> ambient += c;
}

void CAPS::emplace(const CAP::SET& set, const CAP::TYPE& c) {

	this -> emplace(set, CAP(c));
}

void CAPS::erase(const CAP::SET& set) {

	if ( set == CAP::SET::BOUNDING )
		this -> bounding.clear();
	else if ( set == CAP::SET::PERMITTED )
		this -> permitted.clear();
	else if ( set == CAP::SET::INHERITABLE )
		this -> inheritable.clear();
	else if ( set == CAP::SET::EFFECTIVE )
		this -> effective.clear();
	else if ( set == CAP::SET::AMBIENT )
		this -> ambient.clear();
}

void CAPS::erase(const CAP::SET& set, const CAP& c) {

	if ( set == CAP::SET::BOUNDING )
		this -> bounding -= c;
	else if ( set == CAP::SET::PERMITTED )
		this -> permitted -= c;
	else if ( set == CAP::SET::INHERITABLE )
		this -> inheritable -= c;
	else if ( set == CAP::SET::EFFECTIVE )
		this -> effective -= c;
	else if ( set == CAP::SET::AMBIENT )
		this -> ambient -= c;
}

void CAPS::erase(const CAP::SET& set, const CAP::TYPE& c) {

	this -> erase(set, CAP(c));
}

CAPS::iterator CAPS::find(const CAP::SET& set) {

	return CAPS::iterator(*this, CAPS::set_to_idx(set));
}

CAPS::const_iterator CAPS::find(const CAP::SET& set) const {

	return CAPS::const_iterator(*this, CAPS::set_to_idx(set));
}

bool CAPS::contains(const CAP::SET& set) const {

	return set == CAP::SET::BOUNDING || set == CAP::SET::PERMITTED || set == CAP::SET::INHERITABLE ||
			set == CAP::SET::EFFECTIVE || set == CAP::SET::AMBIENT;
}

CAPS::CAPS() {
	this -> bounding = {};
	this -> permitted = {};
	this -> inheritable = {};
	this -> effective = {};
	this -> ambient = {};
}

CAPS::CAPS(const CAPS& other) {
	this -> bounding = other.bounding;
	this -> permitted = other.permitted;
	this -> inheritable = other.inheritable;
	this -> effective = other.effective;
	this -> ambient = other.ambient;
}

CAPS::CAPS(const std::initializer_list<std::pair<CAP::SET, CAPS::SET>>& values) {

	this -> bounding = {};
	this -> permitted = {};
	this -> inheritable = {};
	this -> effective = {};
	this -> ambient = {};

	for ( auto& p : values ) {

		if ( p.first == CAP::SET::BOUNDING )
			this -> bounding = p.second;
		else if ( p.first == CAP::SET::PERMITTED )
			this -> permitted = p.second;
		else if ( p.first == CAP::SET::INHERITABLE )
			this -> inheritable = p.second;
		else if ( p.first == CAP::SET::EFFECTIVE )
			this -> effective = p.second;
		else if ( p.first == CAP::SET::AMBIENT )
			this -> ambient = p.second;
	}
}

CAPS::CAPS(const CAPS::LIST& l) {

	this -> bounding = {};
	this -> permitted = {};
	this -> inheritable = {};
	this -> effective = {};
	this -> ambient = {};

	for ( auto& p : l ) {

		if ( p.second.empty())
			continue;

		for ( auto& s: p.second ) {

			if ( s == CAP::SET::BOUNDING )
				this -> bounding += p.first;
			else if ( s == CAP::SET::PERMITTED )
				this -> permitted += p.first;
			else if ( s == CAP::SET::INHERITABLE )
				this -> inheritable += p.first;
			else if ( s == CAP::SET::EFFECTIVE )
				this -> effective += p.first;
			else if ( s == CAP::SET::AMBIENT )
				this -> ambient += p.first;
		}
	}
}

CAP::SET CAPS::idx_to_set(int idx) {

	switch ( idx ) {
		case 0: return CAP::SET::BOUNDING;
		case 1: return CAP::SET::PERMITTED;
		case 2: return CAP::SET::INHERITABLE;
		case 3: return CAP::SET::EFFECTIVE;
		default: return CAP::SET::AMBIENT;
	}
	return CAP::SET::AMBIENT;
}

int CAPS::set_to_idx(const CAP::SET& set) {

	if ( set == CAP::SET::BOUNDING )
		return 0;
	else if ( set == CAP::SET::PERMITTED )
		return 1;
	else if ( set == CAP::SET::INHERITABLE )
		return 2;
	else if ( set == CAP::SET::EFFECTIVE )
		return 3;
	else if ( set == CAP::SET::AMBIENT )
		return 4;

	return 4;
}

void CAPS::update_ambient(const CAPS::SET& _ambient) {

	this -> ambient.clear();
	this -> ambient = _ambient;
}

void CAPS::update_ambient(const CAPS::SET& _ambient) const {
}

void CAPS::set() const {

	bool validated = true;
	bool updated = false;

	CAPS::SET all = CAPS::all();
	CAPS::SET _bounding = this -> bounding;
	CAPS::SET _permitted = this -> permitted;
	CAPS::SET _inheritable = this -> inheritable;
	CAPS::SET _effective = this -> effective;
	CAPS::SET _ambient = this -> ambient;

	for ( const auto& e : all ) {

		if ( _ambient.contains(e) && !_bounding.contains(e) && !_permitted.contains(e) && !_inheritable.contains(e)) {
			std::cerr << "removed " << e.to_string() << " from ambient set, capability must also be included in bounding, permitted and inheritable sets" << std::endl;
			_ambient.erase(e);
			validated = false;
		}

		if ( !_bounding.contains(e) && !_permitted.contains(e) && !_inheritable.contains(e) &&
			!_effective.contains(e) && !_ambient.contains(e))
			continue;

		int sets = 0;

		if ( _bounding.contains(e))
			sets |= CAPNG_BOUNDING_SET;
		if ( _permitted.contains(e))
			sets |= CAPNG_PERMITTED;
		if ( _inheritable.contains(e))
			sets |= CAPNG_INHERITABLE;
		if ( _effective.contains(e))
			sets |= CAPNG_EFFECTIVE;
		if ( _ambient.contains(e))
			sets |= CAPNG_AMBIENT;

		if ( !updated ) {
			updated = true;
			capng_clear(CAPNG_SELECT_ALL);
		}

		capng_update(CAPNG_ADD, sets, e.value());
	}

	if ( updated && capng_apply(CAPNG_SELECT_ALL))
		throw std::runtime_error("failed to set capabilities, " + err_desc());

	if ( !validated )
		this -> update_ambient(_ambient);
}

CAPS CAPS::get(pid_t pid) {

	capng_clear(CAPNG_SELECT_ALL);
	capng_setpid(pid < 1 ? ::getpid() : pid);

	if ( capng_get_caps_process())
		throw std::runtime_error("failed to get capabilities for pid " + std::to_string(pid) + ", " + err_desc());

	CAPS::SET all = CAPS::all();
	CAPS::SET _bounding = {};
	CAPS::SET _permitted = {};
	CAPS::SET _inheritable = {};
	CAPS::SET _effective = {};
	CAPS::SET _ambient = {};

	for ( const auto& e : all ) {

		if ( capng_have_capability(CAPNG_BOUNDING_SET, e.value()) != 0 )
			_bounding += e;
		if ( capng_have_capability(CAPNG_PERMITTED, e.value()) != 0 )
			_permitted += e;
		if ( capng_have_capability(CAPNG_INHERITABLE, e.value()) != 0 )
			_inheritable += e;
		if ( capng_have_capability(CAPNG_EFFECTIVE, e.value()) != 0 )
			_effective += e;
		if ( capng_have_capability(CAPNG_AMBIENT, e.value()) != 0 )
			_ambient += e;
	}

	return {
		{ CAP::SET::BOUNDING, _bounding },
		{ CAP::SET::PERMITTED, _permitted },
		{ CAP::SET::INHERITABLE, _inheritable },
		{ CAP::SET::EFFECTIVE, _effective },
		{ CAP::SET::AMBIENT, _ambient }
	};

}

void CAPS::lock() {

	if ( capng_lock() != 0 )
		throw std::runtime_error("prctl failed, " + err_desc());
}

static std::string short_cap(const CAP& c) {

	std::string s = c.to_string();

	if ( s.substr(0, 4) == "cap_" )
		s.erase(0, 4);

	return s;
}

void CAPS::set_user(uid_t uid, gid_t gid, const std::set<gid_t>& additional_gids) const {

	bool validated = true;

	CAPS::SET all = CAPS::all();
	CAPS::SET _bounding = this -> bounding;
	CAPS::SET _permitted = this -> permitted;
	CAPS::SET _inheritable = this -> inheritable;
	CAPS::SET _effective = this -> effective;
	CAPS::SET _ambient = this -> ambient;

	for ( const auto& e : all ) {

		if ( _ambient.contains(e) && !_bounding.contains(e) && !_permitted.contains(e) && !_inheritable.contains(e)) {
			std::cerr << "removed " << e.to_string() << " from ambient set, capability must also be included in bounding, permitted and inheritable sets" << std::endl;
			_ambient.erase(e);
			validated = false;
		}
	}

	for ( const CAP& c : { CAP(CAP::SETPCAP), CAP(CAP::SETGID), CAP(CAP::SETUID) }) {
		_permitted += c;
		_effective += c;
	}

	if ( ::prctl(PR_SET_SECUREBITS, SECBIT_NO_SETUID_FIXUP) != 0 )
		throw std::runtime_error("prctl failed to set securebits, " + err_desc());

	if ( ::prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0 )
		throw std::runtime_error("prctl failed to set keepcaps, " + err_desc());

	CAPS _caps = {
		{ CAP::SET::BOUNDING, _bounding },
		{ CAP::SET::PERMITTED, _permitted },
		{ CAP::SET::INHERITABLE, _inheritable },
		{ CAP::SET::EFFECTIVE, _effective },
		{ CAP::SET::AMBIENT, _ambient }
	};

	try {
		_caps.set();
	} catch ( ... ) {

		int _errno = errno;
		if ( ::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) != 0 )
			errno = _errno;

		throw std::runtime_error("failed to set capabilities for user, " + err_desc());
	}

	if ( passwd* pw = ::getpwuid(uid); gid >= 0 && pw != nullptr && ::initgroups(pw -> pw_name, gid) != 0 ) {

		int _errno = errno;
		if ( ::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) != 0 )
			errno = _errno;

		throw std::runtime_error("failed to initgroups, " + err_desc());
	}

	if ( gid >= 0 && ::setresgid(gid, gid, gid) != 0 ) {

		int _errno = errno;
		if ( ::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) != 0 )
			errno = _errno;

		throw std::runtime_error("failed to set group to " + std::to_string(gid) + ", " + err_desc());
	}

	if ( ::setresuid(uid, uid, uid) != 0 ) {

		int _errno = errno;
		if ( ::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) != 0 )
			errno = _errno;

		throw std::runtime_error("failed to set user to " + std::to_string(uid) + ", " + err_desc());
	}

	if ( !additional_gids.empty()) {

		std::vector<gid_t> gids;
		for ( auto& g : additional_gids )
			if ( g != gid )
				gids.push_back(g);

		if ( !gids.empty() && ::setgroups(gids.size(), &gids[0]) != 0 )
			throw std::runtime_error("failed to set groups, " + err_desc());
	}

	if ( ::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) != 0 )
		throw std::runtime_error("prctl failed to unset keepcaps, " + err_desc());

	std::vector<std::string> caps_to_drop;

	for ( const CAP& cap : { CAP(CAP::SETPCAP), CAP(CAP::SETGID), CAP(CAP::SETUID), CAP(CAP::SYS_CHROOT), CAP(CAP::SETFCAP) }) {

		if ( !this -> permitted.contains(cap)) {

			caps_to_drop.push_back(short_cap(cap));
			capng_update(CAPNG_DROP, CAPNG_PERMITTED, cap.value());
		}

		if ( !this -> effective.contains(cap)) {

			if ( caps_to_drop.back() != short_cap(cap))
				caps_to_drop.push_back(short_cap(cap));
			capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, cap.value());
		}
	}


	if ( !caps_to_drop.empty() && capng_apply(CAPNG_SELECT_CAPS) != 0 ) {

		std::vector<std::string>::size_type i = 0;
		std::string caps_str;

		for ( const auto& _s : caps_to_drop ) {

			i++;
			if ( _s.empty())
				continue;

			if ( !caps_str.empty())
				caps_str += i == caps_to_drop.size() ? " and " : ", ";
			caps_str += _s;
		}

		throw std::runtime_error("failed to drop capabilities " + caps_str + ", " + err_desc());
        }

	if ( !validated )
		this -> update_ambient(_ambient);
}

void CAPS::validate_ambient() {

	CAPS::SET all = CAPS::all();
	CAPS::SET _ambient = this -> ambient;

	std::string removed;

        for ( const auto& e : all ) {

                if ( _ambient.contains(e) && !this -> bounding.contains(e) &&
			!this -> permitted.contains(e) && !this -> inheritable.contains(e)) {

			if ( !removed.empty())
				removed += ' ';

			removed += e.to_string();
                        _ambient.erase(e);
                }
        }

	if ( !removed.empty()) {

		this -> ambient = ambient;

		throw std::runtime_error("following capabilities were removed from ambient set, because they are not part of bounding, permitted and inheritable sets:\n" +
			removed);
	}
}

std::ostream& operator <<(std::ostream& os, const CAPS& c) {

	std::string s;

	for ( auto it = c.begin(); it != c.end(); it++ ) {

		if ( !s.empty())
			s += '\n';

		if ( it.set() == CAP::SET::BOUNDING )
			s += "bounding:    ";
		else if ( it.set() == CAP::SET::PERMITTED )
			s += "permitted:   ";
		else if ( it.set() == CAP::SET::INHERITABLE )
			s += "inheritable: ";
		else if ( it.set() == CAP::SET::EFFECTIVE )
			s += "effective:   ";
		else if ( it.set() == CAP::SET::AMBIENT )
			s += "ambient:     ";

		std::string l;

		for ( auto& n : *it ) {

			if ( !l.empty())
				l += ", ";

			l += n.to_string();
		}

		if ( it -> empty())
			s += '-';

		s += l;
	}

	os << s;
        return os;
}

std::ostream& operator <<(std::ostream& os, const CAPS::LIST& l) {

	std::string res;

	for ( const auto& e : l ) {

		CAP c = e.first;
		std::set<CAP::SET> sets = e.second;

		if ( !res.empty())
			res += '\n';

		res += c.to_string();
		res += c.type() != CAP::CHECKPOINT_RESTORE ? "\t\t" : "\t";

		if ( c.type() != CAP::DAC_OVERRIDE && c.type() != CAP::DAC_READ_SEARCH && c.type() != CAP::LINUX_IMMUTABLE &&
			c.type() != CAP::NET_BIND_SERVICE && c.type() != CAP::NET_BROADCAST && c.type() != CAP::SYS_RESOURCE &&
			c.type() != CAP::SYS_TTY_CONFIG && c.type() != CAP::AUDIT_CONTROL && c.type() != CAP::MAC_OVERRIDE &&
			c.type() != CAP::BLOCK_SUSPEND )
			res += c.type() != CAP::BPF ? "\t" : "\t\t";

		res +=
			" bounding: " + std::string(sets.find(CAP::SET::BOUNDING) != sets.end() ? "true " : "false" ) +
			" effective: " + std::string(sets.find(CAP::SET::EFFECTIVE) != sets.end() ? "true " : "false" ) +
			" permitted: " + std::string(sets.find(CAP::SET::PERMITTED) != sets.end() ? "true " : "false" ) +
			" inheritable: " + std::string(sets.find(CAP::SET::INHERITABLE) != sets.end() ? "true " : "false" ) +
			" ambient: " + std::string(sets.find(CAP::SET::AMBIENT) != sets.end() ? "true" : "false" );
	}

	os << res;
	return os;
}
