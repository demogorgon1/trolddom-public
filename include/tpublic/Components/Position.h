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
				POSITION_FLAG_MOVING		= 0x04,
				POSITION_FLAG_SUPER_USER	= 0x08,
				POSITION_FLAG_INVISIBLE		= 0x10,
				POSITION_FLAG_LARGE			= 0x20
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

			void
			Reset()
			{
				m_position = Vec2();
				m_positionFlags = 0;

				m_lastMoveTick = 0;
				m_detached = false;
				m_updatedOnServer = false;
			}

			//int32_t
			//DistqanceSquared(
			//	const Position*		aOther) const
			//{
			//	if(aPosition->)

			//	Vec2 p1 = m_position;
			//	if()
			//}

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
			bool	IsSuperUser() const { return m_positionFlags & POSITION_FLAG_SUPER_USER; }
			void	SetSuperUser() { m_positionFlags |= POSITION_FLAG_SUPER_USER; }
			void	ClearSuperUser() { m_positionFlags &= ~POSITION_FLAG_SUPER_USER; }
			bool	IsInvisible() const { return m_positionFlags & POSITION_FLAG_INVISIBLE; }
			void	SetInvisible() { m_positionFlags |= POSITION_FLAG_INVISIBLE; }
			void	ClearInvisible() { m_positionFlags &= ~POSITION_FLAG_INVISIBLE; }
			bool	IsLarge() const { return m_positionFlags & POSITION_FLAG_LARGE; }
			void	SetLarge() { m_positionFlags |= POSITION_FLAG_LARGE; }
			void	ClearLarge() { m_positionFlags &= ~POSITION_FLAG_LARGE; }

			// Public data
			Vec2		m_position;
			uint8_t		m_positionFlags = 0;

			// Not serialized, internal server stuff
			int32_t		m_lastMoveTick = 0;
			bool		m_detached = false;
			bool		m_updatedOnServer = false;
		};
	}

}