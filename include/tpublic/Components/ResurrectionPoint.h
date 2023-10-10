#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct ResurrectionPoint
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_RESURRECTION_POINT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			static void
			CreateSchema(
				ComponentSchema* /*aSchema*/)
			{
			}

			ResurrectionPoint()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~ResurrectionPoint()
			{

			}

			// ComponentBase implementation
			void
			FromSource(
				const Parser::Node*		/*aSource*/) override
			{
			}

			void
			ToStream(
				IWriter* /*aStream*/) const override
			{
			}

			bool
			FromStream(
				IReader* /*aStream*/) override
			{
				return true;
			}

			// Public data
		};
	}

}