#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/KillNPC.h>

#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class KillNPC::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const KillNPC*					aObjective)
			: m_objective(aObjective)
		{

		}

		virtual ~Instance()
		{

		}

		// ObjectiveInstanceBase implementation
		void
		OnEntityObjectiveEvent(
			EntityObjectiveEvent::Type		aEntityObjectiveEvent) override
		{
			if(aEntityObjectiveEvent == EntityObjectiveEvent::TYPE_KILL_NPC && m_killed < m_objective->m_count)
				m_killed++;
		}

		void	
		ToStream(
			IWriter*						aWriter) const override
		{
			aWriter->WriteUInt(m_killed);
		}
		
		bool	
		FromStream(
			IReader*						aReader) override
		{
			if(!aReader->ReadUInt(m_killed))
				return false;
			return true;
		}

	private:
		
		const KillNPC*	m_objective = NULL;
		uint32_t		m_killed = 0;
	};

	//-------------------------------------------------------------------------------------------

	KillNPC::KillNPC()
		: ObjectiveTypeBase(ID)
	{

	}
	
	KillNPC::~KillNPC()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	KillNPC::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "entities")
				aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
			else if(aChild->m_name == "count")  
				m_count = aChild->GetUInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	KillNPC::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInts(m_entityIds);
		aWriter->WriteUInt(m_count);
	}
	
	bool		
	KillNPC::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInts(m_entityIds))
			return false;
		if (!aReader->ReadUInt(m_count))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	KillNPC::CreateInstance() const 
	{
		return new Instance(this);
	}

	void		
	KillNPC::GetWatchedEntities(
		std::vector<uint32_t>&				aOutEntityIds) const
	{
		aOutEntityIds = m_entityIds;
	}

}