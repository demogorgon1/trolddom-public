#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class ManualBoolean
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_MANUAL_BOOLEAN;
		static const uint32_t FLAGS = FLAG_UNMANAGED | FLAG_BOOLEAN;

								ManualBoolean();
		virtual					~ManualBoolean();

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
	};

}