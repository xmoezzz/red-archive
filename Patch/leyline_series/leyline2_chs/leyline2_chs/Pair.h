#pragma once

#include "Hash.h"

namespace Anz
{

	/// %Pair template class.
	template <class T, class U> class Pair
	{
	public:
		/// Construct undefined.
		Pair()
		{
		}

		/// Construct with values.
		Pair(const T& first, const U& second) :
			first_(first),
			second_(second)
		{
		}

		/// Test for equality with another pair.
		bool operator ==(const Pair<T, U>& rhs) const { return first_ == rhs.first_ && second_ == rhs.second_; }

		/// Test for inequality with another pair.
		bool operator !=(const Pair<T, U>& rhs) const { return first_ != rhs.first_ || second_ != rhs.second_; }

		/// Test for less than with another pair.
		bool operator <(const Pair<T, U>& rhs) const
		{
			if (first_ < rhs.first_)
				return true;
			if (first_ != rhs.first_)
				return false;
			return second_ < rhs.second_;
		}

		/// Test for greater than with another pair.
		bool operator >(const Pair<T, U>& rhs) const
		{
			if (first_ > rhs.first_)
				return true;
			if (first_ != rhs.first_)
				return false;
			return second_ > rhs.second_;
		}

		/// Return hash value for HashSet & HashMap.
		unsigned ToHash() const { return (MakeHash(first_) & 0xffff) | (MakeHash(second_) << 16); }

		/// First value.
		T first_;
		/// Second value.
		U second_;
	};

	/// Construct a pair.
	template <class T, class U> Pair<T, U> MakePair(const T& first, const U& second)
	{
		return Pair<T, U>(first, second);
	}

}

