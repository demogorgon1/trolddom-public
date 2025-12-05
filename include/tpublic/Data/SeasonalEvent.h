#pragma once

#include "../DataBase.h"
#include "../Requirement.h"

namespace tpublic
{

	namespace Data
	{

		struct SeasonalEvent
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_SEASONAL_EVENT;
			static const bool TAGGED = false;

			static const uint32_t
			SourceToMonth(
				const SourceNode*		aSource)
			{
				static const char* MONTH_STRINGS[] = 
				{
					"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december", NULL
				};

				for(uint32_t i = 0; MONTH_STRINGS[i] != NULL; i++)
				{
					if(aSource->IsIdentifier(MONTH_STRINGS[i]))
						return i + 1;
				}
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid month.", aSource->GetIdentifier());
				return 0;
			}

			struct Period
			{
				Period()
				{

				}

				Period(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "month")
							m_month = SourceToMonth(aChild);
						else if(aChild->m_name == "range")
							m_range = UIntRange(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_month);
					aWriter->WriteOptionalObject(m_range);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_month))
						return false;
					if(!aReader->ReadOptionalObject(m_range))
						return false;
					return true;
				}

				// Public data
				uint32_t					m_month = 0;
				std::optional<UIntRange>	m_range;
			};

			struct RandomNPCAura
			{
				RandomNPCAura()
				{

				}

				RandomNPCAura(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "aura")
							m_auraIds.push_back(aChild->GetId(DataType::ID_AURA));
						else if(aChild->m_name == "probability")
							m_probability = aChild->GetUInt32();
						else if (aChild->m_tag == "requirement")
							m_requirements.push_back(Requirement(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}
								
				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInts(m_auraIds);
					aWriter->WriteUInt(m_probability);
					aWriter->WriteObjects(m_requirements);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInts(m_auraIds))
						return false;
					if(!aReader->ReadUInt(m_probability))
						return false;
					if(!aReader->ReadObjects(m_requirements))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>		m_auraIds;
				uint32_t					m_probability = 0;
				std::vector<Requirement>	m_requirements;
			};

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
						if(aChild->m_name == "period")
							m_periods.push_back(Period(aChild));
						else if (aChild->m_name == "random_npc_aura")
							m_randomNPCAuras.push_back(RandomNPCAura(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjects(m_periods);
				aStream->WriteObjects(m_randomNPCAuras);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadObjects(m_periods))
					return false;
				if (!aStream->ReadObjects(m_randomNPCAuras))
					return false;
				return true;
			}

			// Public data
			std::vector<Period>				m_periods;
			std::vector<RandomNPCAura>		m_randomNPCAuras;
		};

	}

}