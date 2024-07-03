#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/ComponentManager.h>
#include <tpublic/ComponentPool.h>

namespace tpublic::Test
{

	namespace
	{

		struct TestComponent
			: ComponentBase
		{
			static const uint32_t ID = 1;

			void
			Reset()
			{
				m_dummy = 123;
			}

			// Public data
			uint32_t		m_dummy = 0;
		};

	}

	void
	TestComponents()
	{
		// Basic pool operations
		{
			ComponentPool<TestComponent> componentPool;

			std::vector<ComponentBase*> components;

			// Allocate enough to fill exactly one chunk
			for (uint8_t i = 0; i < ComponentPoolChunkBase::SIZE; i++)
			{
				assert(componentPool.GetFullChunkCount() == 0);
				assert(i == 0 || componentPool.GetPartialChunkCount() == 1);

				ComponentBase* componentBase = componentPool.Allocate();
				assert(componentBase->GetComponentPoolChunkIndex() == i);
				components.push_back(componentBase);

				assert(componentBase->Cast<TestComponent>()->m_dummy == 123);
			}

			assert(componentPool.GetFullChunkCount() == 1);
			assert(componentPool.GetPartialChunkCount() == 0);

			// Allocate another one, should end up in new chunk
			{
				ComponentBase* componentBase = componentPool.Allocate();
				assert(componentBase->GetComponentPoolChunkIndex() == 0);

				assert(componentPool.GetFullChunkCount() == 1);
				assert(componentPool.GetPartialChunkCount() == 1);

				// Free it again, should get rid of the new chunk too
				componentPool.Release(componentBase);

				assert(componentPool.GetFullChunkCount() == 1);
				assert(componentPool.GetPartialChunkCount() == 0);
			}

			// Free the first component of first chunk
			{
				componentPool.Release(components[0]);
				components[0] = NULL;

				assert(componentPool.GetFullChunkCount() == 0);
				assert(componentPool.GetPartialChunkCount() == 1);
			}

			// Allocate again, should get the slot we just freed
			{
				ComponentBase* componentBase = componentPool.Allocate();
				assert(componentBase->GetComponentPoolChunkIndex() == 0);
				components[0] = componentBase;

				assert(componentPool.GetFullChunkCount() == 1);
				assert(componentPool.GetPartialChunkCount() == 0);
			}

			// Free everything
			for (ComponentBase* componentBase : components)
				componentPool.Release(componentBase);

			assert(componentPool.GetFullChunkCount() == 0);
			assert(componentPool.GetPartialChunkCount() == 0);
		}

		// Component manager
		{
			ComponentManager componentManager;

			ComponentBase* componentBase1 = componentManager.AllocateComponent(Components::CombatPublic::ID);
			ComponentBase* componentBase2 = componentManager.AllocateComponent(Components::CombatPublic::ID);

			{
				Components::CombatPublic* combatPublic = componentBase1->Cast<Components::CombatPublic>();
				assert(combatPublic->m_level == 1);
				combatPublic->m_level = 123;
			}

			{
				Components::CombatPublic* combatPublic = componentBase2->Cast<Components::CombatPublic>();
				assert(combatPublic->m_level == 1);
			}

			componentManager.ReleaseComponent(componentBase1);

			// Allocate after releasing, should get the same component slot, but it should be reset
			ComponentBase* componentBase3 = componentManager.AllocateComponent(Components::CombatPublic::ID);

			{
				Components::CombatPublic* combatPublic = componentBase3->Cast<Components::CombatPublic>();
				assert(combatPublic->m_level == 1);
			}
		}
	}

}