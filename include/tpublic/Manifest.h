#pragma once

#include "AbilityMetrics.h"
#include "DataBase.h"
#include "DefaultSoundEffects.h"
#include "IReader.h"
#include "ItemMetrics.h"
#include "IWriter.h"
#include "MiscMetrics.h"
#include "NPCMetrics.h"
#include "PersistentIdTable.h"
#include "PlayerComponents.h"
#include "ProfessionMetrics.h"
#include "QuestMetrics.h"
#include "TileLayering.h"
#include "WordList.h"
#include "WorshipMetrics.h"
#include "XPMetrics.h"

namespace tpublic
{

	class Document;

	class Manifest
	{
	public:
		class IDataContainer						
		{
		public:
			IDataContainer(
				DataType::Id													aDataType,
				bool															aHasTaggedData)
				: m_dataType(aDataType)
				, m_tagged(aHasTaggedData)
			{

			}
			
			virtual 
			~IDataContainer()
			{

			}

			bool
			IsDataType(
				DataType::Id													aDataType) const
			{
				return m_dataType == aDataType;
			}

			bool
			IsTagged() const
			{
				return m_tagged;
			}

			// Virtual interface
			virtual void			Verify() const = 0;
			virtual void			PrepareRuntime(
										uint8_t									aRuntime,
										const Manifest*							aManifest) = 0;
			virtual void			ToStream(
										IWriter*								aStream) const = 0;
			virtual bool			FromStream(	
										IReader*								aStream) = 0;
			virtual DataBase*		GetBaseByName(
										PersistentIdTable*						aPersistentIdTable,
										const char*								aName) = 0;
			virtual const DataBase*	GetExistingBaseById(
										uint32_t								aId) const = 0;
			virtual const DataBase*	GetExistingBaseByName(
										const char*								aName) const = 0;
			virtual void			ForEachBase(
										std::function<bool(const DataBase*)>	aCallback) const = 0;

		private:

			DataType::Id					m_dataType;
			bool							m_tagged;
		};

