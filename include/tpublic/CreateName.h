#pragma once

#include "WordList.h"

namespace tpublic
{

	namespace Data
	{
		struct NameTemplate;
	}
	
	class Manifest;

	void		CreateName(
					const Manifest*					aManifest,
					const Data::NameTemplate*		aNameTemplate,
					WordList::QueryCache*			aWordListQueryCache,
					std::mt19937&					aRandom,
					std::string&					aOutName,
					std::unordered_set<uint32_t>&	aOutTags);
}