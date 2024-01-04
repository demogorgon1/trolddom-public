#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Tag
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TAG;
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
						if (aChild->m_name == "mutually_exclusive")
							aChild->GetIdArray(DataType::ID_TAG, m_mutuallyExclusive);
						else if (aChild->m_name == "explicit")
							m_excplicit = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				ToStreamBase(aWriter);

				aWriter->WriteUInts(m_mutuallyExclusive);
				aWriter->WriteBool(m_excplicit);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if (!FromStreamBase(aReader))
					return false;

				if(!aReader->ReadUInts(m_mutuallyExclusive))
					return false;
				if(!aReader->ReadBool(m_excplicit))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>	m_mutuallyExclusive;
			bool					m_excplicit = false;
		};

	}

}