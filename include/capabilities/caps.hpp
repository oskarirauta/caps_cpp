#pragma once

#include <map>
#include <set>
#include <ostream>
#include <utility>
#include <sys/types.h>
#include "capabilities/cap.hpp"

class CAPS {

	public:

		class SET {

			public:

				using size_type = size_t;
				using iterator = std::set<CAP>::iterator;
				using const_iterator = std::set<CAP>::const_iterator;

				SET& operator =(const SET& other);
				SET& operator =(const std::initializer_list<CAP>& l);
				SET& operator =(const CAP& c);

				SET& operator +=(const SET& other);
				SET& operator +=(const std::initializer_list<CAP>& l);
				SET& operator +=(const CAP& c);

				SET& operator -=(const SET& other);
				SET& operator -=(const std::initializer_list<CAP>& l);
				SET& operator -=(const CAP& c);

				SET operator +(const SET& other);
				SET operator +(const std::initializer_list<CAP>& l);
				SET operator +(const CAP& c);

				SET operator -(const SET& other);
				SET operator -(const std::initializer_list<CAP>& l);
				SET operator -(const CAP& c);

				bool operator ==(const CAP& c) const;
				bool operator !=(const CAP& c) const;

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
				void add(const CAP& c);
				void emplace(const CAP& c);
				void insert(const CAP& c);
				void remove(const CAP& c);
				void erase(const CAP& c);

				iterator find(const CAP& c);
				const_iterator find(const CAP& c) const;
				bool contains(const CAP& c) const;

				SET();
				SET(const CAP& c);
				SET(const SET& other);
				SET(const std::initializer_list<CAP>& l);

			private:
				std::set<CAP> _s;

		};

		template<bool IsConst>
		class iterator_ {

			friend class CAPS;
			using parent_type = std::conditional_t<IsConst, const CAPS&, CAPS&>;
			using size_type = size_t;
			using pointer = std::conditional_t<IsConst, const CAPS::SET*, CAPS::SET*>;
			using reference = std::conditional_t<IsConst, const CAPS::SET&, CAPS::SET&>;

			private:
				parent_type parent;
				size_type idx;

				iterator_(parent_type parent, size_type idx) : parent(parent), idx(idx) {}

			public:
				iterator_(const iterator_&) = default;
				iterator_& operator=(const iterator_&) = default;

				template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
				iterator_(const iterator_<WasConst>& rhs) : idx(rhs.idx) {}

				template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
				iterator_& operator=(const iterator_<WasConst>& rhs) { idx = rhs.idx; return *this; }

				CAP::SET set() const { return CAPS::idx_to_set(this -> idx); }
				CAP::SET key() const { return this -> set(); }
				reference operator *() { return this -> parent.at(CAPS::idx_to_set(this -> idx)); }
				pointer operator ->() { return &(this -> parent.at(CAPS::idx_to_set(this -> idx))); }
				iterator_& operator ++() { if ( this -> idx < parent.max_size()) idx++; return *this; }
				iterator_ operator ++(int) { iterator_& tmp = *this; ++(*this); return tmp; }
				friend bool operator ==(const iterator_& a, const iterator_& b) { return a.idx == b.idx; }
				friend bool operator !=(const iterator_& a, const iterator_& b) { return a.idx != b.idx; }
		};

		using LIST = std::map<CAP, std::set<CAP::SET>>;
		using size_type = size_t;
		using iterator = iterator_<false>;
		using const_iterator = iterator_<true>;

		CAPS& operator =(const CAPS& other);
		CAPS& operator =(const std::initializer_list<std::pair<CAP::SET, CAPS::SET>>& values);
		CAPS& operator =(const LIST& l);

		operator LIST() const;
		LIST to_list(bool all = false) const;

		CAPS::SET& at(const CAP::SET& set);
		const CAPS::SET& at(const CAP::SET& set) const;

		CAPS::SET& operator [](const CAP::SET& set);
		const CAPS::SET& operator [](const CAP::SET& set) const;

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
		void emplace(const CAP::SET& set, const CAPS::SET& cs);
		void emplace(const CAP::SET& set, const CAP& c);
		void emplace(const CAP::SET& set, const CAP::TYPE& c);
		void erase(const CAP::SET& set);
		void erase(const CAP::SET& set, const CAP& c);
		void erase(const CAP::SET& set, const CAP::TYPE& c);

		iterator find(const CAP::SET& set);
		const_iterator find(const CAP::SET& set) const;
		bool contains(const CAP::SET& set) const;

		void set() const;
		void set_user(uid_t uid, gid_t gid, const std::set<gid_t>& additional_gids) const;

		static CAPS::SET all();
		static CAPS get(pid_t pid = -1);
		static void lock();

		CAPS();
		CAPS(const CAPS& other);
		CAPS(const std::initializer_list<std::pair<CAP::SET, CAPS::SET>>& values);
		CAPS(const LIST& l);

		friend std::ostream& operator <<(std::ostream& os, const CAPS& cs);

	private:

		void update_ambient(const CAPS::SET& _ambient);
		void update_ambient(const CAPS::SET& _ambient) const;

		static CAP::SET idx_to_set(int idx);
		static int set_to_idx(const CAP::SET& set);

		CAPS::SET bounding;
		CAPS::SET permitted;
		CAPS::SET inheritable;
		CAPS::SET effective;
		CAPS::SET ambient;
};

std::ostream& operator <<(std::ostream& os, const CAPS& cs);
std::ostream& operator <<(std::ostream& os, const CAPS::LIST& l);
