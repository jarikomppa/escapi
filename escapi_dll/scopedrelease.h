#pragma once

template <class T> class ScopedRelease
{
public:
	T *mVar;
	ScopedRelease(T * aVar)
	{
		mVar = aVar;
	}
	~ScopedRelease()
	{
		if (mVar != 0)
			mVar->Release();
	}
};
