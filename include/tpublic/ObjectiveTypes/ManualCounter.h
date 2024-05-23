#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class ManualCounter
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_MANUAL_COUNTER;
		static const uint32_t FLAGS = FLAG_UNMANAGED;

								ManualCounter();
		virtual					~ManualCounter();

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

		uint32_t			m_count = 1;
	};

}