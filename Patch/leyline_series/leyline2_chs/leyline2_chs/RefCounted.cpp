#include "RefCounted.h"

namespace Anz
{

	RefCounted::RefCounted() :
		refCount_(new RefCount())
	{
		// Hold a weak ref to self to avoid possible double delete of the refcount
		(refCount_->weakRefs_)++;
	}

	RefCounted::~RefCounted()
	{
		// Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
		refCount_->refs_ = -1;
		(refCount_->weakRefs_)--;
		if (!refCount_->weakRefs_)
			delete refCount_;

		refCount_ = 0;
	}

	void RefCounted::AddRef()
	{
		(refCount_->refs_)++;
	}

	void RefCounted::ReleaseRef()
	{
		(refCount_->refs_)--;
		if (!refCount_->refs_)
			delete this;
	}

	int RefCounted::Refs() const
	{
		return refCount_->refs_;
	}

	int RefCounted::WeakRefs() const
	{
		// Subtract one to not return the internally held reference
		return refCount_->weakRefs_ - 1;
	}
}


