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
			const EntityObjectiveEvent&		aEntityObjectiveEvent) override
		{
			if(aEntityObjectiveEvent.m_type == EntityObjectiveEvent::TYPE_KILL_NPC && m_killed < m_objective->m_count)
			{
				m_killed++;
				OnUpdate();
			}
		}

		bool	
		GetProgress(
			Progress&						aOut) override
		{
			aOut = { m_killed, m_objective->m_count };
			return true;
		}

		bool	
		IsCompleted() const override
		{
			return m_killed == m_objective->m_count;
		}

		uint32_t				
		GetHash() const override
		{
			return m_killed;
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
		: ObjectiveTypeBase(ID, FLAGS)
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
			else if (aChild->m_name == "entity")
				m_entityIds = { aChild->GetId(DataType::ID_ENTITY) };
			else if(aChild->m_name == "count")
				m_count = aChild->GetUInt32();
			else if(aChild->m_name == "boolean")
				m_boolean = aChild->GetBool();
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
		aWriter->WriteBool(m_boolean);
	}
	
	bool		
	KillNPC::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInts(m_entityIds))
			return false;
		if (!aReader->ReadUInt(m_count))
			return false;
		if(!aReader->ReadBool(m_boolean))
			return false;

		if(m_boolean)
			SetBoolean();

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