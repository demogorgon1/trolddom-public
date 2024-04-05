#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct InteractableObject
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_INTERACTABLE_OBJECT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Type : uint32_t
			{
				INVALID_TYPE,

				TYPE_MAILBOX,
				TYPE_PLAYER_WORLD_STASH
			};

			static Type
			StringToType(
				const char*				aString)
			{
				std::string_view t(aString);
				if(t == "mailbox")
					return TYPE_MAILBOX;
				else if(t == "player_world_stash")
					return TYPE_PLAYER_WORLD_STASH;
				return INVALID_TYPE;
			}
			
			enum Field
			{
				FIELD_TYPE
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TYPE, "type", offsetof(InteractableObject, m_type))->m_customReadSource = [](
					const SourceNode*	aSource,
					void*				aPointer)
				{
					*((Type*)aPointer) = StringToType(aSource->GetIdentifier());
					TP_VERIFY(*((Type*)aPointer) != INVALID_TYPE, aSource->m_debugInfo, "'%s' is not a valid interactable object type.", aSource->GetIdentifier());
				};
			}

			// Public data
			Type			m_type = INVALID_TYPE;
		};
	}

}