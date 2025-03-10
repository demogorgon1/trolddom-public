#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct ControlPointState
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CONTROL_POINT_STATE;
			static const bool TAGGED = false;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{	
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "sprite")
						m_spriteId = aChild->GetId(DataType::ID_SPRITE);
					else if (aChild->m_name == "tool_tip")
						m_toolTip = aChild->GetString();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_spriteId);
				aStream->WriteString(m_toolTip);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInt(m_spriteId))
					return false;
				if (!aStream->ReadString(m_toolTip))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_spriteId = 0;
			std::string			m_toolTip;
		};

	}

}