#pragma once

#include "../DataBase.h"
#include "../DirectEffect.h"
#include "../Resource.h"

namespace tpublic
{

	namespace Data
	{

		struct AbilityModifier
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ABILITY_MODIFIER;
			static const bool TAGGED = false;

			struct ModifyResourceCost
			{
				ModifyResourceCost()
				{

				}

				ModifyResourceCost(
					const SourceNode*	aSource)
				{	
					m_resourceId = Resource::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_resourceId != Resource::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid resource.", aSource->m_name.c_str());
					m_value = aSource->GetInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_resourceId);
					aWriter->WriteInt(m_value);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadPOD(m_resourceId))
						return false;
					if (!aReader->ReadInt(m_value))
						return false;
					return true;
				}

				// Public data
				Resource::Id			m_resourceId = Resource::INVALID_ID;
				int32_t					m_value = 0;
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
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "description")
						{
							m_description = aChild->GetString();
						}
						else if (aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "ability")
						{
							m_abilityIds.push_back(aChild->GetId(DataType::ID_ABILITY));
						}
						else if (aChild->m_name == "abilities")
						{
							aChild->GetIdArray(DataType::ID_ABILITY, m_abilityIds);
						}
						else if (aChild->m_tag == "modify_resource_cost")
						{
							m_modifyResourceCost = ModifyResourceCost(aChild);
						}
						else if (aChild->m_name == "modify_aura_update_count")
						{
							m_modifyAuraUpdateCount = aChild->GetInt32();
						}
						else if (aChild->m_name == "modify_range")
						{
							m_modifyRange = aChild->GetInt32();
						}
						else if (aChild->m_name == "modify_cast_time")
						{
							m_modifyCastTime = aChild->GetInt32();
						}
						else if (aChild->m_name == "modify_cooldown")
						{
							m_modifyCooldown = aChild->GetInt32();
						}
						else if(aChild->m_name == "modify_damage_type")
						{
							m_modifyDamageType = DirectEffect::StringToDamageType(aChild->GetIdentifier());
							TP_VERIFY(m_modifyDamageType != DirectEffect::INVALID_DAMAGE_TYPE, aChild->m_debugInfo, "'%s' is not a valid damage type.", aChild->GetIdentifier());
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
				aWriter->WriteString(m_description);
				aWriter->WriteString(m_string);
				aWriter->WriteUInts(m_abilityIds);
				aWriter->WriteOptionalObject(m_modifyResourceCost);
				aWriter->WriteInt(m_modifyAuraUpdateCount);
				aWriter->WriteInt(m_modifyRange);
				aWriter->WriteInt(m_modifyCastTime);
				aWriter->WriteInt(m_modifyCooldown);
				aWriter->WritePOD(m_modifyDamageType);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_description))
					return false;
				if (!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadUInts(m_abilityIds))
					return false;
				if (!aReader->ReadOptionalObject(m_modifyResourceCost))
					return false;
				if (!aReader->ReadInt(m_modifyAuraUpdateCount))
					return false;
				if (!aReader->ReadInt(m_modifyRange))
					return false;
				if (!aReader->ReadInt(m_modifyCastTime))
					return false;
				if (!aReader->ReadInt(m_modifyCooldown))
					return false;
				if(!aReader->ReadPOD(m_modifyDamageType))
					return false;
				return true;
			}

			// Public data
			std::string									m_string;
			std::string									m_description;
			std::vector<uint32_t>						m_abilityIds;
			std::optional<ModifyResourceCost>			m_modifyResourceCost;
			int32_t										m_modifyRange = 0;
			int32_t										m_modifyAuraUpdateCount = 0;
			int32_t										m_modifyCastTime = 0;
			int32_t										m_modifyCooldown = 0;
			DirectEffect::DamageType					m_modifyDamageType = DirectEffect::INVALID_DAMAGE_TYPE;
		};

	}

}