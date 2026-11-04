#include "Pcheader.h"

#include <lodepng/lodepng.h>

#include <tpublic/DirectEffects/Damage.h>
#include <tpublic/DirectEffects/Heal.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/AbilityModifier.h>
#include <tpublic/Data/Aura.h>
#include <tpublic/Data/Class.h>
#include <tpublic/Data/Cooldown.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/Talent.h>
#include <tpublic/Data/TalentTree.h>

#include <tpublic/Components/Talents.h>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/HybridClass.h>
#include <tpublic/Manifest.h>
#include <tpublic/XPMetrics.h>

#include "FileWriter.h"
#include "JSONManifest.h"

namespace tpublic
{

	namespace
	{
		struct PNGEncode
		{
			PNGEncode(
				const unsigned char*	aRGBA,
				uint32_t				aWidth,
				uint32_t				aHeight)
			{
				unsigned int result = lodepng_encode32(&m_data, &m_size, aRGBA, aWidth, aHeight);
				TP_CHECK(result == 0 && m_data != NULL && m_size > 0, "Failed to encode PNG for JSON manifest.");
			}

			~PNGEncode()
			{
				assert(m_data != NULL);
				free(m_data);
			}

			// Public data
			unsigned char*	m_data = NULL;
			size_t			m_size = 0;
		};

		// Quoted JSON string
		std::string
		_JSONString(
			const std::string&		aString)
		{
			std::string out = "\"";
			for(char c : aString)
			{
				switch(c)
				{
				case '"':	out += "\\\""; break;
				case '\\':	out += "\\\\"; break;
				case '\n':	out += "\\n"; break;
				case '\r':	out += "\\r"; break;
				case '\t':	out += "\\t"; break;

				default:
					if((unsigned char)c < 0x20)
					{
						char buffer[8];
						snprintf(buffer, sizeof(buffer), "\\u%04x", (unsigned int)c);
						out += buffer;
					}
					else
					{
						out.push_back(c);
					}
					break;
				}
			}
			out += "\"";
			return out;
		}

		const char*
		_CombatFunctionExpressionName(
			CombatFunction::Expression	aExpression)
		{
			switch(aExpression)
			{
			case CombatFunction::EXPRESSION_A:								return "a";
			case CombatFunction::EXPRESSION_A_MUL_X:						return "a_mul_x";
			case CombatFunction::EXPRESSION_A_MUL_X_PLUS_B:					return "a_mul_x_plus_b";
			case CombatFunction::EXPRESSION_A_MUL_X_PLUS_B_MUL_Y:			return "a_mul_x_plus_b_mul_y";
			case CombatFunction::EXPRESSION_X_PLUS_A:						return "x_plus_a";
			case CombatFunction::EXPRESSION_X:								return "x";
			case CombatFunction::EXPRESSION_A_MUL_X_PLUS_B_MUL_Y_PLUS_C:	return "a_mul_x_plus_b_mul_y_plus_c";
			default:														return "";
			}
		}

		const char*
		_CombatFunctionInputName(
			CombatFunction::Input		aInput)
		{
			switch(aInput)
			{
			case CombatFunction::INPUT_HEALTH_CURRENT:				return "health_current";
			case CombatFunction::INPUT_HEALTH_MAX:					return "health_max";
			case CombatFunction::INPUT_MANA_CURRENT:				return "mana_current";
			case CombatFunction::INPUT_MANA_MAX:					return "mana_max";
			case CombatFunction::INPUT_LEVEL:						return "level";
			case CombatFunction::INPUT_MANA_BASE:					return "mana_base";
			case CombatFunction::INPUT_SPELL_DAMAGE:				return "spell_damage";
			case CombatFunction::INPUT_HEALING:						return "healing";
			case CombatFunction::INPUT_WEAPON:						return "weapon";
			case CombatFunction::INPUT_WEAPON_AVERAGE:				return "weapon_average";
			case CombatFunction::INPUT_RANGED:						return "ranged";
			case CombatFunction::INPUT_RANGED_AVERAGE:				return "ranged_average";
			case CombatFunction::INPUT_WEAPON_NORMALIZED:			return "weapon_normalized";
			case CombatFunction::INPUT_WEAPON_AVERAGE_NORMALIZED:	return "weapon_average_normalized";
			case CombatFunction::INPUT_ATTACK_POWER:				return "attack_power";
			case CombatFunction::INPUT_PVP_CONTROL_POINTS:			return "pvp_control_points";
			case CombatFunction::INPUT_OWNER_SPELL_DAMAGE:			return "owner_spell_damage";
			case CombatFunction::INPUT_OWNER_HEALING:				return "owner_healing";
			case CombatFunction::INPUT_MINION_WEAPON:				return "minion_weapon";
			case CombatFunction::INPUT_BLOCK_VALUE:					return "block_value";
			case CombatFunction::INPUT_STRENGTH:					return "strength";
			default:												return "";
			}
		}

		std::string
		_FloatJSON(
			float					aValue)
		{
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%g", aValue);
			return buffer;
		}

