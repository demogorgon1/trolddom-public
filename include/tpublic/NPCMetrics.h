#pragma once

#include "IReader.h"
#include "ItemType.h"
#include "IWriter.h"
#include "Parser.h"
#include "Resource.h"

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
						aChild->GetUIntRange(m_baseWeaponDamageMin, m_baseWeaponDamageMax);
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
				aWriter->WriteUInt(m_baseWeaponDamageMin);
				aWriter->WriteUInt(m_baseWeaponDamageMax);
				aWriter->WriteFloat(m_eliteWeaponDamage);
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
				if (!aReader->ReadUInt(m_baseWeaponDamageMin))
					return false;
				if (!aReader->ReadUInt(m_baseWeaponDamageMax))
					return false;
				if (!aReader->ReadFloat(m_eliteWeaponDamage))
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
			uint32_t		m_baseWeaponDamageMin = 0;
			uint32_t		m_baseWeaponDamageMax = 0;

			uint32_t		m_baseResource[Resource::NUM_IDS] = { 0 };

			float			m_eliteWeaponDamage = 0.0f;
			float			m_eliteResource[Resource::NUM_IDS] = { 0 };
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
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
			});

			// Check that we don't have any null levels
			for(const std::unique_ptr<Level>& level : m_levels)
				TP_VERIFY(level, aSource->m_debugInfo, "Missing level definition.");
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			aStream->WriteObjectPointers(m_levels);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!aStream->ReadObjectPointers(m_levels))
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
				
		// Public data
		std::vector<std::unique_ptr<Level>>		m_levels;
	};

}
