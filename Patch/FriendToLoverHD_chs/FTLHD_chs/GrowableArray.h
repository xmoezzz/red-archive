#pragma once

#include "NtDefine.h"

template<typename TYPE>
class GrowableArray
{
public:
	static const ULONG_PTR kInvalidIndex = ULONG_PTR_MAX;

protected:
	TYPE* m_Data;       // the actual array of data
	ULONG_PTR m_Size;       // # of elements (upperBound - 1)
	ULONG_PTR m_MaxSize;    // max allocated
	ULONG_PTR PadFor16;

	typedef const TYPE&             CONST_TYPE_REF;
	typedef TYPE&                   TYPE_REF;
	typedef GrowableArray<TYPE>&    SELF_TYPE_REF;

public:
	GrowableArray()
	{
		m_Data = nullptr;
		m_Size = 0;
		m_MaxSize = 0;
	}

	GrowableArray(const GrowableArray<TYPE>& a)
	{
		m_Data = nullptr;
		m_Size = 0;
		m_MaxSize = 0;

		*this = a;
	}

	GrowableArray(const TYPE* buf, ULONG_PTR Length)
	{
		m_Data = nullptr;
		m_Size = 0;
		m_MaxSize = 0;

		this->SetData(buf, Length);
	}

	NTSTATUS SetData(const TYPE* buf, ULONG_PTR Length)
	{
		FAIL_RETURN(this->SetSize(Length));

		CrtMemcpy(this->GetData(), buf, Length * sizeof(TYPE));
		this->UpdateDataCount(Length);

		return STATUS_SUCCESS;
	}

	~GrowableArray()
	{
		RemoveAll();
	}

	CONST_TYPE_REF operator[](ULONG_PTR Index) const
	{
		return GetAt(Index);
	}

	TYPE_REF operator[](ULONG_PTR Index)
	{
		return GetAt(Index);
	}

	SELF_TYPE_REF operator<<(CONST_TYPE_REF Value)
	{
		this->Add(Value);
		return *this;
	}

	SELF_TYPE_REF operator=(const GrowableArray<TYPE>& a)
	{
		if (this == &a)
			return *this;

		RemoveAll();

		TYPE *Data = a.GetData();

		for (ULONG_PTR Count = a.GetSize(); Count != 0; --Count)
			Add(*Data++);

		return *this;
	}

	ULONG_PTR Increment()
	{
		return ++m_Size;
	}

	ULONG_PTR Decrement()
	{
		return --m_Size;
	}

	ULONG_PTR UpdateDataCount(ULONG_PTR Count)
	{
		m_Size = Count;
		return Count;
	}

	NTSTATUS SetSize(ULONG_PTR NewMaxSize)
	{
		ULONG_PTR OldSize = GetSize();

		if (OldSize > NewMaxSize)
		{
			if (m_Data != nullptr)
			{
				// Removing elements. Call dtor.

				TYPE *Data = GetData() + NewMaxSize;

				for (ULONG_PTR Count = OldSize - NewMaxSize; Count != 0; --Count)
				{
					(*Data).~TYPE();
					++Data;
				}
			}
		}

		// Adjust buffer.  Note that there's no need to check for error
		// since if it happens, nOldSize == nNewMaxSize will be true.)
		NTSTATUS Status = SetSizeInternal(NewMaxSize);

		if (OldSize < NewMaxSize)
		{
			if (m_Data != nullptr)
			{
				// Adding elements. Call ctor.

				TYPE *Data = GetData() + OldSize;

				for (ULONG_PTR Count = NewMaxSize - OldSize; Count != 0; --Count)
				{
					new (Data)TYPE;
					++Data;
				}
			}
		}

		return Status;
	}

	NTSTATUS Add(CONST_TYPE_REF Value)
	{
		TYPE*       Data;
		NTSTATUS    Status;
		ULONG_PTR   NewSize;

		//NewSize = _InterlockedIncrementPtr(&m_Size);
		NewSize = ++m_Size;

		Status = SetSizeInternal(NewSize);
		if (!NT_SUCCESS(Status))
		{
			//_InterlockedDecrementPtr(&m_Size);
			--m_Size;
			return Status;
		}

		Data = &m_Data[NewSize - 1];

		// Construct the new element
		new (Data)TYPE;

		// Assign
		*Data = Value;

		return STATUS_SUCCESS;
	}

	NTSTATUS Insert(ULONG_PTR Index, CONST_TYPE_REF Value)
	{
		TYPE*    Data;
		NTSTATUS Status;

		// Validate index
		if (Index > m_Size)
		{
			return STATUS_INVALID_PARAMETER;
		}

		// Prepare the buffer
		Status = SetSizeInternal(m_Size + 1);
		if (!NT_SUCCESS(Status))
			return Status;

		Data = GetData() + Index;

		// Shift the array
		APICaller::RtlMoveMemory(Data + 1, Data, sizeof(*Data) * (m_Size - Index));

		// Construct the new element
		new (Data)TYPE;

		// Set the value and increase the size
		*Data = Value;
		++m_Size;

		return STATUS_SUCCESS;
	}

