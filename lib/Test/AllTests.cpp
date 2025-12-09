#include "../Pcheader.h"

namespace tpublic::Test
{

	void		TestComponents();
	void		TestCompression();
	void		TestUniformDistribution();
	void		TestThreatTable();

	void		
	AllTests()
	{
		TestComponents();
		TestCompression();
		TestUniformDistribution();
		TestThreatTable();
	}

}