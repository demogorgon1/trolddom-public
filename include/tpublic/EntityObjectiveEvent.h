#pragma once

namespace tpublic
{

	class EntityObjectiveEvent
	{
	public:
		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_KILL_NPC,
			TYPE_USE_ABILITY
		};

		// Public data
		Type		m_type = INVALID_TYPE;
		uint32_t	m_characterId = 0;		
		uint32_t	m_entityId = 0;
	};

}