#pragma once

namespace tpublic
{

	class ComponentBase;

	class UniqueComponent
	{
	public:
		typedef std::function<void(ComponentBase*)> DeleteFunction;

		UniqueComponent() 
			: m_componentBase(NULL)
		{

		}

		UniqueComponent(
			UniqueComponent&&		aOther)
		{
			m_componentBase = std::move(aOther.m_componentBase);
			m_deleteFunction = std::move(aOther.m_deleteFunction);
		}

		~UniqueComponent()
		{
			if(m_componentBase != NULL)
				m_deleteFunction(m_componentBase);
		}

		UniqueComponent&
		operator=(
			UniqueComponent&&		aOther)
		{
			m_componentBase = std::move(aOther.m_componentBase);
			m_deleteFunction = std::move(aOther.m_deleteFunction);
			return *this;
		}

		void	
		Set(
			ComponentBase*			aComponentBase,
			DeleteFunction			aDeleteFunction)
		{
			assert(m_componentBase == NULL);
			assert(!m_deleteFunction);

			m_componentBase = aComponentBase;
			m_deleteFunction = aDeleteFunction;
		}

		// Data access
		bool					IsSet() const { return m_componentBase != NULL; }
		ComponentBase*			GetPointer() { return m_componentBase; }
		const ComponentBase*	GetPointer() const { return m_componentBase; }

	private:
		
		ComponentBase*	m_componentBase;
		DeleteFunction	m_deleteFunction;
	};

}