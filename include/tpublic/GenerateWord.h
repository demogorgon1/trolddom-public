#pragma once

#include <tpublic/Data/WordGenerator.h>

namespace tpublic
{
		
	void	GenerateWord(
				std::mt19937&				aRandom,
				const Data::WordGenerator*	aWordGenerator,
				std::string&				aOut);

}