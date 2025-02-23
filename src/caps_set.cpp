#include "capabilities/caps.hpp"

CAPS::SET& CAPS::SET::operator =(const CAPS::SET& other) {

	this -> _s = other._s;
	return *this;
}

CAPS::SET& CAPS::SET::operator =(const std::initializer_list<CAP>& l) {

	this -> _s.clear();
	for ( const CAP& c : l ) {

		if ( !this -> contains(c))
			this -> _s.emplace(c);
	}
	return *this;
}

CAPS::SET& CAPS::SET::operator =(const CAP& c) {

	this -> _s.clear();
	this -> _s.emplace(c);
	return *this;
}

CAPS::SET& CAPS::SET::operator +=(const CAPS::SET& other) {

	for ( const CAP& c : other ) {

		if ( !this -> contains(c))
			this -> _s.emplace(c);
	}
	return *this;
}

CAPS::SET& CAPS::SET::operator +=(const std::initializer_list<CAP>& l) {

	for ( const CAP& c : l ) {

		if ( !this -> contains(c))
			this -> _s.emplace(c);
	}
	return *this;
}

CAPS::SET& CAPS::SET::operator +=(const CAP& c) {

	if ( !this -> contains(c))
		this -> _s.emplace(c);
	return *this;
}

CAPS::SET& CAPS::SET::operator -=(const CAPS::SET& other) {

	for ( const CAP& c : other ) {

		if ( this -> contains(c))
			this -> _s.erase(c);
	}
	return *this;
}

CAPS::SET& CAPS::SET::operator -=(const std::initializer_list<CAP>& l) {

	for ( const CAP& c : l ) {

		if ( this -> contains(c))
			this -> _s.erase(c);
	}
	return *this;
}

CAPS::SET& CAPS::SET::operator -=(const CAP& c) {

	if ( this -> contains(c))
		this -> _s.erase(c);
	return *this;
}

CAPS::SET CAPS::SET::operator +(const CAPS::SET& other) {

	CAPS::SET n;
	n._s = this -> _s;
	n += other;
	return n;
}

CAPS::SET CAPS::SET::operator +(const std::initializer_list<CAP>& l) {

	CAPS::SET n;
	n._s = this -> _s;
	n += l;
	return n;
}

CAPS::SET CAPS::SET::operator +(const CAP& c) {

	CAPS::SET n;
	n._s = this -> _s;
	n += c;
	return n;
}

CAPS::SET CAPS::SET::operator -(const CAPS::SET& other) {

	CAPS::SET n;
	n._s = this -> _s;
	n -= other;
	return n;
}

CAPS::SET CAPS::SET::operator -(const std::initializer_list<CAP>& l) {

	CAPS::SET n;
	n._s = this -> _s;
	n -= l;
	return n;
}

CAPS::SET CAPS::SET::operator -(const CAP& c) {

	CAPS::SET n;
	n._s = this -> _s;
	n -= c;
	return n;
}

bool CAPS::SET::operator ==(const CAP& c) const {

	return this -> contains(c);
}

bool CAPS::SET::operator !=(const CAP& c) const {

	return !(this -> contains(c));
}

CAPS::SET::iterator CAPS::SET::begin() {
	return this -> _s.begin();
}

CAPS::SET::const_iterator CAPS::SET::begin() const {
	return this -> _s.cbegin();
}

CAPS::SET::const_iterator CAPS::SET::cbegin() const {
	return this -> _s.cbegin();
}

CAPS::SET::iterator CAPS::SET::end() {
	return this -> _s.end();
}

CAPS::SET::const_iterator CAPS::SET::end() const {
	return this -> _s.cend();
}

CAPS::SET::const_iterator CAPS::SET::cend() const {
	return this -> _s.cend();
}

bool CAPS::SET::empty() const {
	return this -> _s.empty();
}

CAPS::SET::size_type CAPS::SET::size() const {
	return this -> _s.size();
}

CAPS::SET::size_type CAPS::SET::max_size() const {
	return CAP::caps_max();
}

void CAPS::SET::clear() {
	this -> _s.clear();
}

void CAPS::SET::add(const CAP& c) {
	if ( !this -> contains(c))
		this -> _s.emplace(c);
}

void CAPS::SET::emplace(const CAP& c) {
	this -> add(c);
}

void CAPS::SET::insert(const CAP& c) {
	this -> add(c);
}

void CAPS::SET::remove(const CAP& c) {
	if ( this -> contains(c))
		this -> _s.erase(c);
}

void CAPS::SET::erase(const CAP& c) {
	this -> remove(c);
}

CAPS::SET::iterator CAPS::SET::find(const CAP& c) {
	return this -> _s.find(c);
}

CAPS::SET::const_iterator CAPS::SET::find(const CAP& c) const {
	return this -> _s.find(c);
}

bool CAPS::SET::contains(const CAP& c) const {
	return this -> _s.find(c) != this -> _s.end();
}

CAPS::SET::SET() {
	this -> _s.clear();
}

CAPS::SET::SET(const CAP& c) {
	this -> _s.clear();
	this -> _s.emplace(c);
}

CAPS::SET::SET(const SET& other) {
	this -> _s = other._s;
}

CAPS::SET::SET(const std::initializer_list<CAP>& l) {
	this -> _s.clear();

	for ( const CAP& c : l ) {
		if ( !this -> contains(c))
			this -> _s.emplace(c);
	}
}
