#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/ManualBoolean.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class ManualBoolean::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const ManualBoolean*					/*aObjective*/)
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
			aOut = { (uint32_t)(m_completed ? 1 : 0), (uint32_t)1 };
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
			return m_completed ? 1 : 0;
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
		OnCompleteManualObjective() override
		{
			if(!m_completed)
			{
				m_completed = true;
				OnUpdate();
			}
		}

	private:
		
		bool						m_completed = false;
	};

	//-------------------------------------------------------------------------------------------

	ManualBoolean::ManualBoolean()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	ManualBoolean::~ManualBoolean()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	ManualBoolean::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	ManualBoolean::ToStream(
		IWriter*							/*aWriter*/) 
	{
	}
	
	bool		
	ManualBoolean::FromStream(
		IReader*							/*aReader*/) 
	{
		return true;
	}

	ObjectiveInstanceBase* 
	ManualBoolean::CreateInstance() const 
	{
		return new Instance(this);
	}

}