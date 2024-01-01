#include "Pcheader.h"

#include <tpublic/Singleton.h>
 
namespace tpublic
{

	namespace 
	{
		const int MAX_SINGLETONS = 100;

		ISingleton*			m_allSingletons[MAX_SINGLETONS];
		std::atomic_int		m_nextSingletonIndex = 0;
	}

	//---------------------------------------------------------------------------

	ISingleton::ISingleton()
	{
		int index = m_nextSingletonIndex++;
		assert(index < MAX_SINGLETONS);
		assert(index >= 0);

		m_allSingletons[index] = this;
	}

	ISingleton::~ISingleton()
	{
		int count = m_nextSingletonIndex;

		for(int i = 0; i < count; i++)
		{
			if(m_allSingletons[i] == this)
			{
				m_allSingletons[i] = NULL;
				break;
			}
		}
	}

	//---------------------------------------------------------------------------

	void		
	DestroyAllSingletons()
	{
		int count = m_nextSingletonIndex;

		for(int i = 0; i < count; i++)
		{
			ISingleton* t = m_allSingletons[i];

			if(t != NULL)
				delete t;
		}
	}

}