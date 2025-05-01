#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Sound
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_SOUND;
			static const bool TAGGED = true;

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
						if(aChild->m_name == "source")
							m_source = (aChild->m_realPath + "/") + aChild->GetString();
						else if(aChild->m_name == "streamed")
							m_streamed = aChild->GetBool();
						else if (aChild->m_name == "cooldown_ms")
							m_cooldownMS = aChild->GetUInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteBool(m_streamed);
				aWriter->WriteUInt(m_cooldownMS);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadBool(m_streamed))
					return false;
				if (!aReader->ReadUInt(m_cooldownMS))
					return false;
				return true;
			}

			// Public data			
			bool				m_streamed = false;
			uint32_t			m_cooldownMS = 80;
			std::string			m_source; // Not serialized
		};

	}

}