		// A combat function with its level curves sampled at the given level, as the game does for a player of that level
		std::string
		_CombatFunctionJSON(
			const CombatFunction&	aFunction,
			uint32_t				aLevel)
		{
			float a = aFunction.m_aLevelCurve.IsSet() ? (float)aFunction.m_aLevelCurve.Sample(aLevel) : aFunction.m_a;
			float b = aFunction.m_bLevelCurve.IsSet() ? (float)aFunction.m_bLevelCurve.Sample(aLevel) : aFunction.m_b;
			float c = aFunction.m_cLevelCurve.IsSet() ? (float)aFunction.m_cLevelCurve.Sample(aLevel) : aFunction.m_c;

			std::string out = "{\"expression\":" + _JSONString(_CombatFunctionExpressionName(aFunction.m_expression));
			if(aFunction.m_x != CombatFunction::INVALID_INPUT)
				out += ",\"x\":" + _JSONString(_CombatFunctionInputName(aFunction.m_x));
			if(aFunction.m_y != CombatFunction::INVALID_INPUT)
				out += ",\"y\":" + _JSONString(_CombatFunctionInputName(aFunction.m_y));
			out += ",\"a\":" + _FloatJSON(a) + ",\"b\":" + _FloatJSON(b) + ",\"c\":" + _FloatJSON(c);
			if(aFunction.m_spread > 0.0f)
				out += ",\"spread\":" + _FloatJSON(aFunction.m_spread);
			if(aFunction.m_entity == CombatFunction::ENTITY_TARGET)
				out += ",\"entity\":\"target\"";
			out += "}";
			return out;
		}

		std::string
		_AbilityModifierNamesJSON(
			const Manifest*					aManifest,
			const std::vector<uint32_t>&	aAbilityModifierIds)
		{
			std::string out = "[";
			for(size_t i = 0; i < aAbilityModifierIds.size(); i++)
				out += (i > 0 ? "," : "") + _JSONString(aManifest->GetById<Data::AbilityModifier>(aAbilityModifierIds[i])->m_name);
			return out + "]";
		}

		// What the game's $damage$ and $heal$ add up in ability tooltips: one entry per damage or heal direct effect, with
		// the conditions the tooltip checks (ability modifiers the player must or must not have, other requirements)
		std::string
		_AbilityFormulasJSON(
			const Manifest*					aManifest,
			uint32_t						aAbilityId,
			uint32_t						aLevel)
		{
			std::string damage;
			std::string heal;

			const Data::Ability* ability = aManifest->GetById<Data::Ability>(aAbilityId);
			for(const std::unique_ptr<Data::Ability::DirectEffectEntry>& directEffect : ability->m_directEffects)
			{
				const DirectEffectBase* base = directEffect->m_directEffectBase.get();

				std::string conditions;
				if(!base->m_mustHaveAbilityModifierIds.empty())
					conditions += ",\"must_have\":" + _AbilityModifierNamesJSON(aManifest, base->m_mustHaveAbilityModifierIds);
				if(!base->m_mustNotHaveAbilityModifierIds.empty())
					conditions += ",\"must_not_have\":" + _AbilityModifierNamesJSON(aManifest, base->m_mustNotHaveAbilityModifierIds);
				if(!base->m_requirements.empty())
					conditions += ",\"conditional\":true";

				if(const DirectEffects::Damage* damageEffect = dynamic_cast<const DirectEffects::Damage*>(base))
				{
					std::string damageType = DirectEffect::DamageTypeToName(damageEffect->m_damageType);
					std::transform(damageType.begin(), damageType.end(), damageType.begin(), [](char aChar) { return (char)tolower(aChar); });

					// Sorted by name, so the output doesn't change between builds
					std::vector<std::pair<std::string, float>> multipliers;
					for(const std::pair<const uint32_t, float>& multiplier : damageEffect->m_abilityModifierMultipliers)
						multipliers.push_back({ aManifest->GetById<Data::AbilityModifier>(multiplier.first)->m_name, multiplier.second });
					std::sort(multipliers.begin(), multipliers.end());

					std::string entry = "{\"function\":" + _CombatFunctionJSON(damageEffect->m_function, aLevel) + ",\"damage_type\":" + _JSONString(damageType);
					if(damageEffect->m_spread > 0.0f)
						entry += ",\"spread\":" + _FloatJSON(damageEffect->m_spread);
					if(!multipliers.empty())
					{
						entry += ",\"multipliers\":[";
						for(size_t i = 0; i < multipliers.size(); i++)
							entry += (i > 0 ? "," : "") + std::string("{\"ability_modifier\":") + _JSONString(multipliers[i].first) + ",\"multiplier\":" + _FloatJSON(multipliers[i].second) + "}";
						entry += "]";
					}
					entry += conditions + "}";

					damage += (damage.empty() ? "" : ",") + entry;
				}
				else if(const DirectEffects::Heal* healEffect = dynamic_cast<const DirectEffects::Heal*>(base))
				{
					std::string entry = "{\"function\":" + _CombatFunctionJSON(healEffect->m_function, aLevel);
					if(healEffect->m_spread > 0.0f)
						entry += ",\"spread\":" + _FloatJSON(healEffect->m_spread);
					if(healEffect->m_maxHealthPercentage)
						entry += ",\"max_health_percentage\":true";
					entry += conditions + "}";

					heal += (heal.empty() ? "" : ",") + entry;
				}
			}

			return "{\"damage\":[" + damage + "],\"heal\":[" + heal + "]}";
		}

