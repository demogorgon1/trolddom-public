#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct RealmBalance
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_REALM_BALANCE;
			static const bool TAGGED = false;

			struct PeriodicReduction
			{
				PeriodicReduction()
				{

				}

				PeriodicReduction(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "interval")
							m_interval = aChild->GetUInt32();
						else if(aChild->m_name == "value")
							m_value = aChild->GetFloat();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_interval);
					aWriter->WriteFloat(m_value);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_interval))
						return false;
					if(!aReader->ReadFloat(m_value))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_interval = 1;
				float							m_value = 0.0f;
			};

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
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "default_value")
							m_defaultValue = aChild->GetFloat();
						else if (aChild->m_name == "max_value")
							m_maxValue = aChild->GetFloat();
						else if(aChild->m_name == "periodic_reduction")
							m_periodicReduction = PeriodicReduction(aChild);
						else if(aChild->m_name == "color")
							m_color = Image::RGBA(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_string);
				aStream->WriteFloat(m_defaultValue);
				aStream->WriteFloat(m_maxValue);
				aStream->WriteOptionalObject(m_periodicReduction);
				aStream->WritePOD(m_color);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadFloat(m_defaultValue))
					return false;
				if (!aStream->ReadFloat(m_maxValue))
					return false;
				if (!aStream->ReadOptionalObject(m_periodicReduction))
					return false;
				if(!aStream->ReadPOD(m_color))
					return false;
				return true;
			}

			// Public data
			std::string							m_string;
			float								m_defaultValue = 0.0f;
			float								m_maxValue = 0.0f;
			std::optional<PeriodicReduction>	m_periodicReduction;
			Image::RGBA							m_color;
		};

	}

}