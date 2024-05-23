#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/ManualCounter.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class ManualCounter::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const ManualCounter*			aObjective)
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
		OnCompleteManualObjective() override
		{
			if(m_count < m_objective->m_count)
			{
				m_count++;
				OnUpdate();
			}
		}

	private:
		
		const ManualCounter*	m_objective = NULL;
		uint32_t				m_count = 0;
	};

	//-------------------------------------------------------------------------------------------

	ManualCounter::ManualCounter()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	ManualCounter::~ManualCounter()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	ManualCounter::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "count")
				m_count = aChild->GetUInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	ManualCounter::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInt(m_count);
	}
	
	bool		
	ManualCounter::FromStream(
		IReader*							aReader) 
	{
		if(!aReader->ReadUInt(m_count))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	ManualCounter::CreateInstance() const 
	{
		return new Instance(this);
	}

}