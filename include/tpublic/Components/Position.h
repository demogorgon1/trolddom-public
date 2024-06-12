#pragma once

#include "../ComponentBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct Position
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_POSITION;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum PositionFlag : uint8_t
			{
				POSITION_FLAG_BLOCKING		= 0x01,
				POSITION_FLAG_TELEPORTED	= 0x02,
				POSITION_FLAG_MOVING		= 0x04
			};

			enum Field : uint32_t
			{
				FIELD_POSITION,
				FIELD_POSITION_FLAGS
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_VEC2, FIELD_POSITION, NULL, offsetof(Position, m_position));
				aSchema->DefineCustomPODNoSource<uint8_t>(FIELD_POSITION_FLAGS, offsetof(Position, m_positionFlags));
			}

			// Helpers
			bool	IsBlocking() const { return m_positionFlags & POSITION_FLAG_BLOCKING; }
			void	SetBlocking() { m_positionFlags |= POSITION_FLAG_BLOCKING; }
			void	ClearBlocking() { m_positionFlags &= ~POSITION_FLAG_BLOCKING; }
			bool	IsTeleported() const { return m_positionFlags & POSITION_FLAG_TELEPORTED; }
			void	SetTeleported() { m_positionFlags |= POSITION_FLAG_TELEPORTED; }
			void	ClearTeleported() { m_positionFlags &= ~POSITION_FLAG_TELEPORTED; }
			bool	IsMoving() const { return m_positionFlags & POSITION_FLAG_MOVING; }
			void	SetMoving() { m_positionFlags |= POSITION_FLAG_MOVING; }
			void	ClearMoving() { m_positionFlags &= ~POSITION_FLAG_MOVING; }

			// Public data
			Vec2		m_position;
			uint8_t		m_positionFlags = 0;

			// Not serialized, internal server stuff
			uint32_t	m_lastMoveTick = 0;
			bool		m_detached = false;
			bool		m_updatedOnServer = false;
		};
	}

}