		// Same text as the game's talent tooltip for a rank, and whether it's passive
		void
		_TalentPointDescription(
			const Manifest*					aManifest,
			const Data::Talent::Point&		aPoint,
			std::string&					aOutDescription,
			bool&							aOutPassive)
		{
			aOutPassive = false;

			std::vector<std::string> parts;

			if(aPoint.m_auraId != 0)
			{
				parts.push_back(aManifest->GetById<Data::Aura>(aPoint.m_auraId)->m_description);
				aOutPassive = true;
			}

			if(aPoint.m_abilityId != 0)
				parts.push_back(aManifest->GetById<Data::Ability>(aPoint.m_abilityId)->m_description);

			if(aPoint.m_abilityModifierId != 0)
			{
				parts.push_back(aManifest->GetById<Data::AbilityModifier>(aPoint.m_abilityModifierId)->m_description);
				aOutPassive = true;
			}

			// A rank that gives an ability is an ability, even if it also gives an aura (e.g. Counterkick's aura only lets
			// the ability be used after a dodge or parry). The game's tooltip calls it passive.
			if(aPoint.m_abilityId != 0)
				aOutPassive = false;

			aOutDescription.clear();
			for(const std::string& part : parts)
			{
				if(part.empty())
					continue;
				if(!aOutDescription.empty())
					aOutDescription += "\n\n";
				aOutDescription += part;
			}
		}

	}

	//-------------------------------------------------------

	JSONManifest::JSONManifest(
		const Manifest*			aManifest,
		const char*				aSpriteDataPath)
		: m_manifest(aManifest)
	{
		bool ok = m_spriteData.Load(aSpriteDataPath);
		TP_CHECK(ok, "Failed to load sprite data for JSON manifest: %s", aSpriteDataPath);
	}

	JSONManifest::~JSONManifest()
	{

	}

