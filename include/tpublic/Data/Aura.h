#pragma once

#include "../AuraEffectBase.h"
#include "../AuraEffectFactory.h"
#include "../CombatFunction.h"
#include "../DataBase.h"
#include "../Image.h"
#include "../Requirement.h"
#include "../StatModifiers.h"

namespace tpublic
{

	namespace Data
	{

		struct Aura
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_AURA;
			static const bool TAGGED = true;

			struct AuraEffectEntry
			{
				AuraEffectEntry()
				{

				}

				AuraEffectEntry(
					const SourceNode*		aSource)
				{
					m_auraEffectId = AuraEffect::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_auraEffectId != AuraEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid aura effect.", aSource->m_name.c_str());

					std::unique_ptr<AuraEffectBase> effect(aSource->m_sourceContext->m_auraEffectFactory->Create(m_auraEffectId));
					assert(effect);

					if (!aSource->m_children.empty())
						effect->FromSource(aSource);

					m_auraEffectBase = std::move(effect);
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_auraEffectId);
					aStream->WriteObjectPointer(m_auraEffectBase);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_auraEffectId))
						return false;

					m_auraEffectBase.reset(aStream->GetAuraEffectFactory()->Create(m_auraEffectId));
					if(!m_auraEffectBase->FromStream(aStream))
						return false;

