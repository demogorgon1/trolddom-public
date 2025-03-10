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

			struct MaxValueEvent
			{
				MaxValueEvent()
				{

				}

				MaxValueEvent(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "reset")
							m_reset = aChild->GetBool();
						else if (aChild->m_name == "apply_world_aura")
							m_applyWorldAuraId = aChild->GetId(DataType::ID_WORLD_AURA);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteBool(m_reset);
					aWriter->WriteUInt(m_applyWorldAuraId);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadBool(m_reset))
						return false;
					if (!aReader->ReadUInt(m_applyWorldAuraId))
						return false;
					return true;
				}

				// Public data
				bool							m_reset = false;
				uint32_t						m_applyWorldAuraId = 0;
			};

			struct ThresholdToolTip
			{
				ThresholdToolTip()
				{

				}

				ThresholdToolTip(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing threshold annotation.");
					m_threshold = aSource->m_annotation->GetInt32();
					m_string = aSource->GetString();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteInt(m_threshold);
					aWriter->WriteString(m_string);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadInt(m_threshold))
						return false;
					if(!aReader->ReadString(m_string))
						return false;
					return true;
				}

				// Public data
				int32_t							m_threshold = 0;
				std::string						m_string;
			};

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
						else if (aChild->m_name == "tool_tip")
							m_toolTip = aChild->GetString();
						else if (aChild->m_name == "default_value")
							m_defaultValue = aChild->GetFloat();
						else if (aChild->m_name == "max_value")
							m_maxValue = aChild->GetFloat();
						else if (aChild->m_name == "periodic_reduction")
							m_periodicReduction = PeriodicReduction(aChild);
						else if (aChild->m_name == "max_value_event")
							m_maxValueEvent = MaxValueEvent(aChild);
						else if(aChild->m_name == "color")
							m_color = Image::RGBA(aChild);
						else if(aChild->m_name == "threshold_tool_tip")
							m_thresholdToolTips.push_back(ThresholdToolTip(aChild));
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
				aStream->WriteOptionalObject(m_maxValueEvent);
				aStream->WritePOD(m_color);
				aStream->WriteString(m_toolTip);
				aStream->WriteObjects(m_thresholdToolTips);
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
				if (!aStream->ReadOptionalObject(m_maxValueEvent))
					return false;
				if(!aStream->ReadPOD(m_color))
					return false;
				if (!aStream->ReadString(m_toolTip))
					return false;
				if(!aStream->ReadObjects(m_thresholdToolTips))
					return false;
				return true;
			}

			// Public data
			std::string							m_string;
			float								m_defaultValue = 0.0f;
			float								m_maxValue = 0.0f;
			std::optional<PeriodicReduction>	m_periodicReduction;
			std::optional<MaxValueEvent>		m_maxValueEvent;
			Image::RGBA							m_color;
			std::string							m_toolTip;
			std::vector<ThresholdToolTip>		m_thresholdToolTips;
		};

	}

}