	void
	JSONManifest::Export(
		const char*				aPath)
	{
		_ExtractClasses();
		_ExtractUISprites();

		FileWriter file(aPath);

		file.PrintF("{\r\n");

		// Talent points: none at level 1, then one for each level gained
		{
			uint32_t maxLevel = m_manifest->m_xpMetrics->m_maxLevel;
			file.PrintF("\"max_level\":%u,\r\n", maxLevel);
			file.PrintF("\"talent_points\":%u,\r\n", maxLevel > 0 ? maxLevel - 1 : 0);
		}

		// Classes
		{
			file.PrintF("\"classes\":[\r\n");

			for(size_t i = 0; i < m_classes.size(); i++)
			{
				const std::unique_ptr<ClassExport>& classExport = m_classes[i];

				file.PrintF("{\r\n");
				file.PrintF("\"name\":%s,", _JSONString(classExport->m_data->m_name).c_str());
				file.PrintF("\"string\":%s,", _JSONString(classExport->m_data->m_displayName).c_str());
				file.PrintF("\"description\":%s,", _JSONString(classExport->m_data->m_description).c_str());
				file.PrintF("\"sprite\":%zu,", classExport->m_sprite->m_index);
				file.PrintF("\r\n");

				file.PrintF("\"talent_trees\":[\r\n");
				for(size_t j = 0; j < classExport->m_talentTrees.size(); j++)
				{
					const std::unique_ptr<ClassTalentTreeExport>& talentTreeExport = classExport->m_talentTrees[j];
					const Data::TalentTree* talentTree = talentTreeExport->m_data;

					file.PrintF("{\r\n");
					file.PrintF("\"name\":%s,", _JSONString(talentTree->m_name).c_str());
					file.PrintF("\"string\":%s,", _JSONString(talentTree->m_string).c_str());
					file.PrintF("\"description\":%s,", _JSONString(talentTree->m_description).c_str());
					file.PrintF("\"icon\":%zu,\r\n", talentTreeExport->m_icon->m_index);

					// Grid of palette symbols, anything not in the palette is empty
					file.PrintF("\"map\":[");
					for(size_t k = 0; k < talentTree->m_map.size(); k++)
						file.PrintF("%s%s", _JSONString(talentTree->m_map[k]).c_str(), k + 1 < talentTree->m_map.size() ? "," : "");
					file.PrintF("],\r\n");

					// An array, as symbols are case sensitive
					file.PrintF("\"palette\":[");
					{
						bool first = true;

						if(talentTree->m_mapPalette)
						{
							for(const Data::TalentTree::MapPaletteEntry& entry : talentTree->m_mapPalette->m_entries)
							{
								if(entry.m_type != Data::TalentTree::MapPaletteEntry::TYPE_TALENT)
									continue;

								file.PrintF("%s{\"symbol\":%s,\"talent\":%s}", first ? "" : ",", _JSONString(std::string(1, entry.m_symbol)).c_str(),
									_JSONString(m_manifest->GetById<Data::Talent>(entry.m_id)->m_name).c_str());
								first = false;
							}
						}

						for(const PaletteSpriteExport& paletteSprite : talentTreeExport->m_paletteSprites)
						{
							file.PrintF("%s{\"symbol\":%s,\"sprite\":%zu}", first ? "" : ",", _JSONString(std::string(1, paletteSprite.m_symbol)).c_str(), paletteSprite.m_sprite->m_index);
							first = false;
						}
					}
					file.PrintF("]\r\n");

					file.PrintF("}%s\r\n", j + 1 < classExport->m_talentTrees.size() ? "," : "");
				}
				file.PrintF("],\r\n");

				// Talent trees of other classes that can replace one of this class' trees
				file.PrintF("\"hybrid_talent_trees\":[");
				for(size_t j = 0; j < classExport->m_data->m_hybridTalentTrees.size(); j++)
				{
					const Data::Class::HybridTalentTree& hybridTalentTree = classExport->m_data->m_hybridTalentTrees[j];
					file.PrintF("%s{\"talent_tree\":%s,\"string\":%s,", j > 0 ? "," : "",
						_JSONString(m_manifest->GetById<Data::TalentTree>(hybridTalentTree.m_talentTreeId)->m_name).c_str(),
						_JSONString(hybridTalentTree.m_string).c_str());

					// The character's abilities with this hybrid, by the class' talent tree it replaces
					file.PrintF("\"abilities\":{");
					const HybridExport& hybridExport = classExport->m_hybrids[j];
					for(size_t k = 0; k < hybridExport.m_abilitiesByReplacedTalentTreeId.size(); k++)
					{
						file.PrintF("%s%s:", k > 0 ? "," : "", _JSONString(m_manifest->GetById<Data::TalentTree>(hybridExport.m_abilitiesByReplacedTalentTreeId[k].first)->m_name).c_str());
						_WriteAbilityUnlocks(file, hybridExport.m_abilitiesByReplacedTalentTreeId[k].second);
					}
					file.PrintF("}}");
				}
				file.PrintF("],\r\n");

				// Abilities from level progression (without a hybrid)
				file.PrintF("\"abilities\":");
				_WriteAbilityUnlocks(file, classExport->m_abilities);
				file.PrintF(",\r\n");

				// Icons of abilities that show the equipped weapon, with the start equipment
				file.PrintF("\"ability_icons\":{");
				for(size_t j = 0; j < classExport->m_abilityIcons.size(); j++)
				{
					file.PrintF("%s%s:%zu", j > 0 ? "," : "", _JSONString(m_manifest->GetById<Data::Ability>(classExport->m_abilityIcons[j].first)->m_name).c_str(),
						classExport->m_abilityIcons[j].second->m_index);
				}
				file.PrintF("}\r\n");

				file.PrintF("}%s\r\n", i + 1 < m_classes.size() ? "," : "");
			}

			file.PrintF("],\r\n");
		}

		// Talents
		{
			file.PrintF("\"talents\":{\r\n");

			for(size_t i = 0; i < m_talents.size(); i++)
			{
				const std::unique_ptr<TalentExport>& talentExport = m_talents[i];
				const Data::Talent* talent = talentExport->m_data;

				file.PrintF("%s:{", _JSONString(talent->m_name).c_str());
				file.PrintF("\"string\":%s,", _JSONString(talent->m_string).c_str());
				file.PrintF("\"icon\":%zu,", talentExport->m_icon->m_index);

				// A prerequisite only needs one point
				file.PrintF("\"prerequisites\":[");
				for(size_t j = 0; j < talent->m_prerequisites.size(); j++)
					file.PrintF("%s%s", _JSONString(m_manifest->GetById<Data::Talent>(talent->m_prerequisites[j])->m_name).c_str(), j + 1 < talent->m_prerequisites.size() ? "," : "");
				file.PrintF("],");

				file.PrintF("\"talent_tree_points_required\":%u,", talent->m_talentTreePointsRequired);

				file.PrintF("\"ranks\":[");
				for(size_t j = 0; j < talent->m_points.size(); j++)
				{
					std::string description;
					bool passive;
					const Data::Talent::Point& point = talent->m_points[j];
					_TalentPointDescription(m_manifest, point, description, passive);

					// Ability descriptions can have $damage$ and $heal$, which the game calculates from the player's stats,
					// so the formulas they come from are included
					file.PrintF("{\"passive\":%s,\"description\":%s", passive ? "true" : "false", _JSONString(description).c_str());
					if(point.m_abilityModifierId != 0)
						file.PrintF(",\"ability_modifier\":%s", _JSONString(m_manifest->GetById<Data::AbilityModifier>(point.m_abilityModifierId)->m_name).c_str());
					if(point.m_abilityId != 0)
					{
						file.PrintF(",\"ability\":%s", _JSONString(m_manifest->GetById<Data::Ability>(point.m_abilityId)->m_name).c_str());
						std::string formulas = ",\"formulas\":" + _AbilityFormulasJSON(m_manifest, point.m_abilityId, m_manifest->m_xpMetrics->m_maxLevel);
						file.Write(formulas.c_str(), formulas.length());
					}
					file.PrintF("}%s", j + 1 < talent->m_points.size() ? "," : "");
				}
				file.PrintF("]");

				file.PrintF("}%s\r\n", i + 1 < m_talents.size() ? "," : "");
			}

			file.PrintF("},\r\n");
		}

		// Abilities: class abilities and abilities from talents, with what the game's ability tooltip shows
		{
			file.PrintF("\"abilities\":{\r\n");

			for(size_t i = 0; i < m_abilities.size(); i++)
			{
				const Data::Ability* ability = m_abilities[i]->m_data;

				std::string out = _JSONString(ability->m_name) + ":{\"string\":" + _JSONString(ability->m_displayName);
				out += ",\"icon\":" + (m_abilities[i]->m_icon != NULL ? std::to_string(m_abilities[i]->m_icon->m_index) : std::string("null"));
				out += ",\"description\":" + _JSONString(ability->m_description);

				// "Melee", "Range 10" or "Range 2 - 8", nothing for self only
				if(ability->m_range != 0 && !ability->TargetSelfOnly())
				{
					if(ability->IsMelee())
						out += ",\"melee\":true";
					out += ",\"range\":" + std::to_string(ability->m_range);
					if(ability->m_minRange != 0)
						out += ",\"min_range\":" + std::to_string(ability->m_minRange);
				}

				// Mana costs are a percentage of base mana
				out += ",\"costs\":[";
				bool firstCost = true;
				for(uint32_t j = 1; j < (uint32_t)Resource::NUM_IDS; j++)
				{
					if(ability->m_resourceCosts[j] == 0)
						continue;
					out += std::string(firstCost ? "" : ",") + "{\"resource\":" + _JSONString(Resource::GetInfo((Resource::Id)j)->m_capitalizedName) + ",\"value\":" + std::to_string(ability->m_resourceCosts[j]);
					if(j == (uint32_t)Resource::ID_MANA)
						out += ",\"percent_of_base_mana\":true";
					out += "}";
					firstCost = false;
				}
				out += "]";

				// In tenths of seconds. Ranged cast time is the ranged weapon's speed.
				if(ability->IsRangedCastTime())
					out += ",\"ranged_cast_time\":true";
				else
					out += ",\"cast_time\":" + std::to_string(ability->m_castTime);

				int32_t cooldown = 0;
				for(uint32_t cooldownId : ability->m_cooldowns)
				{
					const Data::Cooldown* cooldownData = m_manifest->GetById<Data::Cooldown>(cooldownId);
					if(cooldownData->m_trigger == Data::Cooldown::TRIGGER_ABILITY && !cooldownData->m_shared)
						cooldown = Base::Max(cooldown, cooldownData->m_duration);
				}
				out += ",\"cooldown\":" + std::to_string(cooldown);

				out += ",\"formulas\":" + _AbilityFormulasJSON(m_manifest, ability->m_id, m_manifest->m_xpMetrics->m_maxLevel);
				out += std::string("}") + (i + 1 < m_abilities.size() ? "," : "") + "\r\n";

				file.Write(out.c_str(), out.length());
			}

			file.PrintF("},\r\n");
		}

		// Ability modifiers given by talents: what they change on abilities (damage multipliers are on the damage effects)
		{
			file.PrintF("\"ability_modifiers\":{\r\n");

			for(size_t i = 0; i < m_abilityModifierIds.size(); i++)
			{
				const Data::AbilityModifier* abilityModifier = m_manifest->GetById<Data::AbilityModifier>(m_abilityModifierIds[i]);

				std::string out = _JSONString(abilityModifier->m_name) + ":{\"abilities\":[";
				for(size_t j = 0; j < abilityModifier->m_abilityIds.size(); j++)
					out += (j > 0 ? "," : "") + _JSONString(m_manifest->GetById<Data::Ability>(abilityModifier->m_abilityIds[j])->m_name);
				out += "]";
				if(abilityModifier->m_modifyResourceCost.has_value())
					out += ",\"modify_resource_cost\":{\"resource\":" + _JSONString(Resource::GetInfo(abilityModifier->m_modifyResourceCost->m_resourceId)->m_capitalizedName) + ",\"value\":" + std::to_string(abilityModifier->m_modifyResourceCost->m_value) + "}";
				if(abilityModifier->m_modifyRange != 0)
					out += ",\"modify_range\":" + std::to_string(abilityModifier->m_modifyRange);
				if(abilityModifier->m_modifyCastTime != 0)
					out += ",\"modify_cast_time\":" + std::to_string(abilityModifier->m_modifyCastTime);
				if(abilityModifier->m_modifyCooldown != 0)
					out += ",\"modify_cooldown\":" + std::to_string(abilityModifier->m_modifyCooldown);
				out += std::string("}") + (i + 1 < m_abilityModifierIds.size() ? "," : "") + "\r\n";

				file.Write(out.c_str(), out.length());
			}

			file.PrintF("},\r\n");
		}

		// UI sprites by name
		{
			file.PrintF("\"ui_sprites\":{");
			for(size_t i = 0; i < m_uiSprites.size(); i++)
				file.PrintF("%s:%zu%s", _JSONString(m_uiSprites[i].first).c_str(), m_uiSprites[i].second->m_index, i + 1 < m_uiSprites.size() ? "," : "");
			file.PrintF("},\r\n");
		}

		// Sprites
		{
			file.PrintF("\"sprites\":[\r\n");

			for(size_t j = 0; j < m_spriteExports.size(); j++)
			{
				const std::unique_ptr<SpriteExport>& spriteExport = m_spriteExports[j];

				// Not PrintF(), which has a fixed size buffer
				file.Write("\"", 1);
				file.Write(spriteExport->m_pngBase64.c_str(), spriteExport->m_pngBase64.length());
				file.Write("\"", 1);
				file.PrintF("%s\r\n", j + 1 < m_spriteExports.size() ? "," : "");
			}

			file.PrintF("]\r\n");
		}

		file.PrintF("}\r\n");
	}

