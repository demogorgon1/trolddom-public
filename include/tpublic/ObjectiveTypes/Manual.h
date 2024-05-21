#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class Manual
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_MANUAL;
		static const uint32_t FLAGS = FLAG_UNMANAGED | FLAG_BOOLEAN;

								Manual();
		virtual					~Manual();

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