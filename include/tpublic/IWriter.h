#pragma once

#include "VarSizeUInt.h"

namespace tpublic
{

	class IWriter
	{
	public:
		virtual				~IWriter() {}

		template <typename _T>
		void
		WriteUIntSet(
			const std::unordered_set<_T>&			aSet)
		{
			WriteUInt(aSet.size());
			for(_T value : aSet)
				WriteUInt(value);
		}

		template <typename _T1, typename _T2, typename _TTable = std::unordered_map<_T1, _T2>>
		void
		WriteUIntToUIntTable(
			const _TTable&							aTable)
		{
			WriteUInt(aTable.size());
			for(typename _TTable::const_iterator i = aTable.cbegin(); i != aTable.cend(); i++)
			{
				WriteUInt(i->first);
				WriteUInt(i->second);
			}
		}

		template <typename _T1, typename _T2, typename _TTable = std::unordered_map<_T1, _T2>>
		void
		WriteUIntToIntTable(
			const _TTable&							aTable)
		{
			WriteUInt(aTable.size());
			for(typename _TTable::const_iterator i = aTable.cbegin(); i != aTable.cend(); i++)
			{
				WriteUInt(i->first);
				WriteInt(i->second);
			}
		}

		template <typename _T1, typename _T2, typename _TTable = std::unordered_map<_T1, _T2>>
		void
		WriteUIntToObjectTable(
			const _TTable&							aTable)
		{
			WriteUInt(aTable.size());
			for(typename _TTable::const_iterator i = aTable.cbegin(); i != aTable.cend(); i++)
			{
				WriteUInt(i->first);
				i->second.ToStream(this);
			}
		}

		template <typename _T>
		void
		WritePOD(
			_T										aValue)
		{
			Write(&aValue, sizeof(aValue));
		}

		template <typename _T>
		void
		WriteOptionalPOD(
			const std::optional<_T>&				aValue)
		{
			WriteBool(aValue.has_value());
			if(aValue.has_value())
				Write(&aValue.value(), sizeof(_T));
		}

		void
		WriteFloat(
			float									aValue)
		{
			Write(&aValue, sizeof(aValue));
		}

		template <typename _T>
		void
		WriteInt(
			_T										aValue)
		{
			uint64_t v = aValue >= 0 ? ((uint64_t)aValue << 1) : (((uint64_t)(-aValue) << 1) | 1);
			WriteUInt(v);
		}

		template <typename _T>
		void
		WriteUInt(
			_T										aValue)
		{
			VarSizeUInt::Encoder<_T> t;
			t.Encode(aValue);
			Write(t.GetBuffer(), t.GetBufferSize());
		}

		template <typename _T>
		void
		WriteIntDelta(
			_T										aBase,
			_T										aValue)
		{
			WriteInt(aValue - aBase);
		}

		template <typename _T>
		void
		WriteUIntDelta(
			_T										aBase,
			_T										aValue)
		{
			WriteInt((int64_t)aValue - (int64_t)aBase);
		}

		template <typename _T>
		void
		WriteUInts(
			const std::vector<_T>& aValues)
		{
			WriteUInt(aValues.size());
			for (const _T& value : aValues)
				WriteUInt<_T>(value);
		}

		template <typename _T>
		void
		WriteInts(
			const std::vector<_T>& aValues)
		{
			WriteUInt(aValues.size());
			for (const _T& value : aValues)
				WriteInt<_T>(value);
		}

		template <typename _T>
		void
		WritePODs(
			const std::vector<_T>&					aValues)
		{
			WriteUInt(aValues.size());
			for(const _T& value : aValues)
				WritePOD(value);
		}

		template <typename _T>
		void
		WriteObjects(
			const std::vector<_T>&					aObjects)
		{
			WriteUInt(aObjects.size());
			for(const _T& object : aObjects)
				object.ToStream(this);
		}

		template <typename _T>
		void
		WriteObjectPointers(
			const std::vector<std::unique_ptr<_T>>&	aObjects)
		{
			WriteUInt(aObjects.size());
			for(const std::unique_ptr<_T>& object : aObjects)
			{
				assert(object);
				object->ToStream(this);
			}
		}

		template <typename _T>
		void
		WriteObjectPointersWithBase(
			const std::vector<std::unique_ptr<_T>>&	aObjects)
		{
			WriteUInt(aObjects.size());
			for(const std::unique_ptr<_T>& object : aObjects)
			{
				assert(object);
				object->ToStreamBase(this);
				object->ToStream(this);
			}
		}

		template <typename _T>
		void
		WriteOptionalObjectPointer(
			const std::unique_ptr<_T>&				aObject)
		{
			WriteBool((bool)aObject);
			if(aObject)
				aObject->ToStream(this);
		}

		template <typename _T>
		void
		WriteObjectPointer(
			const std::unique_ptr<_T>&				aObject)
		{
			assert(aObject);
			aObject->ToStream(this);
		}

		template <typename _T>
		void
		WriteOptionalObject(
			const std::optional<_T>&				aObject)
		{
			WriteBool(aObject.has_value());
			if (aObject)
				aObject.value().ToStream(this);
		}

		void
		WriteString(
			const std::string&						aValue)
		{
			size_t length = aValue.length();
			WriteUInt(length);
			Write(aValue.c_str(), length);
		}

		void
		WriteStrings(
			const std::vector<std::string>&			aStrings)
		{
			WriteUInt(aStrings.size());
			for(const std::string& t : aStrings)
				WriteString(t);
		}

		void
		WriteBool(
			bool									aValue)
		{
			Write(&aValue, sizeof(aValue));
		}

		void
		SetTick(
			int32_t									aTick)
		{
			m_tick = aTick;
		}

		int32_t
		GetTick() const
		{
			return m_tick;
		}

		void
		SetNetworkStream(
			bool									aNetworkStream)
		{
			m_networkStream = aNetworkStream;
		}

		bool
		IsNetworkStream() const
		{
			return m_networkStream;
		}
		
		// Virtual interface
		virtual void		Write(
								const void*			aBuffer,
								size_t				aBufferSize) = 0;

	private:
		
		int32_t			m_tick = 0;
		bool			m_networkStream = false;
	};

}