#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Talent
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TALENT;
			static const bool TAGGED = false;

			struct Point
			{
				Point()
				{

				}

				Point(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode*	aChild)
					{
						if (aChild->m_name == "ability")
							m_abilityId = aChild->GetId(DataType::ID_ABILITY);
						else if (aChild->m_name == "aura")
							m_auraId = aChild->GetId(DataType::ID_AURA);
						else if (aChild->m_name == "ability_modifier")
							m_abilityModifierId = aChild->GetId(DataType::ID_ABILITY_MODIFIER);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteUInt(m_abilityId);
					aStream->WriteUInt(m_auraId);
					aStream->WriteUInt(m_abilityModifierId);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadUInt(m_abilityId))
						return false;
					if (!aStream->ReadUInt(m_auraId))
						return false;
					if (!aStream->ReadUInt(m_abilityModifierId))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_abilityId = 0;
				uint32_t				m_auraId = 0;
				uint32_t				m_abilityModifierId = 0;
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
					const SourceNode*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "icon")
							m_iconSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "prerequisites")
							aChild->GetIdArray(DataType::ID_TALENT, m_prerequisites);
						else if (aChild->m_name == "talent_tree_points_required")
							m_talentTreePointsRequired = aChild->GetUInt32();
						else if (aChild->m_name == "points")
							aChild->GetObjectArray(m_points);
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
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteUInts(m_prerequisites);
				aStream->WriteUInt(m_talentTreePointsRequired);
				aStream->WriteObjects(m_points);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if(!aStream->ReadUInts(m_prerequisites))
					return false;
				if (!aStream->ReadUInt(m_talentTreePointsRequired))
					return false;
				if(!aStream->ReadObjects(m_points))
					return false;
				return true;
			}

			// Public data
			std::string				m_string;
			uint32_t				m_iconSpriteId = 0;
			std::vector<uint32_t>	m_prerequisites;
			uint32_t				m_talentTreePointsRequired = 0;
			std::vector<Point>		m_points;
		};


	}

}