	//------------------------------------------------------------------------

	void
	JSONManifest::_ExtractClasses()
	{
		m_manifest->GetContainer<Data::Class>()->ForEach([&](
			const Data::Class* aClass) -> bool
		{
			std::unique_ptr<ClassExport> classExport = std::make_unique<ClassExport>();
			classExport->m_data = aClass;
			classExport->m_sprite = _GetCharacterSpriteExport(aClass);

			for(uint32_t talentTreeId : aClass->m_talentTrees)
			{
				std::unique_ptr<ClassTalentTreeExport> talentTreeExport = std::make_unique<ClassTalentTreeExport>();
				talentTreeExport->m_data = m_manifest->GetById<Data::TalentTree>(talentTreeId);
				talentTreeExport->m_icon = _GetSpriteExport(talentTreeExport->m_data->m_iconSpriteId);

				if(talentTreeExport->m_data->m_mapPalette)
				{
					for(const Data::TalentTree::MapPaletteEntry& entry : talentTreeExport->m_data->m_mapPalette->m_entries)
					{
						if(entry.m_type == Data::TalentTree::MapPaletteEntry::TYPE_SPRITE)
							talentTreeExport->m_paletteSprites.push_back({ entry.m_symbol, _GetSpriteExport(entry.m_id) });
					}
				}

				_ExtractTalents(talentTreeExport->m_data);

				classExport->m_talentTrees.push_back(std::move(talentTreeExport));
			}

			// Abilities from level progression, also for each hybrid talent tree replacing each of the class' trees
			{
				Components::Talents talents;
				_GetAbilityUnlocks(aClass, &talents, classExport->m_abilities);
			}

			for(const Data::Class::HybridTalentTree& hybridTalentTree : aClass->m_hybridTalentTrees)
			{
				HybridExport hybridExport;
				hybridExport.m_talentTreeId = hybridTalentTree.m_talentTreeId;

				for(uint32_t replacedTalentTreeId : aClass->m_talentTrees)
				{
					Components::Talents talents;
					talents.m_hybridReplacedTalentTreeId = replacedTalentTreeId;
					talents.m_hybridTalentTreeId = hybridTalentTree.m_talentTreeId;

					AbilityUnlocks abilityUnlocks;
					_GetAbilityUnlocks(aClass, &talents, abilityUnlocks);
					hybridExport.m_abilitiesByReplacedTalentTreeId.push_back({ replacedTalentTreeId, abilityUnlocks });
				}

				classExport->m_hybrids.push_back(hybridExport);
			}

			// Abilities that use the icon of the equipped weapon or ranged weapon (like Attack), with the start equipment.
			// Like the game, the ability's own icon if nothing is equipped there, except a bow for ranged attacks.
			{
				std::unordered_set<uint32_t> abilityIds;
				for(const AbilityUnlock& abilityUnlock : classExport->m_abilities)
					abilityIds.insert(abilityUnlock.m_abilityId);
				for(const HybridExport& hybridExport : classExport->m_hybrids)
				{
					for(const std::pair<uint32_t, AbilityUnlocks>& t : hybridExport.m_abilitiesByReplacedTalentTreeId)
					{
						for(const AbilityUnlock& abilityUnlock : t.second)
							abilityIds.insert(abilityUnlock.m_abilityId);
					}
				}

				std::vector<uint32_t> sortedAbilityIds(abilityIds.begin(), abilityIds.end());
				std::sort(sortedAbilityIds.begin(), sortedAbilityIds.end());

				for(uint32_t abilityId : sortedAbilityIds)
				{
					const Data::Ability* ability = m_manifest->GetById<Data::Ability>(abilityId);
					uint32_t itemId = 0;
					if(ability->m_flags & Data::Ability::FLAG_USE_WEAPON_ICON)
						itemId = aClass->GetStartEquipmentItemId(EquipmentSlot::ID_MAIN_HAND);
					else if(ability->m_flags & Data::Ability::FLAG_USE_RANGED_ICON)
						itemId = aClass->GetStartEquipmentItemId(EquipmentSlot::ID_RANGED);

					if(itemId != 0)
						classExport->m_abilityIcons.push_back({ abilityId, _GetSpriteExport(m_manifest->GetById<Data::Item>(itemId)->m_iconSpriteId) });
					else if(ability->m_flags & Data::Ability::FLAG_USE_RANGED_ICON)
						classExport->m_abilityIcons.push_back({ abilityId, _GetSpriteExport(m_manifest->GetContainer<Data::Sprite>()->GetExistingIdByName("icon_bow_1")) }); // Nicer than the ability's own question mark
				}
			}

			m_classes.push_back(std::move(classExport));
			return true;
		});
	}

