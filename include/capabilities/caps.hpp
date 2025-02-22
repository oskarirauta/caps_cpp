#pragma once

#include <map>
#include <set>
#include <ostream>
#include <utility>
#include <sys/types.h>
#include "capabilities/cap.hpp"

class CAPS {

	public:

		using LIST = std::map<CAP, std::set<CAP::SET>>;
		using size_type = std::map<CAP::SET, std::set<CAP>>::size_type;
		using iterator = std::map<CAP::SET, std::set<CAP>>::iterator;
		using const_iterator = std::map<CAP::SET, std::set<CAP>>::const_iterator;

		CAPS& operator =(const CAPS& other);
		CAPS& operator =(const std::map<CAP::SET, std::set<CAP>>& m);
		CAPS& operator =(const std::initializer_list<std::pair<CAP::SET, std::set<CAP>>>& values);
		CAPS& operator =(const LIST& l);

		operator LIST() const;
		LIST to_list() const;

		std::set<CAP>& at(const CAP::SET& set);
		const std::set<CAP>& at(const CAP::SET& set) const;

		std::set<CAP>& operator [](const CAP::SET& set);
		const std::set<CAP>& operator [](const CAP::SET& set) const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;

		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

		bool empty() const;
		size_type size() const;
		size_type max_size() const;
		void clear();
		void emplace(const CAP::SET& s, const std::set<CAP>& c);
		void erase(const CAP::SET& s);

		iterator find(const CAP::SET& s);
		const_iterator find(const CAP::SET& s) const;
		bool contains(const CAP::SET& s) const;

		CAPS();
		CAPS(const CAPS& other);
		CAPS(const std::map<CAP::SET, std::set<CAP>>& m);
		CAPS(const std::initializer_list<std::pair<CAP::SET, std::set<CAP>>>& values);
		CAPS(const LIST& l);

		static std::set<CAP::TYPE> all();
		static bool set(const CAPS& c);
		static CAPS get(pid_t pid = -1);
		static bool lock();
		static bool set_user(uid_t uid, gid_t gid, const std::set<gid_t>& additional_gids, const CAPS& caps);

		friend std::ostream& operator <<(std::ostream& os, const CAPS& cs);

	private:

		std::map<CAP::SET, std::set<CAP>> _m;
};

std::ostream& operator <<(std::ostream& os, const CAPS& cs);
std::ostream& operator <<(std::ostream& os, const CAPS::LIST& l);
