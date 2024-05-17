#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class DialogueTrigger
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_DIALOGUE_TRIGGER;
		static const uint32_t FLAGS = FLAG_UNMANAGED | FLAG_BOOLEAN;

								DialogueTrigger();
		virtual					~DialogueTrigger();

		// ObjectiveTypeBase implementation
		void					FromSource(
									const SourceNode*		aSource) override;
		void					ToStream(
									IWriter*				aWriter) override;
		bool					FromStream(
									IReader*				aReader) override;
		ObjectiveInstanceBase*	CreateInstance() const override;
		
	private:

		class Instance;

		std::vector<uint32_t>	m_entityIds;
	};

}