	void
	JSONManifest::_GetAbilityUnlocks(
		const Data::Class*		aClass,
		const Components::Talents* aTalents,
		AbilityUnlocks&			aOut)
	{
		// Same as the game server, level by level to find out where each ability is learned
		std::unordered_set<uint32_t> seen;

		for(uint32_t level = 1; level <= m_manifest->m_xpMetrics->m_maxLevel; level++)
		{
			HybridClass::LevelProgression levelProgression;
			HybridClass::GetLevelProgression(m_manifest, aClass, aTalents, level, levelProgression);

			for(uint32_t abilityId : levelProgression.m_abilityIds)
			{
				if(seen.contains(abilityId))
					continue;

				seen.insert(abilityId);
				aOut.push_back({ abilityId, level });
				_AddAbility(abilityId);
			}
		}
	}

	void
	JSONManifest::_AddAbility(
		uint32_t				aAbilityId)
	{
		if(m_abilityIds.contains(aAbilityId))
			return;
		m_abilityIds.insert(aAbilityId);

		std::unique_ptr<AbilityExport> abilityExport = std::make_unique<AbilityExport>();
		abilityExport->m_data = m_manifest->GetById<Data::Ability>(aAbilityId);
		if(abilityExport->m_data->m_iconSpriteId != 0)
			abilityExport->m_icon = _GetSpriteExport(abilityExport->m_data->m_iconSpriteId);
		m_abilities.push_back(std::move(abilityExport));
	}

