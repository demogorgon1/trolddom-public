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
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				/*aWriter*/) const override
			{
			}
			
			bool
			FromStream(
				IReader*				/*aReader*/) override
			{
				return true;
			}

			// Public data			
			std::string			m_source; // Not serialized
		};

	}

}