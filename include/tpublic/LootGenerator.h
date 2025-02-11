#pragma once

namespace tpublic
{

	namespace Components
	{
		struct Lootable;
	}

	namespace Data
	{
		struct LootTable;
	}

	class EntityInstance;
	class Manifest;

	class LootGenerator
	{
	public:
		typedef std::function<void(const tpublic::ItemInstance&)> ItemCallback;

						LootGenerator(
							const Manifest*								aManifest);			
						~LootGenerator();

		void			GenerateLootable(
							std::mt19937&								aRandom,
							const std::vector<const EntityInstance*>&	aPlayerEntityInstances,
							const EntityInstance*						aLootableEntityInstance,
							uint32_t									aLevel,
							uint32_t									aCreatureTypeId,
							bool										aIsElite,
							uint32_t									aPlayerWorldCharacterId,
							Components::Lootable*						aLootable) const;
		void			GenerateLootableItems(
							std::mt19937&								aRandom,
							const std::vector<const EntityInstance*>&	aPlayerEntityInstances,
							const EntityInstance*						aLootableEntityInstance,
							uint32_t									aLevel,
							uint32_t									aCreatureTypeId,
							const Data::LootTable*						aLootTable,
							uint32_t									aPlayerWorldCharacterId,
							Components::Lootable*						aLootable) const;
		void			GenerateItems(
							std::mt19937&								aRandom,
							const std::vector<const EntityInstance*>&	aPlayerEntityInstances,
							const EntityInstance*						aLootableEntityInstance,
							uint32_t									aLevel,
							uint32_t									aCreatureTypeId,
							const Data::LootTable*						aLootTable,
							ItemCallback								aItemCallback) const;						

		// Data access
		const Manifest*	GetManifest() const { return m_manifest; }
		
	private:

		const Manifest*					m_manifest;

		struct LevelBucket
		{
			std::vector<uint32_t>		m_itemIds;
		};

		struct Group
		{
			LevelBucket*
			GetOrCreateLevelBucket(
				uint32_t											aLevel)
			{
				LevelBucketTable::iterator i = m_levelBucketTable.find(aLevel);
				if (i != m_levelBucketTable.end())
					return i->second.get();

				LevelBucket* t = new LevelBucket();
				m_levelBucketTable[aLevel] = std::unique_ptr<LevelBucket>(t);
				return t;
			}

			const LevelBucket*
			GetLevelBucket(
				uint32_t											aLevel) const
			{
				LevelBucketTable::const_iterator i = m_levelBucketTable.find(aLevel);
				if (i == m_levelBucketTable.cend())
					return NULL;
				return i->second.get();
			}

			// Public data	
			typedef std::unordered_map<uint32_t, std::unique_ptr<LevelBucket>> LevelBucketTable;
			LevelBucketTable			m_levelBucketTable;
			LevelBucket					m_defaultLevelBucket;
		};

		typedef std::unordered_map<uint32_t, std::unique_ptr<Group>> GroupTable;
		GroupTable						m_groups;

		Group*		_GetOrCreateGroup(
						uint32_t									aLootGroupId);
	};

}