	void
	JSONManifest::_WriteAbilityUnlocks(
		FileWriter&				aFile,
		const AbilityUnlocks&	aAbilityUnlocks)
	{
		aFile.PrintF("[");
		for(size_t i = 0; i < aAbilityUnlocks.size(); i++)
		{
			aFile.PrintF("%s{\"ability\":%s,\"level\":%u}", i > 0 ? "," : "",
				_JSONString(m_manifest->GetById<Data::Ability>(aAbilityUnlocks[i].m_abilityId)->m_name).c_str(), aAbilityUnlocks[i].m_level);
		}
		aFile.PrintF("]");
	}

	void
	JSONManifest::_ExtractUISprites()
	{
		// Talent cell frames, as in the game's talent window
		static const char* NAMES[] = { "talent_tree_cell", "talent_tree_cell_green", "talent_tree_cell_yellow", "talent_tree_cell_yellow_bright" };

		for(const char* name : NAMES)
			m_uiSprites.push_back({ name, _GetSpriteExport(m_manifest->GetContainer<Data::Sprite>()->GetExistingIdByName(name)) });
	}

	void
	JSONManifest::_ExtractTalents(
		const Data::TalentTree*	aTalentTree)
	{
		aTalentTree->ForEachTalent([&](
			uint32_t aTalentId)
		{
			if(m_talentIds.contains(aTalentId))
				return;
			m_talentIds.insert(aTalentId);

			std::unique_ptr<TalentExport> talentExport = std::make_unique<TalentExport>();
			talentExport->m_data = m_manifest->GetById<Data::Talent>(aTalentId);
			talentExport->m_icon = _GetSpriteExport(talentExport->m_data->m_iconSpriteId);

			for(const Data::Talent::Point& point : talentExport->m_data->m_points)
			{
				if(point.m_abilityId != 0)
					_AddAbility(point.m_abilityId);

				if(point.m_abilityModifierId != 0 && !m_abilityModifierIdSet.contains(point.m_abilityModifierId))
				{
					m_abilityModifierIdSet.insert(point.m_abilityModifierId);
					m_abilityModifierIds.push_back(point.m_abilityModifierId);
				}
			}

			m_talents.push_back(std::move(talentExport));
		});
	}

	JSONManifest::SpriteExport*
	JSONManifest::_GetSpriteExport(
		uint32_t		aSpriteId)
	{
		std::unordered_map<uint32_t, SpriteExport*>::iterator i = m_spriteExportTable.find(aSpriteId);
		if(i != m_spriteExportTable.end())
			return i->second;

		const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(aSpriteId);
		const SpriteData::Sprite& spriteData = m_spriteData.m_sheets[sprite->m_spriteSheetIndex]->m_sprites[sprite->m_index];
		SpriteExport* t = _AddSpriteExport((const unsigned char*)spriteData.m_rgba, spriteData.m_width, spriteData.m_height);
		m_spriteExportTable[aSpriteId] = t;
		return t;
	}

