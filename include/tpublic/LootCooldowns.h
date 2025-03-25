#pragma once

namespace tpublic
{

	class IReader;
	class IWriter;
	class Manifest;

	class LootCooldowns
	{
	public:
		void		ToStream(
						IWriter*		aWriter) const;
		bool		FromStream(
						IReader*		aReader);
		void		Add(
						const Manifest*	aManifest,						
						uint32_t		aLootCooldownId);
		bool		Check(
						uint32_t		aLootCooldownId,
						uint64_t		aCurrentTime,
						uint64_t*		aOutUntilServerTime) const;
		bool		Update();
		void		Reset();
	
		// Public data
		typedef std::unordered_map<uint32_t, uint64_t> Table;
		Table		m_table;
	};


}