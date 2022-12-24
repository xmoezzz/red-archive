#include "VectorBase.h"

namespace Anz
{
	unsigned char* VectorBase::AllocateBuffer(unsigned size)
	{
		return new unsigned char[size];
	}
}