	JSONManifest::SpriteExport*
	JSONManifest::_GetCharacterSpriteExport(
		const Data::Class*		aClass)
	{
		// Same layers as the game's character view (PlayerView in the client): default customization wearing the
		// start equipment
		const Data::Class::CharacterCustomization& customization = aClass->m_customization;
		if(!customization.ValidateCustomization(customization.m_default))
			return _GetSpriteExport(aClass->GetDefaultSpriteId());

		struct Layer
		{
			uint32_t				m_spriteId = 0;
			Image::RGBA				m_color;
		};

		static const Image::RGBA WHITE = { 255, 255, 255, 255 };

		std::vector<Layer> layers;

		{
			ArmorStyle::Visual armorStyleVisual;
			uint32_t chestItemId = aClass->GetStartEquipmentItemId(EquipmentSlot::ID_CHEST);
			if(chestItemId != 0)
				armorStyleVisual = m_manifest->GetById<Data::Item>(chestItemId)->GetArmorStyleVisual();
			else
				armorStyleVisual.m_id = ArmorStyle::ID_NONE;

			layers.push_back({ customization.m_body[customization.m_default.m_body]->m_armorStyles[armorStyleVisual.m_id].m_spriteId, armorStyleVisual.m_teintColor });

			if(armorStyleVisual.m_decorationColor.m_a != 0)
				layers.push_back({ aClass->m_armorDecoration.m_spriteId, armorStyleVisual.m_decorationColor });
		}

		layers.push_back({ customization.m_beard[customization.m_default.m_beard]->m_spriteId, customization.m_beardColor[customization.m_default.m_beardColor].m_color });
		layers.push_back({ customization.m_hair[customization.m_default.m_hair]->m_spriteId, customization.m_hairColor[customization.m_default.m_hairColor].m_color });

		{
			uint32_t offHandItemId = aClass->GetStartEquipmentItemId(EquipmentSlot::ID_OFF_HAND);
			if(offHandItemId != 0)
			{
				const Data::Item* item = m_manifest->GetById<Data::Item>(offHandItemId);
				if(item->m_itemType == ItemType::ID_SHIELD && aClass->m_weaponSprites[item->m_itemType])
					layers.push_back({ aClass->m_weaponSprites[item->m_itemType]->m_spriteId, WHITE });
			}
		}

		{
			uint32_t mainHandItemId = aClass->GetStartEquipmentItemId(EquipmentSlot::ID_MAIN_HAND);
			if(mainHandItemId != 0)
			{
				const Data::Item* item = m_manifest->GetById<Data::Item>(mainHandItemId);
				if((item->m_flags & Data::Item::FLAG_FISHING_ROD) != 0 && aClass->m_fishingSprites)
					layers.push_back({ aClass->m_fishingSprites->m_spriteId, WHITE });
				else if(aClass->m_weaponSprites[item->m_itemType])
					layers.push_back({ aClass->m_weaponSprites[item->m_itemType]->m_spriteId, WHITE });
			}
		}

		// Blend the layers like the game does (color modulated, alpha blended). The game draws them all at the same
		// size, so any layer with another size is scaled to the body's.
		const Data::Sprite* baseSprite = m_manifest->GetById<Data::Sprite>(layers[0].m_spriteId);
		const SpriteData::Sprite& baseSpriteData = m_spriteData.m_sheets[baseSprite->m_spriteSheetIndex]->m_sprites[baseSprite->m_index];
		uint32_t width = baseSpriteData.m_width;
		uint32_t height = baseSpriteData.m_height;

		std::vector<float> image((size_t)(width * height * 4), 0.0f);

		for(const Layer& layer : layers)
		{
			if(layer.m_spriteId == 0)
				continue;

			const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(layer.m_spriteId);
			const SpriteData::Sprite& spriteData = m_spriteData.m_sheets[sprite->m_spriteSheetIndex]->m_sprites[sprite->m_index];

			for(uint32_t y = 0; y < height; y++)
			{
				for(uint32_t x = 0; x < width; x++)
				{
					uint32_t sx = (x * spriteData.m_width) / width;
					uint32_t sy = (y * spriteData.m_height) / height;
					const SpriteData::RGBA& source = spriteData.m_rgba[sx + sy * spriteData.m_width];

					float a = ((float)source.m_a / 255.0f) * ((float)layer.m_color.m_a / 255.0f);
					if(a <= 0.0f)
						continue;

					float* destination = &image[(size_t)((x + y * width) * 4)];
					destination[0] = ((float)source.m_r / 255.0f) * ((float)layer.m_color.m_r / 255.0f) * a + destination[0] * (1.0f - a);
					destination[1] = ((float)source.m_g / 255.0f) * ((float)layer.m_color.m_g / 255.0f) * a + destination[1] * (1.0f - a);
					destination[2] = ((float)source.m_b / 255.0f) * ((float)layer.m_color.m_b / 255.0f) * a + destination[2] * (1.0f - a);
					destination[3] = a + destination[3] * (1.0f - a);
				}
			}
		}

		std::vector<unsigned char> rgba(image.size());
		for(size_t i = 0; i < image.size(); i++)
			rgba[i] = (unsigned char)(std::clamp(image[i], 0.0f, 1.0f) * 255.0f + 0.5f);

		return _AddSpriteExport(rgba.data(), width, height);
	}

	JSONManifest::SpriteExport*
	JSONManifest::_AddSpriteExport(
		const unsigned char*	aRGBA,
		uint32_t				aWidth,
		uint32_t				aHeight)
	{
		std::unique_ptr<SpriteExport> t = std::make_unique<SpriteExport>();
		t->m_index = m_spriteExports.size();

		// Base64-encoded PNG
		PNGEncode pngEncode(aRGBA, aWidth, aHeight);
		Base64::Encode(pngEncode.m_data, pngEncode.m_size, t->m_pngBase64);

		m_spriteExports.push_back(std::move(t));
		return m_spriteExports.back().get();
	}


}