	NTSTATUS SetAt(ULONG_PTR Index, CONST_TYPE_REF Value)
	{
		// Validate arguments
		if (Index < 0 || Index >= m_Size)
			return STATUS_INVALID_PARAMETER;

		m_Data[Index] = Value;
		return STATUS_SUCCESS;
	}

	TYPE_REF GetLast() const
	{
		return m_Data[m_Size - 1];
	}

	TYPE_REF GetAt(ULONG_PTR Index) const
	{
		return m_Data[Index];
	}

	TYPE* GetAtPtr(ULONG_PTR Index) const
	{
		return m_Data == nullptr ? nullptr : Index > m_Size ? nullptr : &m_Data[Index];
	}

	ULONG_PTR GetSize() const
	{
		return m_Size;
	}

	TYPE* GetData() const
	{
		return m_Data;
	}

	BOOL Contains(CONST_TYPE_REF Value)
	{
		return (kInvalidIndex != IndexOf(Value));
	}

	ULONG_PTR IndexOf(CONST_TYPE_REF Value)
	{
		return IndexOf(Value, 0, m_Size);
	}

	ULONG_PTR IndexOf(CONST_TYPE_REF Value, ULONG_PTR Start)
	{
		return IndexOf(Value, Start, m_Size - Start);
	}

	ULONG_PTR IndexOf(CONST_TYPE_REF Value, ULONG_PTR Start, ULONG_PTR NumberOfElements)
	{
		TYPE *Data;

		// Validate arguments
		if (Start >= m_Size || Start + NumberOfElements > m_Size)
			return kInvalidIndex;

		Data = GetData() + Start;

		// Search
		for (ULONG_PTR Count = NumberOfElements - Start; Count != 0; ++Data, --Count)
		{
			if (*Data == Value)
				return Data - GetData();
		}

		// Not found
		return kInvalidIndex;
	}

	ULONG_PTR LastIndexOf(CONST_TYPE_REF Value)
	{
		return LastIndexOf(Value, m_Size - 1, m_Size);
	}

	ULONG_PTR LastIndexOf(CONST_TYPE_REF Value, ULONG_PTR Index)
	{
		return LastIndexOf(Value, Index, Index + 1);
	}

	ULONG_PTR LastIndexOf(CONST_TYPE_REF Value, ULONG_PTR End, ULONG_PTR NumberOfElements)
	{
		// Validate arguments
		if (End < 0 || End >= m_nSize || End < NumberOfElements)
		{
			return kInvalidIndex;
		}

		// Search
		TYPE *Data = GetData() + End;

		for (ULONG_PTR Count = End - NumberOfElements; Count != 0; --Data, --Count)
		{
			if (*Data == Value)
				return Data - GetData();
		}

		// Not found
		return kInvalidIndex;
	}

	NTSTATUS Remove(ULONG_PTR Index)
	{
		TYPE *Data;

		if (Index >= m_Size)
			return STATUS_INVALID_PARAMETER;

		// Destruct the element to be removed
		Data = GetData() + Index;
		(*Data).~TYPE();

		// Compact the array and decrease the size
		RtlMoveMemory(Data, Data + 1, sizeof(*Data) * (m_Size - (Index + 1)));
		--m_Size;

		return STATUS_SUCCESS;
	}

	void RemoveAll()
	{
		SetSize(0);
	}

	void Reset()
	{
		SetSize(0);
		//        m_Size = 0;
	}

	TYPE* begin() const
	{
		return GetData();
	}

	TYPE* end() const
	{
		return GetData() + GetSize();
	}

protected:
	NTSTATUS SetSizeInternal(ULONG_PTR NewMaxSize)  // This version doesn't call ctor or dtor.
	{
		if ((NewMaxSize > ULONG_PTR_MAX / sizeof(TYPE)))
			return STATUS_INVALID_PARAMETER;

		if (NewMaxSize == 0)
		{
			// Shrink to 0 size & cleanup
			FreeMemoryP(m_Data);
			m_Data = nullptr;

			m_MaxSize = 0;
			m_Size = 0;
		}
		else if (m_Data == nullptr || NewMaxSize > m_MaxSize)
		{
			// Grow array
			ULONG_PTR GrowBy = (m_MaxSize == 0) ? 16 : m_MaxSize;

			// Limit nGrowBy to keep m_nMaxSize less than INT_MAX
			if ((ULONG64)m_MaxSize + GrowBy > ULONG_PTR_MAX)
				GrowBy = ULONG_PTR_MAX - m_MaxSize;

			NewMaxSize = MAX(NewMaxSize, m_MaxSize + GrowBy);

			TYPE* DataNew = (TYPE *)ReAllocateMemoryP(m_Data, NewMaxSize * sizeof(*DataNew));

			if (DataNew == nullptr)
				return STATUS_NO_MEMORY;

			m_Data = DataNew;
			m_MaxSize = NewMaxSize;
		}

		return STATUS_SUCCESS;
	}
};

