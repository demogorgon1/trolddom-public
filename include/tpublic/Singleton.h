#pragma once

namespace tpublic
{

	struct ISingleton
	{
				ISingleton();
		virtual	~ISingleton();
	};

	void		DestroyAllSingletons();

	// Singleton creation is thread safe
	// Destroy is NOT thread safe
	template<typename T>
	class Singleton : public ISingleton
	{
	public:
		static const int SINGLETON_READY = -1;

		static T*	
		GetInstance()
		{
			if(m_singletonGuard != SINGLETON_READY)
			{
				if(++m_singletonGuard > 1)
				{
					// Wait for singleton to be created
					while(m_singletonGuard != SINGLETON_READY)
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				else
				{
					// Create singleton
					try
					{
						m_object = new T();
					}
					catch(...)
					{
						assert(false);
					}

					m_singletonGuard = SINGLETON_READY;
				}
			}

			assert(m_object != NULL);
				
			return m_object;
		}

		void
		Destroy()
		{
			// CAREFUL with Destroy(). NOT threadsafe.
			if(m_singletonGuard != SINGLETON_READY)
				return;

			if(m_object != NULL)
				delete m_object;

			m_object = NULL;

			m_singletonGuard = 0;
		}

	private:
		static std::atomic_int		m_singletonGuard;
		static T*					m_object;
	};

	template<typename T>
	std::atomic_int Singleton<T>::m_singletonGuard;

	template<typename T>
	T* Singleton<T>::m_object;

};