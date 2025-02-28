#pragma once

#include "DataType.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{
	
	class Manifest;

	class ComponentSchema
	{
	public:		
		static constexpr uint8_t STORAGE_FORMAT_VERSION = 1;
		static const uint32_t MAX_FIELD_ID = 64;

		enum Flag : uint8_t
		{
			FLAG_NO_NETWORK	= 0x01,
			FLAG_NO_STORAGE = 0x02
		};

		enum Type : uint8_t
		{
			TYPE_NONE,
			TYPE_VEC2,
			TYPE_STRING,
			TYPE_BOOL,
			TYPE_INT32,
			TYPE_INT64,
			TYPE_UINT32,
			TYPE_UINT64,
			TYPE_FLOAT,
			TYPE_UINT32_ARRAY,
			TYPE_CUSTOM
		};

		enum ReadType
		{
			READ_TYPE_NETWORK,
			READ_TYPE_STORAGE,
			READ_TYPE_SOURCE
		};

		typedef std::function<void(const char*)> ExportTextCallback;
		typedef std::function<void(IWriter*, const void*)> CustomWriteCallback;
		typedef std::function<bool(IReader*, void*)> CustomReadCallback;
		typedef std::function<void(const SourceNode*, void*)> CustomReadSourceCallback;
		typedef std::function<void(void*)> CustomNewCallback;
		typedef std::function<void(void*)> CustomDeleteCallback;
		typedef std::function<void(void*, const void*)> InitFromDeprecatedCallback;
		typedef std::function<void(void*, ReadType, const Manifest*)> OnReadCallback;
		typedef std::function<void(void*, const SourceNode*)> SourceModifierCallback;
		
		struct Field
		{
			Field*
			SetDataType(
				DataType::Id			aDataType)
			{
				m_dataType = aDataType;
				return this;
			}

			Field*
			SetFlags(
				uint8_t					aFlags)
			{
				m_flags = aFlags;
				return this;
			}

			// Public data
			Type										m_type = TYPE_NONE;
			uint32_t									m_id = 0;
			const char*									m_name = NULL;
			uint32_t									m_offset = 0;
			uint8_t										m_flags = 0;

			DataType::Id								m_dataType = DataType::INVALID_ID;

			uint32_t									m_initFromDeprecatedId = UINT32_MAX;
			InitFromDeprecatedCallback					m_initFromDeprecatedCallback;
			std::vector<uint32_t>						m_upgradeChain;

			CustomReadCallback							m_customRead;
			CustomWriteCallback							m_customWrite;
			CustomReadSourceCallback					m_customReadSource;
			CustomNewCallback							m_customNewCallback;
			CustomDeleteCallback						m_customDeleteCallback;
			uint32_t									m_customSize = 0;			
		};

		Field*			Define(				
							Type					aType,
							uint32_t				aId,
							const char*				aName,
							uint32_t				aOffset);
		void			InitUpgradeChains();
		void			ReadSource(
							const SourceNode*		aSource,
							void*					aObject) const;
		void			WriteNetwork(
							IWriter*				aWriter,
							const void*				aObject) const;
		bool			ReadNetwork(
							IReader*				aReader,
							void*					aObject) const;						
		void			WriteStorage(
							IWriter*				aWriter,
							const void*				aObject) const;
		bool			ReadStorage(
							const Manifest*			aManifest,
							IReader*				aReader,
							void*					aObject) const;			
		void			ExportText(
							const void*				aObject,
							ExportTextCallback		aExportTextCallback) const;
		void			ImportText(
							const char*				aText,
							void*					aObject) const;
		std::string		AsDebugString(
							const void*				aObject) const;
		void			Validate() const;
		
		template <typename _FromT, typename _ToT, typename _UpgradeCallback>
		void
		Upgrade(
			uint32_t							aFromId,
			uint32_t							aToId,
			_UpgradeCallback					aUpgradeCallback)
		{
			Field* from = _GetFieldById(aFromId);
			Field* to = _GetFieldById(aToId);

			TP_UNUSED(from);

			assert(from != NULL && to != NULL);
			assert(from->m_offset == UINT32_MAX); // Must be deprecated

			to->m_initFromDeprecatedId = aFromId;
			to->m_initFromDeprecatedCallback = [&](
				void*		aToData,
				const void* aFromData)
			{
				aUpgradeCallback((const _FromT*)aFromData, (_ToT*)aToData);
			};
		}

		template <typename _T, typename _OnReadCallback>
		void
		OnRead(
			_OnReadCallback						aOnReadCallback)
		{
			m_onReadCallback = [&](
				void*			aData,
				ReadType		aReadType,
				const Manifest* aManifest)
			{
				aOnReadCallback((_T*)aData, aReadType, aManifest);
			};
		}

		template <typename _T, typename _SourceModifierCallback>
		void
		AddSourceModifier(
			const char*							aName,
			_SourceModifierCallback				aSourceModifierCallback)
		{
			assert(!m_sourceModifierCallbacks.contains(aName));
			m_sourceModifierCallbacks[aName] = [&](
				void*				aData,
				const SourceNode*	aSource)
			{
				aSourceModifierCallback((_T*)aData, aSource);
			};
		}

		template <typename _T>
		Field*
		DefineCustom(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = Define(ComponentSchema::TYPE_CUSTOM, aId, aName, aOffset);
			t->m_customSize = sizeof(_T);
			t->m_customNewCallback = [](
				void* aObject)
			{
				new (aObject) _T();
			};
			t->m_customDeleteCallback = [](
				void* aObject)
			{
				((_T*)aObject)->~_T();
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObjectPointersNoSource(
			uint32_t							aId,
			uint32_t							aOffset)
		{
			Field* t = DefineCustom<std::vector<std::unique_ptr<_T>>>(aId, NULL, aOffset);
			t->m_customRead = [](
				IReader*	aReader,
				void*		aObject) -> bool
			{
				std::vector<std::unique_ptr<_T>>* p = (std::vector<std::unique_ptr<_T>>*)aObject;
				return aReader->ReadObjectPointers(*p);
			};
			t->m_customWrite = [](
				IWriter*	aWriter,
				const void* aObject)
			{
				const std::vector<std::unique_ptr<_T>>* p = (const std::vector<std::unique_ptr<_T>>*)aObject;
				aWriter->WriteObjectPointers(*p);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObjectPointers(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = DefineCustomObjectPointersNoSource<_T>(aId, aOffset);
			t->m_name = aName;
			t->m_customReadSource = [](
				const SourceNode*	aSource,
				void*				aObject)
			{
				std::vector<std::unique_ptr<_T>>* p = (std::vector<std::unique_ptr<_T>>*)aObject;
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					std::unique_ptr<_T> entry = std::make_unique<_T>();
					entry->FromSource(aChild);
					p->push_back(std::move(entry));
				});
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObjectPointersSingleAppend(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = DefineCustomObjectPointersNoSource<_T>(aId, aOffset);
			t->m_name = aName;
			t->m_customReadSource = [](
				const SourceNode*	aSource,
				void*				aObject)
			{
				std::vector<std::unique_ptr<_T>>* p = (std::vector<std::unique_ptr<_T>>*)aObject;
				std::unique_ptr<_T> entry = std::make_unique<_T>();
				entry->FromSource(aSource);
				p->push_back(std::move(entry));
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObjectsNoSource(
			uint32_t							aId,
			uint32_t							aOffset)
		{
			Field* t = DefineCustom<std::vector<_T>>(aId, NULL, aOffset);
			t->m_customRead = [](
				IReader*	aReader,
				void*		aObject) -> bool
			{
				std::vector<_T>* p = (std::vector<_T>*)aObject;
				return aReader->ReadObjects(*p);
			};
			t->m_customWrite = [](
				IWriter*	aWriter,
				const void* aObject)
			{
				const std::vector<_T>* p = (const std::vector<_T>*)aObject;
				aWriter->WriteObjects(*p);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObjects(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = DefineCustomObjectsNoSource<_T>(aId, aOffset);
			t->m_name = aName;
			t->m_customReadSource = [](
				const SourceNode*	aSource,
				void*				aObject)
			{
				std::vector<_T>* p = (std::vector<_T>*)aObject;
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					_T entry;
					entry.FromSource(aChild);
					p->push_back(entry);
				});
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomOptionalObjectNoSource(
			uint32_t							aId,
			uint32_t							aOffset)
		{
			Field* t = DefineCustom<std::optional<_T>>(aId, NULL, aOffset);
			t->m_customRead = [](
				IReader*	aReader,
				void*		aObject) -> bool
			{
				std::optional<_T>* p = (std::optional<_T>*)aObject;
				return aReader->ReadOptionalObject(*p);
			};
			t->m_customWrite = [](
				IWriter*	aWriter,
				const void* aObject)
			{
				const std::optional<_T>* p = (const std::optional<_T>*)aObject;
				aWriter->WriteOptionalObject(*p);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomOptionalObject(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = DefineCustomOptionalObjectNoSource<_T>(aId, aOffset);
			t->m_name = aName;
			t->m_customReadSource = [](
				const SourceNode*	aSource,
				void*				aObject)
			{
				std::optional<_T>* p = (std::optional<_T>*)aObject;
				_T entry;
				entry.FromSource(aSource);
				*p = entry;
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObjectNoSource(
			uint32_t							aId,
			uint32_t							aOffset)
		{
			Field* t = DefineCustom<_T>(aId, NULL, aOffset);
			t->m_customRead = [](
				IReader*	aReader,
				void*		aObject) -> bool
			{
				_T* p = (_T*)aObject;
				return p->FromStream(aReader);
			};
			t->m_customWrite = [](
				IWriter*	aWriter,
				const void* aObject)
			{
				const _T* p = (const _T*)aObject;
				p->ToStream(aWriter);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomObject(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = DefineCustomObjectNoSource<_T>(aId, aOffset);
			t->m_name = aName;
			t->m_customReadSource = [](
				const SourceNode*	aSource,
				void*				aObject)
			{
				_T* p = (_T*)aObject;
				p->FromSource(aSource);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomPODNoSource(
			uint32_t							aId,
			uint32_t							aOffset)
		{
			Field* t = DefineCustom<_T>(aId, NULL, aOffset);
			t->m_customRead = [](
				IReader*	aReader,
				void*		aObject) -> bool
			{
				_T* p = (_T*)aObject;
				return aReader->ReadPOD<_T>(*p);
			};
			t->m_customWrite = [](
				IWriter*	aWriter,
				const void* aObject)
			{
				const _T* p = (const _T*)aObject;
				aWriter->WritePOD<_T>(*p);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomPOD(
			uint32_t							aId,
			const char*							aName,
			uint32_t							aOffset)
		{
			Field* t = DefineCustomPODNoSource<_T>(aId, aOffset);
			t->m_name = aName;
			t->m_customReadSource = [](
				const SourceNode*	aSource,
				void*				aObject)
			{
				_T* p = (_T*)aObject;
				p->FromSource(aSource);
			};
			return t;
		}

		template <typename _T>
		Field*
		DefineCustomOptionalPODNoSource(
			uint32_t							aId,
			uint32_t							aOffset)
		{
			Field* t = DefineCustom<std::optional<_T>>(aId, NULL, aOffset);
			t->m_customRead = [](
				IReader*	aReader,
				void*		aObject) -> bool
			{
				std::optional<_T>* p = (std::optional<_T>*)aObject;
				return aReader->ReadOptionalPOD(*p);
			};
			t->m_customWrite = [](
				IWriter*	aWriter,
				const void* aObject)
			{
				const std::optional<_T>* p = (const std::optional<_T>*)aObject;
				aWriter->WriteOptionalPOD(*p);
			};
			return t;
		}

	private:

		std::vector<Field>										m_fields;
		OnReadCallback											m_onReadCallback;
		std::unordered_map<std::string, SourceModifierCallback>	m_sourceModifierCallbacks;

		uint32_t		_GetFieldSize(
							const Field*			aField) const;
		const Field*	_GetFieldByName(
							const char*				aName) const;
		const Field*	_GetFieldById(
							uint32_t				aId) const;
		Field*			_GetFieldById(
							uint32_t				aId);
		void			_WriteValue(
							IWriter*				aWriter,
							const void*				aObject,
							const Field*			aField) const;
		bool			_ReadValue(
							IReader*				aReader,
							void*					aObject,
							const Field*			aField) const;
		std::string		_ValueAsString(
							const void*				aObject,
							const Field*			aField) const;
	};

}