#pragma once

#include "IReader.h"
#include "ItemType.h"
#include "IWriter.h"
#include "Parser.h"
#include "Resource.h"
#include "UIntRange.h"

namespace tpublic
{

	class NPCMetrics
	{
	public:
		struct Level
		{
			void
			FromSource(
				const SourceNode*		aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "base_weapon_damage")
					{
						m_baseWeaponDamage = UIntRange(aChild);
					}
					else if (aChild->m_name == "cash")
					{
						m_cash = UIntRange(aChild);
					}
					else if(aChild->m_name == "base_resource")
					{
						TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing resource annotation.");
						Resource::Id resourceId = Resource::StringToId(aChild->m_annotation->GetIdentifier());
						TP_VERIFY(resourceId != Resource::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid resource.", aChild->m_annotation->GetIdentifier());
						m_baseResource[resourceId] = aChild->GetUInt32();
					}
					else if (aChild->m_name == "elite_weapon_damage")
					{
						m_eliteWeaponDamage = aChild->GetFloat();
					}
					else if (aChild->m_name == "elite_cash")
					{
						m_eliteCash = aChild->GetFloat();
					}
					else if (aChild->m_name == "base_armor")
					{
						m_baseArmor = aChild->GetUInt32();
					}
					else if (aChild->m_name == "elite_resource")
					{
						TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing resource annotation.");
						Resource::Id resourceId = Resource::StringToId(aChild->m_annotation->GetIdentifier());
						TP_VERIFY(resourceId != Resource::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid resource.", aChild->m_annotation->GetIdentifier());
						m_eliteResource[resourceId] = aChild->GetFloat();
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aWriter) const
			{	
				aWriter->WriteUInt(m_baseArmor);
				m_baseWeaponDamage.ToStream(aWriter);
				m_cash.ToStream(aWriter);
				aWriter->WriteFloat(m_eliteWeaponDamage);
				aWriter->WriteFloat(m_eliteCash);
				for(uint32_t i = 1; i < (uint32_t)Resource::NUM_IDS; i++)
				{
					aWriter->WriteUInt(m_baseResource[i]);
					aWriter->WriteFloat(m_eliteResource[i]);
				}
			}

			bool
			FromStream(
				IReader*				aReader) 
			{
				if(!aReader->ReadUInt(m_baseArmor))
					return false;
				if (!m_baseWeaponDamage.FromStream(aReader))
					return false;
				if (!m_cash.FromStream(aReader))
					return false;
				if (!aReader->ReadFloat(m_eliteWeaponDamage))
					return false;
				if (!aReader->ReadFloat(m_eliteCash))
					return false;
				for (uint32_t i = 1; i < (uint32_t)Resource::NUM_IDS; i++)
				{
					if (!aReader->ReadUInt(m_baseResource[i]))
						return false;
					if (!aReader->ReadFloat(m_eliteResource[i]))
						return false;
				}
				return true;
			}

			// Public data
			UIntRange		m_baseWeaponDamage;
			UIntRange		m_cash;

			uint32_t		m_baseResource[Resource::NUM_IDS] = { 0 };
			uint32_t		m_baseArmor = 0;

			float			m_eliteWeaponDamage = 0.0f;
			float			m_eliteResource[Resource::NUM_IDS] = { 0 };
			float			m_eliteCash = 0.0f;
		};

		NPCMetrics()
		{

		}

		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "level")
				{
					TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing level annotation.");
					uint32_t level = aChild->m_annotation->GetUInt32();
					TP_VERIFY(level > 0 && level < 100, aChild->m_debugInfo, "Invalid level annotation.");
					AddLevel(level)->FromSource(aChild);
				}
				else if(aChild->m_name == "aggro_ranges")
				{
					aChild->GetUIntArray(m_aggroRanges);
				}
				else if(aChild->m_name == "aggro_range_base_level_difference")
				{
					m_aggroRangeBaseLevelDifference = aChild->GetInt32();
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
			});

			// Check that we don't have any null levels
			for(const std::unique_ptr<Level>& level : m_levels)
				TP_VERIFY(level, aSource->m_debugInfo, "Missing level definition.");

			// Also we need to have at least one aggro range
			TP_VERIFY(m_aggroRanges.size() > 0, aSource->m_debugInfo, "No aggro ranges defined.");
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			aStream->WriteObjectPointers(m_levels);
			aStream->WriteUInts(m_aggroRanges);
			aStream->WriteInt(m_aggroRangeBaseLevelDifference);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!aStream->ReadObjectPointers(m_levels))
				return false;
			if(!aStream->ReadUInts(m_aggroRanges))
				return false;
			if(!aStream->ReadInt(m_aggroRangeBaseLevelDifference))
				return false;
			return true;
		}

		Level*
		AddLevel(
			uint32_t						aLevel)
		{
			assert(aLevel > 0);
			size_t i = (size_t)aLevel - 1;

			if(i >= m_levels.size())
				m_levels.resize(i + 1);

			std::unique_ptr<Level>& t = m_levels[i];
			if(t)
				return NULL; // Already exists

			t = std::make_unique<Level>();

			return t.get();
		}

		const Level*
		GetLevel(
			uint32_t						aLevel) const
		{
			assert(aLevel > 0);
			size_t i = (size_t)aLevel - 1;
			if(i >= m_levels.size())
				return NULL;
			return m_levels[i].get();
		}

		int32_t 
		GetMaxAggroRange() const
		{
			assert(m_aggroRanges.size() > 0);
			return (int32_t)m_aggroRanges[0];
		}

		int32_t
		GetAggroRangeForLevelDifference(
			int32_t							aNPCLevel,
			int32_t							aTargetLevel) const
		{	
			assert(m_aggroRanges.size() > 0);
			int32_t i = Base::Max(0, aTargetLevel - aNPCLevel - m_aggroRangeBaseLevelDifference);
			return m_aggroRanges[Base::Min(i, (int32_t)m_aggroRanges.size() - 1)];
		}

		// Public data
		std::vector<std::unique_ptr<Level>>		m_levels;
		std::vector<uint32_t>					m_aggroRanges;
		int32_t									m_aggroRangeBaseLevelDifference = 0;
	};

}