		template <typename _T>
		struct DataContainer
			: public IDataContainer
		{
			DataContainer()
				: IDataContainer(_T::DATA_TYPE, _T::TAGGED)
			{

			}

			virtual
			~DataContainer()
			{

			}

			_T*
			GetByName(
				PersistentIdTable*												aPersistentIdTable,
				const char*														aName)
			{
				assert(!m_hasUnnamedEntries);

				typename std::unordered_map<std::string, _T*>::iterator it = m_nameTable.find(aName);
				if(it == m_nameTable.end())
				{
					if(aPersistentIdTable == NULL)
						return NULL;

					std::unique_ptr<_T> t = std::make_unique<_T>();
					t->m_name = aName;
					t->m_id = aPersistentIdTable->GetId(_T::DATA_TYPE, aName);

					if (t->m_id > m_maxId)
						m_maxId = t->m_id;

					m_nameTable.insert(std::pair<std::string, _T*>(t->m_name, t.get()));
					m_idTable.insert(std::pair<uint32_t, _T*>(t->m_id, t.get()));
					m_entries.push_back(std::move(t));

					return m_entries[m_entries.size() - 1].get();
				}

				return it->second;
			}

			_T*
			GetExistingByName(
				const char*														aName)
			{
				if(aName[0] == '$')
				{
					uint32_t id = (uint32_t)strtoul(aName + 1, NULL, 10);
					return TryGetById(id);
				}

				typename std::unordered_map<std::string, _T*>::iterator it = m_nameTable.find(aName);
				if(it == m_nameTable.end())
					return NULL;
				return it->second;
			}

			const _T*
			GetExistingByName(
				const char*														aName) const
			{
				if (aName[0] == '$')
				{
					uint32_t id = (uint32_t)strtoul(aName + 1, NULL, 10);
					return TryGetById(id);
				}

				typename std::unordered_map<std::string, _T*>::const_iterator it = m_nameTable.find(aName);
				if(it == m_nameTable.cend())
					return NULL;
				return it->second;
			}

			uint32_t
			GetExistingIdByName(
				const char*														aName) const
			{
				const _T* t = GetExistingByName(aName);
				TP_CHECK(t != NULL, "'%s' is not defined.", aName);
				return t->m_id;
			}

			uint32_t
			TryGetExistingIdByName(
				const char*														aName) const
			{
				const _T* t = GetExistingByName(aName);
				if(t == NULL)
					return 0;
				return t->m_id;
			}

			_T*
			CreateUnnamed()
			{
				std::unique_ptr<_T> t = std::make_unique<_T>();
				t->m_id = ++m_maxId;
				m_idTable.insert(std::pair<uint32_t, _T*>(t->m_id, t.get()));
				_T* returnValue = t.get();
				m_entries.push_back(std::move(t));

				m_hasUnnamedEntries = true;
				return returnValue;
			}

			const _T*
			TryGetById(
				uint32_t														aId) const
			{
				auto it = m_idTable.find(aId);
				if(it == m_idTable.end())
					return NULL;
				return it->second;
			}

			_T*
			TryGetById(
				uint32_t														aId) 
			{
				auto it = m_idTable.find(aId);
				if(it == m_idTable.end())
					return NULL;
				return it->second;
			}

			const _T*
			GetById(
				uint32_t														aId) const
			{
				const _T* t = TryGetById(aId);
				TP_CHECK(t != NULL, "Invalid '%s' id: %u", DataType::IdToString(_T::DATA_TYPE), aId);
				return t;
			}

			_T*
			GetById(
				uint32_t														aId) 
			{
				_T* t = TryGetById(aId);
				TP_CHECK(t != NULL, "Invalid '%s' id: %u", DataType::IdToString(_T::DATA_TYPE), aId);
				return t;
			}

			void
			ForEach(
				std::function<bool(_T*)>										aCallback)
			{
				for (std::unique_ptr<_T>& t : m_entries)
				{
					if(!aCallback(t.get()))
						break;
				}
			}

			void
			ForEach(
				std::function<bool(const _T*)>									aCallback) const
			{
				for (const std::unique_ptr<_T>& t : m_entries)
				{
					if (!aCallback(t.get()))
						break;
				}
			}

			// IDataContainer implementation
			void
			Verify() const override
			{
				std::unordered_map<uint32_t, const _T*> idTable;

				for(const std::unique_ptr<_T>& t : m_entries)
				{
					t->Verify();

					TP_VERIFY(idTable.find(t->m_id) == idTable.end(), t->m_debugInfo, "Id collision: %u", t->m_id);
					idTable[t->m_id] = t.get();
				}
			}

			void		
			PrepareRuntime(
				uint8_t															aRuntime,
				const Manifest*													aManifest) override
			{
				for (const std::unique_ptr<_T>& t : m_entries)
					t->PrepareRuntime(aRuntime, aManifest);
			}

			void
			ToStream(
				IWriter*														aStream) const override
			{
				aStream->WriteObjectPointersWithBase(m_entries);
			}

			bool
			FromStream(
				IReader*														aStream) override
			{
				if(!aStream->ReadObjectPointersWithBase(m_entries, 8192))
					return false;

				for (std::unique_ptr<_T>& t : m_entries)
				{
					TP_CHECK(t->m_defined, "'%s' not defined.", DataType::IdToString(_T::DATA_TYPE));
					TP_CHECK(m_idTable.find(t->m_id) == m_idTable.end(), "Id collision for '%s': %u", DataType::IdToString(_T::DATA_TYPE), t->m_id);
					TP_CHECK(m_nameTable.find(t->m_name) == m_nameTable.end(), "Name collision for '%s': %s", DataType::IdToString(_T::DATA_TYPE), t->m_name.c_str());

					m_idTable[t->m_id] = t.get();
					m_nameTable[t->m_name] = t.get();

					if(t->m_id > m_maxId)
						m_maxId = t->m_id;
				}

				return true;
			}

			DataBase*
			GetBaseByName(
				PersistentIdTable*												aPersistentIdTable,
				const char*														aName) override
			{
				return GetByName(aPersistentIdTable, aName);
			}

			const DataBase* 
			GetExistingBaseById(
				uint32_t														aId) const override
			{
				typename std::unordered_map<uint32_t, _T*>::const_iterator i = m_idTable.find(aId);
				if(i != m_idTable.cend())
					return i->second;
				return NULL;
			}

			const DataBase* 
			GetExistingBaseByName(
				const char*														aName) const override
			{
				typename std::unordered_map<std::string, _T*>::const_iterator i = m_nameTable.find(aName);
				if(i != m_nameTable.cend())
					return i->second;
				return NULL;
			}

			void			
			ForEachBase(
				std::function<bool(const DataBase*)>							aCallback) const override
			{
				for (const std::unique_ptr<_T>& t : m_entries)
				{
					if (!aCallback(t.get()))
						break;
				}
			}

			// Public data
			std::vector<std::unique_ptr<_T>>			m_entries;
			std::unordered_map<std::string, _T*>		m_nameTable;
			std::unordered_map<uint32_t, _T*>			m_idTable;
			uint32_t									m_maxId = 0;
			bool										m_hasUnnamedEntries = false;
		};

