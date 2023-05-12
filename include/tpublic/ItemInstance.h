#pragma once

namespace tpublic
{

	struct ItemInstance
	{
		bool	
		IsSet() const
		{
			return m_itemId != 0;
		}

		// Public data
		uint32_t		m_itemId = 0;
		uint32_t		m_seed = 0;
	};

}