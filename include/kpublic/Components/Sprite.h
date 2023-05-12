#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Sprite
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SPRITE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			Sprite()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Sprite()
			{

			}

			// ComponentBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_name == "id")
						m_spriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_spriteId);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_spriteId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_spriteId = 0;
		};
	}

}