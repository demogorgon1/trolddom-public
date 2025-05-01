#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/RouteNPC.h>

#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class RouteNPC::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const RouteNPC*					aObjective)
			: m_objective(aObjective)
		{
			(void)m_objective;
		}

		virtual ~Instance()
		{

		}

		// ObjectiveInstanceBase implementation
		void
		OnEntityObjectiveEvent(
			const EntityObjectiveEvent&		aEntityObjectiveEvent) override
		{
			if(aEntityObjectiveEvent.m_type == EntityObjectiveEvent::TYPE_ROUTE_NPC && !m_triggered)
			{
				m_triggered = true;
				OnUpdate();
			}
		}

		bool	
		GetProgress(
			Progress&						aOut) override
		{
			aOut = { (uint32_t)(m_triggered ? 1 : 0), (uint32_t)1 };
			return true;
		}

		bool	
		IsCompleted() const override
		{
			return m_triggered;
		}

		uint32_t				
		GetHash() const override
		{
			return m_triggered ? 1 : 0;
		}

		void	
		ToStream(
			IWriter*						aWriter) const override
		{
			aWriter->WriteBool(m_triggered);
		}
		
		bool	
		FromStream(
			IReader*						aReader) override
		{
			if(!aReader->ReadBool(m_triggered))
				return false;
			return true;
		}

	private:
		
		const RouteNPC*	m_objective = NULL;
		bool			m_triggered = false;
	};

	//-------------------------------------------------------------------------------------------

	RouteNPC::RouteNPC()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	RouteNPC::~RouteNPC()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	RouteNPC::FromSource(
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
	RouteNPC::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInts(m_entityIds);
	}
	
	bool		
	RouteNPC::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInts(m_entityIds))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	RouteNPC::CreateInstance() const 
	{
		return new Instance(this);
	}

	void		
	RouteNPC::GetWatchedEntities(
		std::vector<uint32_t>&				aOutEntityIds) const
	{
		aOutEntityIds = m_entityIds;
	}

}