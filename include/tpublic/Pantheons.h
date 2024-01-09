#pragma once

namespace tpublic
{
	
	namespace Data
	{
		struct Deity;
		struct Pantheon;
	}

	class Manifest;

	class Pantheons
	{
	public:
		struct Entry
		{
			const Data::Pantheon*				m_pantheon = NULL;
			std::vector<const Data::Deity*>		m_deities;
		};

						Pantheons(
							const Manifest*			aManifest);
						~Pantheons();
				
		const Entry*	GetEntry(
							uint32_t				aPantheonId) const;
		
	private:

		typedef std::unordered_map<uint32_t, std::unique_ptr<Entry>> Table;
		Table									m_table;
	};

}