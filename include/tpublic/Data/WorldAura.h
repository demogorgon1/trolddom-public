#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct WorldAura
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_WORLD_AURA;
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
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "duration_seconds")
							m_durationSeconds = aChild->GetUInt32();
						else if (aChild->m_name == "duration_minutes")
							m_durationSeconds = aChild->GetUInt32() * 60;
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteUInt(m_durationSeconds);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadUInt(m_durationSeconds))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_durationSeconds = 0;
		};

	}

}