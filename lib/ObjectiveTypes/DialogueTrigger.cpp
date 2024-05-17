#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/DialogueTrigger.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class DialogueTrigger::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const DialogueTrigger*			aObjective)
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
			aOut.m_current = m_completed ? 1 : 0;
			aOut.m_goal = 1;
			return true;
		}

		bool	
		IsCompleted() const override
		{
			return m_completed;
		}

		uint32_t				
		GetHash() const override
		{
			return m_completed ? 0 : 1;
		}

		void	
		ToStream(
			IWriter*						aWriter) const override
		{
			aWriter->WriteBool(m_completed);
		}
		
		bool	
		FromStream(
			IReader*						aReader) override
		{
			if(!aReader->ReadBool(m_completed))
				return false;
			return true;
		}

		void					
		OnDialogueTrigger(
			const EntityInstance*			aEntityInstance) override
		{
			if(!m_completed)
			{
				for(uint32_t entityId : m_objective->m_entityIds)
				{
					if(aEntityInstance->GetEntityId() == entityId)
					{
						m_completed = true;
						OnUpdate();
						break;
					}
				}
			}
		}

	private:
		
		const DialogueTrigger*	m_objective = NULL;
		bool					m_completed = false;
	};

	//-------------------------------------------------------------------------------------------

	DialogueTrigger::DialogueTrigger()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	DialogueTrigger::~DialogueTrigger()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	DialogueTrigger::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "entities")
				aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	DialogueTrigger::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInts(m_entityIds);
	}
	
	bool		
	DialogueTrigger::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInts(m_entityIds))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	DialogueTrigger::CreateInstance() const 
	{
		return new Instance(this);
	}

}