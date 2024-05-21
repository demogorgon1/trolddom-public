#include "../Pcheader.h"

#include <tpublic/ObjectiveTypes/Manual.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class Manual::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const Manual*					aObjective)
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
			aOut = { m_completed ? 1UL : 0UL, 1UL };
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
		
		const Manual*		m_objective = NULL;
		bool				m_completed = false;
	};

	//-------------------------------------------------------------------------------------------

	Manual::Manual()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	Manual::~Manual()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	Manual::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	Manual::ToStream(
		IWriter*							/*aWriter*/) 
	{
	}
	
	bool		
	Manual::FromStream(
		IReader*							/*aReader*/) 
	{
		return true;
	}

	ObjectiveInstanceBase* 
	Manual::CreateInstance() const 
	{
		return new Instance(this);
	}

}