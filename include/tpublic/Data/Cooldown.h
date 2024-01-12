#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Cooldown
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_COOLDOWN;
			static const bool TAGGED = true;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "duration")
							m_duration = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteInt(m_duration);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadInt(m_duration))
					return false;
				return true;
			}

			// Public data
			int32_t			m_duration = 0;
		};

	}

}