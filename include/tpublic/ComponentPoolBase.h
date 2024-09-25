#pragma once

namespace tpublic
{

	class ComponentBase;

	class ComponentPoolBase
	{
	public:
		virtual ~ComponentPoolBase() {}

		// Virtual methods
		virtual ComponentBase*	Allocate() = 0;
		virtual void			Release(
									ComponentBase*		aComponentBase) = 0;		
		virtual void			Validate() const = 0;
	};

}