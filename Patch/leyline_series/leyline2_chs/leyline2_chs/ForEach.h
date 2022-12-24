#pragma once

#include "Vector.h"
#include "List.h"
#include "HashSet.h"
#include "HashMap.h"

#include <algorithm>

// VS2010+ and other compilers: use std::begin(), std::end() & range based for
// C++11 features need to be enabled
#if !defined(_MSC_VER) || _MSC_VER > 1600
#define foreach(VAL, VALS) for (VAL : VALS)
#else

namespace Urho3D
{

	template<typename T>
	struct false_wrapper {
		false_wrapper(const T& value) : value(value) { }
		operator bool() const { return false; }
		T value;
	};

	template<typename T>
	false_wrapper<T> make_false_wrapper(const T& value) {
		return false_wrapper<T>(value);
	}

	// vector support functions

	template <class T>
	Urho3D::RandomAccessIterator<T> Begin(Urho3D::Vector<T> &v) {
		return v.Begin();
	}
	template <class T>
	Urho3D::RandomAccessIterator<T> Begin(Urho3D::Vector<T> *v) {
		return v->Begin();
	}

	template <class T>
	Urho3D::RandomAccessConstIterator<T> Begin(const Urho3D::Vector<T> &v) {
		return v.Begin();
	}
	template <class T>
	Urho3D::RandomAccessConstIterator<T> Begin(const Urho3D::Vector<T> *v) {
		return v->Begin();
	}

	template <class T>
	Urho3D::RandomAccessIterator<T> End(Urho3D::Vector<T> &v) {
		return v.End();
	}
	template <class T>
	Urho3D::RandomAccessIterator<T> End(Urho3D::Vector<T> *v) {
		return v->End();
	}

	template <class T>
	Urho3D::RandomAccessConstIterator<T> End(const Urho3D::Vector<T> &v) {
		return v.End();
	}
	template <class T>
	Urho3D::RandomAccessConstIterator<T> End(const Urho3D::Vector<T> *v) {
		return v->End();
	}

	// podvector support functions

	template <class T>
	Urho3D::RandomAccessIterator<T> Begin(Urho3D::PODVector<T> &v) {
		return v.Begin();
	}
	template <class T>
	Urho3D::RandomAccessIterator<T> Begin(Urho3D::PODVector<T> *v) {
		return v->Begin();
	}

	template <class T>
	Urho3D::RandomAccessConstIterator<T> Begin(const Urho3D::PODVector<T> &v) {
		return v.Begin();
	}
	template <class T>
	Urho3D::RandomAccessConstIterator<T> Begin(const Urho3D::PODVector<T> *v) {
		return v->Begin();
	}

	template <class T>
	Urho3D::RandomAccessIterator<T> End(Urho3D::PODVector<T> &v) {
		return v.End();
	}
	template <class T>
	Urho3D::RandomAccessIterator<T> End(Urho3D::PODVector<T> *v) {
		return v->End();
	}

	template <class T>
	Urho3D::RandomAccessConstIterator<T> End(const Urho3D::PODVector<T> &v) {
		return v.End();
	}
	template <class T>
	Urho3D::RandomAccessConstIterator<T> End(const Urho3D::PODVector<T> *v) {
		return v->End();
	}

}

#define foreach(VAL, VALS) \
    if (const auto& _foreach_begin = Urho3D::make_false_wrapper(Urho3D::Begin(VALS))) { } else \
    if (const auto& _foreach_end = Urho3D::make_false_wrapper(Urho3D::End(VALS))) { } else \
    for (auto it = _foreach_begin.value; it != _foreach_end.value; ++it) \
    if (bool _foreach_flag = false) { } else \
    for (VAL = *it; !_foreach_flag; _foreach_flag = true)

#endif