					return true;
				}

				// Public data
				uint32_t							m_auraEffectId = 0;
				std::unique_ptr<AuraEffectBase>		m_auraEffectBase;
			};

			enum Type : uint8_t
			{	
				INVALID_TYPE, 

				TYPE_HIDDEN,
				TYPE_DEBUFF,
				TYPE_BUFF
			};

			enum Flag : uint32_t
			{
				FLAG_UNIQUE					= 0x00000001,
				FLAG_CHANNELED				= 0x00000002,
				FLAG_HIDE_DESCRIPTION		= 0x00000004,
				FLAG_CANCEL_IN_COMBAT		= 0x00000008,
				FLAG_PERSIST_IN_DEATH		= 0x00000010,
				FLAG_SILENT					= 0x00000020,
				FLAG_CHARGED				= 0x00000040,
				FLAG_MAGIC					= 0x00000080,
				FLAG_BLESSING				= 0x00000100,
				FLAG_INDEFINITE				= 0x00000200,
				FLAG_CANCEL_OUTSIDE_COMBAT	= 0x00000400,
				FLAG_CANCEL_ON_DAMAGE		= 0x00000800,
				FLAG_SINGLE_TARGET			= 0x00001000,
				FLAG_PRIVATE				= 0x00002000,
				FLAG_ITEM					= 0x00004000,
				FLAG_NO_SOURCE_NEEDED		= 0x00008000,
				FLAG_CANCEL_INDOOR			= 0x00010000,
				FLAG_NO_REFRESH				= 0x00020000,
				FLAG_POISON					= 0x00040000,
				FLAG_ALWAYS_SELF_APPLIED	= 0x00080000,
				FLAG_UNIQUE_PER_SOURCE		= 0x00100000,
				FLAG_NO_MOUNT				= 0x00200000,
				FLAG_EFFECTS_AS_CHARGES		= 0x00400000,
				FLAG_IGNORE_IMMUNITIES		= 0x00800000,
				FLAG_GLOBAL					= 0x01000000,
				FLAG_GLOBAL_PLAYER			= 0x02000000,
				FLAG_ALWAYS_SHOW_CHARGES	= 0x04000000,
				FLAG_ALWAYS_SHOW_TIMER		= 0x08000000,
			};

			static Type
			SourceToType(
				const SourceNode*		aSource)
			{
				const char* string = aSource->GetIdentifier();
				if (strcmp(string, "hidden") == 0)
					return TYPE_HIDDEN;
				if (strcmp(string, "debuff") == 0)
					return TYPE_DEBUFF;
				if (strcmp(string, "buff") == 0)
					return TYPE_BUFF;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid aura type.", string);
				return Type(0);
			}

			static uint32_t
			SourceToFlags(
				const SourceNode*		aSource)
			{
				uint32_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode*	aFlag)
				{
					const char* string = aFlag->GetIdentifier();
					if (strcmp(string, "unique") == 0)
						flags |= FLAG_UNIQUE;
					else if (strcmp(string, "channeled") == 0)
						flags |= FLAG_CHANNELED;
					else if (strcmp(string, "hide_description") == 0)
						flags |= FLAG_HIDE_DESCRIPTION;
					else if (strcmp(string, "cancel_in_combat") == 0)
						flags |= FLAG_CANCEL_IN_COMBAT;
					else if (strcmp(string, "cancel_indoor") == 0)
						flags |= FLAG_CANCEL_INDOOR;
					else if (strcmp(string, "persist_in_death") == 0)
						flags |= FLAG_PERSIST_IN_DEATH;
					else if (strcmp(string, "silent") == 0)
						flags |= FLAG_SILENT;
					else if (strcmp(string, "charged") == 0)
						flags |= FLAG_CHARGED;
					else if (strcmp(string, "magic") == 0)
						flags |= FLAG_MAGIC;
					else if (strcmp(string, "blessing") == 0)
						flags |= FLAG_BLESSING;
					else if (strcmp(string, "indefinite") == 0)
						flags |= FLAG_INDEFINITE; 
					else if (strcmp(string, "cancel_outside_combat") == 0)
						flags |= FLAG_CANCEL_OUTSIDE_COMBAT;
					else if (strcmp(string, "cancel_on_damage") == 0)
						flags |= FLAG_CANCEL_ON_DAMAGE;
					else if (strcmp(string, "single_target") == 0)
						flags |= FLAG_SINGLE_TARGET;
					else if (strcmp(string, "private") == 0)
						flags |= FLAG_PRIVATE;
					else if (strcmp(string, "item") == 0)
						flags |= FLAG_ITEM;
					else if (strcmp(string, "no_source_needed") == 0)
						flags |= FLAG_NO_SOURCE_NEEDED;
					else if (strcmp(string, "no_refresh") == 0)
						flags |= FLAG_NO_REFRESH;
					else if (strcmp(string, "poison") == 0)
						flags |= FLAG_POISON;
					else if (strcmp(string, "always_self_applied") == 0)
						flags |= FLAG_ALWAYS_SELF_APPLIED;
					else if (strcmp(string, "unique_per_source") == 0)
						flags |= FLAG_UNIQUE_PER_SOURCE;
					else if (strcmp(string, "no_mount") == 0)
						flags |= FLAG_NO_MOUNT;
					else if (strcmp(string, "effects_as_charges") == 0)
						flags |= FLAG_EFFECTS_AS_CHARGES;
					else if (strcmp(string, "ignore_immunities") == 0)
						flags |= FLAG_IGNORE_IMMUNITIES;
					else if (strcmp(string, "global") == 0)
						flags |= FLAG_GLOBAL;
					else if (strcmp(string, "global_player") == 0)
						flags |= FLAG_GLOBAL_PLAYER;
					else if (strcmp(string, "always_show_charges") == 0)
						flags |= FLAG_ALWAYS_SHOW_CHARGES;
					else if (strcmp(string, "always_show_timer") == 0)
						flags |= FLAG_ALWAYS_SHOW_TIMER;
					else
						TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid aura flag.", string);
				});
				return flags;
			}

			static int32_t
			SourceToDuration(
				const SourceNode*		aSource)
			{
				if(aSource->IsIdentifier("based_on_effects"))
					return -1;
				else
					return aSource->GetInt32();
			}

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
						if (aChild->m_name == "icon")
							m_iconSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "override_sprite")
							m_overrideSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "encounter")
							m_encounterId = aChild->GetId(DataType::ID_ENCOUNTER);
						else if (aChild->m_name == "particle_system")
							m_particleSystemId = aChild->GetId(DataType::ID_PARTICLE_SYSTEM);
						else if (aChild->m_name == "duration")
							m_duration = SourceToDuration(aChild);
						else if (aChild->m_name == "type")
							m_type = SourceToType(aChild);
						else if (aChild->m_name == "flags")
							m_flags |= SourceToFlags(aChild);
						else if (aChild->m_tag == "aura_effect")
							m_auraEffects.push_back(std::make_unique<AuraEffectEntry>(aChild));
						else if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "description")
							m_description = aChild->GetString();
						else if (aChild->m_name == "max_stack")
							m_maxStack = aChild->GetUInt32();
						else if (aChild->m_name == "stat_modifiers")
							m_statModifiers = std::make_unique<StatModifiers>(aChild);
						else if(aChild->m_name == "charges")
							m_charges = CombatFunction(aChild);
						else if (aChild->m_name == "sound")
							m_soundId = aChild->GetId(DataType::ID_SOUND);
						else if (aChild->m_name == "aura_group")
							m_auraGroupId = aChild->GetId(DataType::ID_AURA_GROUP);
						else if (aChild->m_name == "mount")
							m_mountId = aChild->GetId(DataType::ID_MOUNT);
						else if (aChild->m_name == "must_not_have_world_aura")
							m_mustNotHaveWorldAuraId = aChild->GetId(DataType::ID_WORLD_AURA);
						else if(aChild->m_tag == "cancel_requirement")
							m_cancelRequirements.push_back(Requirement(aChild));
						else if(aChild->m_name == "color_effect")
							m_colorEffect = Image::RGBA(aChild);
						else if (aChild->m_name == "color_weapon_glow")
							m_colorWeaponGlow = Image::RGBA(aChild);
						else if(aChild->m_name == "stat_conversion")
							m_statConversions.push_back(Stat::Conversion(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteInt(m_duration);
				aStream->WritePOD(m_type);
				aStream->WritePOD(m_flags);
				aStream->WriteObjectPointers(m_auraEffects);
				aStream->WriteString(m_string);
				aStream->WriteString(m_description);
				aStream->WriteOptionalObjectPointer(m_statModifiers);
				m_charges.ToStream(aStream);
				aStream->WriteUInt(m_encounterId);
				aStream->WriteUInt(m_particleSystemId);
				aStream->WriteUInt(m_soundId);
				aStream->WriteUInt(m_auraGroupId);
				aStream->WriteObjects(m_cancelRequirements);
				aStream->WriteOptionalPOD(m_colorEffect);
				aStream->WriteOptionalPOD(m_colorWeaponGlow);
				aStream->WriteUInt(m_mountId);
				aStream->WriteUInt(m_mustNotHaveWorldAuraId);
				aStream->WriteUInt(m_maxStack);
				aStream->WriteUInt(m_overrideSpriteId);
				aStream->WriteObjects(m_statConversions);
			}
			 
			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if (!aStream->ReadInt(m_duration))
					return false;
				if(!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadPOD(m_flags))
					return false;
				if(!aStream->ReadObjectPointers(m_auraEffects))
					return false;
				if(!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadString(m_description))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_statModifiers))
					return false;
				if(!m_charges.FromStream(aStream))
					return false;

				if(!aStream->IsEnd())
				{
					if (!aStream->ReadUInt(m_encounterId))
						return false;
				}

				if(!aStream->IsEnd())
				{
					if (!aStream->ReadUInt(m_particleSystemId))
						return false;
				}

				if(!aStream->IsEnd())
				{
					if(!aStream->ReadUInt(m_soundId))
						return false;
				}

				if(!aStream->IsEnd())
				{
					if(!aStream->ReadUInt(m_auraGroupId))
						return false;
				}

				if(!aStream->IsEnd())
				{
					if(!aStream->ReadObjects(m_cancelRequirements))
						return false;
				}

				if(!aStream->IsEnd())
				{
					if(!aStream->ReadOptionalPOD(m_colorEffect))
						return false;
				}

				if (!aStream->IsEnd())
				{
					if (!aStream->ReadOptionalPOD(m_colorWeaponGlow))
						return false;
				}

				if (!aStream->IsEnd())
				{
					if (!aStream->ReadUInt(m_mountId))
						return false;
				}

				if (!aStream->IsEnd())
				{
					if (!aStream->ReadUInt(m_mustNotHaveWorldAuraId))
						return false;
				}

				if (!aStream->IsEnd())
				{
					if (!aStream->ReadUInt(m_maxStack))
						return false;
				}

				if (!aStream->IsEnd())
				{
					if (!aStream->ReadUInt(m_overrideSpriteId))
						return false;
				}

				if(!aStream->IsEnd())
				{
					if(!aStream->ReadObjects(m_statConversions))
						return false;
				}

				return true;
			}

			// Helpers
			bool IsStacking() const { return m_maxStack > 1; }
			bool IsUniquePerSource() const { return m_flags & FLAG_UNIQUE_PER_SOURCE; }
			bool IsUnique() const { return m_flags & FLAG_UNIQUE; }
			bool ShouldIgnoreImmunities() const { return m_flags & FLAG_IGNORE_IMMUNITIES; }

			// Public data
			std::string										m_string;
			std::string										m_description;
			uint32_t										m_iconSpriteId = 0;
			int32_t											m_duration = 0;
			Type											m_type = TYPE_HIDDEN;
			uint32_t										m_flags = 0;
			std::vector<std::unique_ptr<AuraEffectEntry>>	m_auraEffects;
			std::unique_ptr<StatModifiers>					m_statModifiers;
			std::vector<Stat::Conversion>					m_statConversions;
			CombatFunction									m_charges;
			uint32_t										m_encounterId = 0;
			uint32_t										m_particleSystemId = 0;
			uint32_t										m_soundId = 0;
			uint32_t										m_auraGroupId = 0;
			uint32_t										m_mountId = 0;
			std::vector<Requirement>						m_cancelRequirements;			
			std::optional<Image::RGBA>						m_colorEffect;
			std::optional<Image::RGBA>						m_colorWeaponGlow;
			uint32_t										m_mustNotHaveWorldAuraId = 0;
			uint32_t										m_maxStack = 1;
			uint32_t										m_overrideSpriteId = 0;
		};

	}

}