					Manifest();
					~Manifest();
		
		void		Verify() const;
		void		ToStream(
						IWriter*						aStream) const;
		bool		FromStream(
						IReader*						aStream);
		void		PrepareRuntime(
						uint8_t							aRuntime);
		const char*	GetNameByTypeAndId(
						DataType::Id					aDataTypeId,
						uint32_t						aId) const;

		template<typename _T>
		DataContainer<_T>*
		GetContainer()
		{
			std::unique_ptr<IDataContainer>& t = m_containers[_T::DATA_TYPE];
			assert(t);
			assert(t->IsDataType(_T::DATA_TYPE));
			return (DataContainer<_T>*)t.get();
		}

		template<typename _T>
		const DataContainer<_T>*
		GetContainer() const
		{
			const std::unique_ptr<IDataContainer>& t = m_containers[_T::DATA_TYPE];
			assert(t);
			assert(t->IsDataType(_T::DATA_TYPE));
			return (const DataContainer<_T>*)t.get();
		}

		template<typename _T>
		_T*
		GetById(
			uint32_t									aId)
		{
			return GetContainer<_T>()->GetById(aId);
		}

		template<typename _T>
		const _T*
		GetById(
			uint32_t									aId) const
		{
			return GetContainer<_T>()->GetById(aId);
		}

		template<typename _T>
		_T*
		TryGetById(
			uint32_t									aId)
		{
			return GetContainer<_T>()->TryGetById(aId);
		}

		template<typename _T>
		const _T*
		TryGetById(
			uint32_t									aId) const
		{
			return GetContainer<_T>()->TryGetById(aId);
		}

		template<typename _T>
		const _T*
		GetExistingByName(
			const char*									aName) const
		{
			return GetContainer<_T>()->GetExistingByName(aName);
		}

		template<typename _T>
		uint32_t
		GetExistingIdByName(
			const char*									aName) const
		{
			return GetContainer<_T>()->GetExistingIdByName(aName);
		}

		template<typename _T>
		uint32_t
		TryGetExistingIdByName(
			const char*									aName) const
		{
			return GetContainer<_T>()->TryGetExistingIdByName(aName);
		}

		// Global non-itemized data
		PlayerComponents								m_playerComponents;
		XPMetrics										m_xpMetrics;
		ItemMetrics										m_itemMetrics;
		NPCMetrics										m_npcMetrics;
		QuestMetrics									m_questMetrics;
		ProfessionMetrics								m_professionMetrics;
		AbilityMetrics									m_abilityMetrics;
		WorshipMetrics									m_worshipMetrics;
		MiscMetrics										m_miscMetrics;
		WordList::Data									m_wordList;
		DefaultSoundEffects								m_defaultSoundEffects;
		TileLayering									m_tileLayering;
		std::unique_ptr<Document>						m_changelog;
		uint32_t										m_baseTileBorderPatternSpriteId = 0;

		// Public data
		std::unique_ptr<IDataContainer>					m_containers[DataType::NUM_IDS];

	private:

		template <typename _T>
		void 
		RegisterDataContainer()
		{			
			m_containers[_T::DATA_TYPE].reset(new DataContainer<_T>());
		}
	};

}