#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/KillPlayer.h>

#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class KillPlayer::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const KillPlayer*				aObjective)
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
			if(aEntityObjectiveEvent.m_type == EntityObjectiveEvent::TYPE_KILL_PLAYER && 
				(m_objective->m_factionId == 0 || aEntityObjectiveEvent.m_factionId == m_objective->m_factionId) &&
				m_killed < m_objective->m_count)
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
		
		const KillPlayer*	m_objective = NULL;
		uint32_t			m_killed = 0;
	};

	//-------------------------------------------------------------------------------------------

	KillPlayer::KillPlayer()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	KillPlayer::~KillPlayer()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	KillPlayer::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "count")  
				m_count = aChild->GetUInt32();
			else if (aChild->m_name == "faction")
				m_factionId = aChild->GetId(DataType::ID_FACTION);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	KillPlayer::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInt(m_count);
		aWriter->WriteUInt(m_factionId);
	}
	
	bool		
	KillPlayer::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInt(m_count))
			return false;
		if (!aReader->ReadUInt(m_factionId))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	KillPlayer::CreateInstance() const 
	{
		return new Instance(this);
	}

	void		
	KillPlayer::GetWatchedEntities(
		std::vector<uint32_t>&				aOutEntityIds) const
	{
		aOutEntityIds = { 0 }; // Players
	}

}