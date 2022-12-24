#include "Swap.h"

#include "Str.h"
#include "ListBase.h"
#include "VectorBase.h"
#include "HashBase.h"

namespace Anz
{

	template <> void Swap<String>(String& first, String& second)
	{
		first.Swap(second);
	}

	template <> void Swap<VectorBase>(VectorBase& first, VectorBase& second)
	{
		first.Swap(second);
	}

	template <> void Swap<ListBase>(ListBase& first, ListBase& second)
	{
		first.Swap(second);
	}

	template <> void Swap<HashBase>(HashBase& first, HashBase& second)
	{
		first.Swap(second);
	}

}

