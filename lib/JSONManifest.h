#pragma once

#include <tpublic/SpriteData.h>

namespace tpublic
{

	namespace Components
	{
		struct Talents;
	}

	namespace Data
	{
		struct Ability;
		struct Class;
		struct Talent;
		struct TalentTree;
	}

	class FileWriter;
	class Manifest;

	class JSONManifest
	{
	public:
				JSONManifest(
					const Manifest*			aManifest,
					const char*				aSpriteDataPath);
				~JSONManifest();

		void	Export(
					const char*				aPath);

	private:

		const Manifest*											m_manifest;
		SpriteData												m_spriteData;

		struct SpriteExport
		{
			const Data::Sprite*									m_data = NULL;
			std::string											m_pngBase64;
			size_t												m_index = 0;
		};

		std::vector<std::unique_ptr<SpriteExport>>				m_spriteExports;			// In index order
		std::unordered_map<uint32_t, SpriteExport*>				m_spriteExportTable;		// Sprite id -> export

		struct PaletteSpriteExport
		{
			char												m_symbol = '\0';
			const SpriteExport*									m_sprite = NULL;
		};

		struct ClassTalentTreeExport
		{
			const Data::TalentTree*								m_data = NULL;
			const SpriteExport*									m_icon = NULL;
			std::vector<PaletteSpriteExport>					m_paletteSprites;
		};

		// Abilities a character has at max level, with the level each one is learned at
		struct AbilityUnlock
		{
			uint32_t											m_abilityId = 0;
			uint32_t											m_level = 0;
		};

		typedef std::vector<AbilityUnlock> AbilityUnlocks;

		struct HybridExport
		{
			uint32_t											m_talentTreeId = 0;
			std::vector<std::pair<uint32_t, AbilityUnlocks>>	m_abilitiesByReplacedTalentTreeId;
		};

		struct ClassExport
		{
			const Data::Class*									m_data = NULL;
			const SpriteExport*									m_sprite = NULL;
			std::vector<std::unique_ptr<ClassTalentTreeExport>>	m_talentTrees;
			AbilityUnlocks										m_abilities;
			std::vector<HybridExport>							m_hybrids;
			std::vector<std::pair<uint32_t, const SpriteExport*>> m_abilityIcons;	// Abilities using the icon of a start equipment item
		};

		struct AbilityExport
		{
			const Data::Ability*								m_data = NULL;
			const SpriteExport*									m_icon = NULL;
		};

		std::vector<std::unique_ptr<AbilityExport>>				m_abilities;
		std::unordered_set<uint32_t>							m_abilityIds;
		std::vector<uint32_t>									m_abilityModifierIds;		// Given by talents
		std::unordered_set<uint32_t>							m_abilityModifierIdSet;

		std::vector<std::unique_ptr<ClassExport>>				m_classes;

		struct TalentExport
		{
			const Data::Talent*									m_data = NULL;
			const SpriteExport*									m_icon = NULL;
		};

		std::vector<std::unique_ptr<TalentExport>>				m_talents;
		std::unordered_set<uint32_t>							m_talentIds;

		std::vector<std::pair<std::string, const SpriteExport*>> m_uiSprites;

		void			_ExtractClasses();
		void			_ExtractUISprites();
		void			_GetAbilityUnlocks(
							const Data::Class*		aClass,
							const Components::Talents* aTalents,
							AbilityUnlocks&			aOut);
		void			_AddAbility(
							uint32_t				aAbilityId);
		void			_WriteAbilityUnlocks(
							FileWriter&				aFile,
							const AbilityUnlocks&	aAbilityUnlocks);
		void			_ExtractTalents(
							const Data::TalentTree*	aTalentTree);
		SpriteExport*	_GetSpriteExport(
							uint32_t				aSpriteId);
		SpriteExport*	_GetCharacterSpriteExport(
							const Data::Class*		aClass);
		SpriteExport*	_AddSpriteExport(
							const unsigned char*	aRGBA,
							uint32_t				aWidth,
							uint32_t				aHeight);
	};

}
