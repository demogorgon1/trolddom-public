#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/UseAbility.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class UseAbility::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const UseAbility*				aObjective)
			: m_objective(aObjective)
		{

		}

		virtual ~Instance()
		{

		}

		// ObjectiveInstanceBase implementation
		bool	
		GetProgress(
			Progress&						aOut) override
		{
			aOut = { m_count, m_objective->m_count };
			return true;
		}

		bool	
		IsCompleted() const override
		{
			return m_count == m_objective->m_count;
		}

		uint32_t				
		GetHash() const override
		{
			return m_count;
		}

		void	
		ToStream(
			IWriter*						aWriter) const override
		{
			aWriter->WriteUInt(m_count);
		}
		
		bool	
		FromStream(
			IReader*						aReader) override
		{
			if(!aReader->ReadUInt(m_count))
				return false;
			return true;
		}

		void					
		OnAbilityEvent(
			uint32_t						aAbilityId,
			const EntityInstance*			aEntityInstance) override
		{
			if(m_objective->m_abilityId == aAbilityId && m_count < m_objective->m_count)
			{
				for(uint32_t entityId : m_objective->m_entityIds)
				{
					if(aEntityInstance->GetEntityId() == entityId)
					{
						m_count++;
						OnUpdate();
						break;
					}
				}
			}
		}

	private:
		
		const UseAbility*	m_objective = NULL;
		uint32_t			m_count = 0;
	};

	//-------------------------------------------------------------------------------------------

	UseAbility::UseAbility()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	UseAbility::~UseAbility()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	UseAbility::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "entities")
				aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
			else if(aChild->m_name == "count")  
				m_count = aChild->GetUInt32();
			else if(aChild->m_name == "ability")
				m_abilityId = aChild->GetId(DataType::ID_ABILITY);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	UseAbility::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInts(m_entityIds);
		aWriter->WriteUInt(m_count);
		aWriter->WriteUInt(m_abilityId);
	}
	
	bool		
	UseAbility::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInts(m_entityIds))
			return false;
		if (!aReader->ReadUInt(m_count))
			return false;
		if (!aReader->ReadUInt(m_abilityId))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	UseAbility::CreateInstance() const 
	{
		return new Instance(this);
	}

}