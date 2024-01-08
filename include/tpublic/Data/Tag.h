#pragma once

#include "../DataBase.h"
#include "../Stat.h"

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
						{
							aChild->GetIdArray(DataType::ID_TAG, m_mutuallyExclusive);
						}
						else if (aChild->m_name == "explicit")
						{
							m_excplicit = aChild->GetBool();
						}
						else if (aChild->m_name == "transferable")
						{
							m_transferable = aChild->GetBool();
						}
						else if (aChild->m_name == "transferable")
						{
							m_transferable = aChild->GetBool();
						}
						else if (aChild->m_name == "random_associated_stat_weights")
						{
							m_randomAssociatedStatWeights = aChild->GetBool();
						}
						else if(aChild->m_name == "associated_stat_weights")
						{
							m_associatedStatWeights = std::make_unique<Stat::Collection>();
							m_associatedStatWeights->FromSource(aChild);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
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
				aWriter->WriteBool(m_transferable);
				aWriter->WriteBool(m_randomAssociatedStatWeights);
				aWriter->WriteOptionalObjectPointer(m_associatedStatWeights);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if (!FromStreamBase(aReader))
					return false;

				if(!aReader->ReadUInts(m_mutuallyExclusive))
					return false;
				if (!aReader->ReadBool(m_excplicit))
					return false;
				if (!aReader->ReadBool(m_transferable))
					return false;
				if (!aReader->ReadBool(m_randomAssociatedStatWeights))
					return false;
				if(!aReader->ReadOptionalObjectPointer(m_associatedStatWeights))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>				m_mutuallyExclusive;
			bool								m_excplicit = false;
			bool								m_transferable = true;
			std::unique_ptr<Stat::Collection>	m_associatedStatWeights;
			bool								m_randomAssociatedStatWeights = false;
		};

	}

}