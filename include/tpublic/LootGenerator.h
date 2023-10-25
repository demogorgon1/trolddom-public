#pragma once

namespace tpublic
{

	namespace Components
	{
		struct Lootable;
	}

	class Manifest;

	class LootGenerator
	{
	public:
					LootGenerator(
						const Manifest*			aManifest);			
					~LootGenerator();

		void		Generate(
						std::mt19937&			aRandom,
						Components::Lootable*	aLootable) const;

	private:

		const Manifest*					m_manifest;

		struct Group
		{
			uint32_t
			GetRandom(
				std::mt19937&					aRandom) const
			{
				assert(m_itemIds.size() > 0);
				if(m_itemIds.size() == 1)
					return m_itemIds[0];

				std::uniform_int_distribution<size_t> distribution(0, m_itemIds.size() - 1);	
				return m_itemIds[distribution(aRandom)];
			}

			// Public data
			std::vector<uint32_t>		m_itemIds;
		};

		typedef std::unordered_map<uint32_t, std::unique_ptr<Group>> GroupTable;
		GroupTable						m_groups;

		Group*		_GetOrCreateGroup(
						uint32_t				aLootGroupId);
	};

}