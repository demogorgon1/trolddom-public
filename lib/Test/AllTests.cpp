#include "../Pcheader.h"

namespace tpublic::Test
{

	void		TestComponents();
	void		TestCompression();
	void		TestUniformDistribution();

	void		
	AllTests()
	{
		TestComponents();
		TestCompression();
		TestUniformDistribution();
	}

}