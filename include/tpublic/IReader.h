#pragma once

namespace tpublic
{

	class AuraEffectFactory;
	class ComponentManager;
	class DirectEffectFactory;
	class MapGeneratorFactory;
	class ObjectiveTypeFactory;

	class IReader
	{
	public:
		virtual				~IReader() {}

		template <typename _T>
		bool	
		ReadPOD(
			_T&										aOut)
		{
			return Read(&aOut, sizeof(aOut)) == sizeof(aOut);
		}

		bool
		ReadFloat(
			float&									aOut)
		{
			// FIXME: clever encoding
			return Read(&aOut, sizeof(float)) == sizeof(float);
		}

		template <typename _T>
		bool	
		ReadInt(
			_T&										aOut)
		{
			// FIXME: varsize
			return Read(&aOut, sizeof(aOut)) == sizeof(aOut);
		}

		template <typename _T>
		bool	
		ReadUInt(
			_T&										aOut)
		{
			// FIXME: varsize
			return Read(&aOut, sizeof(aOut)) == sizeof(aOut);
		}

		template <typename _T>
		bool
		ReadIntDelta(
			_T										aBase,
			_T&										aOut)
		{
			int32_t delta;
			if(!ReadInt(delta))
				return false;
			aOut = aBase + delta;
			return true;
		}

		template <typename _T>
		bool
		ReadUIntDelta(
			_T										aBase,
			_T&										aOut)
		{
			int64_t delta;
			if(!ReadInt(delta))
				return false;
			int64_t value = (int64_t)aBase + delta;
			if(value > 0)
				aOut = (_T)value;
			else
				aOut = 0;
			return true;
		}

		template <typename _T>
		bool
		ReadUInts(
			std::vector<_T>&						aOut,
			size_t									aMaxCount = 1024)
		{
			size_t count;
			if(!ReadUInt(count))
				return false;

			if(count > aMaxCount)
				return false;

			aOut.resize(count);
			for(_T& value : aOut)
			{
				if(!ReadUInt(value))
					return false;
			}

			return true;
		}

		template <typename _T>
		bool
		ReadPODs(
			std::vector<_T>&						aOut,
			size_t									aMaxCount = 1024)
		{
			size_t count;
			if(!ReadUInt(count))
				return false;

			if(count > aMaxCount)
				return false;

			aOut.resize(count);
			for(_T& value : aOut)
			{
				if(!ReadPOD(value))
					return false;
			}

			return true;
		}

		template <typename _T>
		bool
		ReadObjects(
			std::vector<_T>&						aOut,
			size_t									aMaxCount = 1024)
		{
			size_t count;
			if(!ReadUInt(count))
				return false;

			if(count > aMaxCount)
				return false;

			aOut.resize(count);
			for(_T& object : aOut)
			{
				if(!object.FromStream(this))
					return false;
			}

			return true;
		}

		template <typename _T>
		bool
		ReadObjectPointers(
			std::vector<std::unique_ptr<_T>>&		aOut,
			size_t									aMaxCount = 1024)
		{
			size_t count;
			if(!ReadUInt(count))
				return false;

			if(count > aMaxCount)
				return false;

			aOut.resize(count);
			for(std::unique_ptr<_T>& object : aOut)
			{
				object = std::make_unique<_T>();
				if(!object->FromStream(this))
					return false;
			}

			return true;
		}

		template <typename _T>
		bool
		ReadOptionalObjectPointer(
			std::unique_ptr<_T>&					aOut)
		{
			bool isSet;
			if(!ReadBool(isSet))
				return false;

			if(isSet)
			{
				aOut = std::make_unique<_T>();
				if(!aOut->FromStream(this))
					return false;
			}
			else
			{
				aOut.reset();
			}

			return true;
		}

		template <typename _T>
		bool
		ReadObjectPointer(
			std::unique_ptr<_T>&					aOut)
		{
			aOut = std::make_unique<_T>();
			if(!aOut->FromStream(this))
				return false;

			return true;
		}

		template <typename _T>
		bool
		ReadOptionalObject(
			std::optional<_T>&						aOut)
		{
			bool isSet;
			if (!ReadBool(isSet))
				return false;

			if(isSet)
			{
				_T t;
				if(!t.FromStream(this))
					return false;

				aOut = t;
			}
			else
			{
				aOut.reset();
			}

			return true;
		}

		bool
		ReadString(
			std::string&							aOut,
			size_t									aMaxLength = 256)
		{
			size_t length;
			if(!ReadUInt(length))
				return false;

			if(length > aMaxLength)
				return false;

			aOut.resize(length);
			
			if(Read(&aOut[0], length) != length)
				return false;

			return true;
		}

		bool
		ReadStrings(
			std::vector<std::string>&				aOut,
			size_t									aMaxArrayLength = 256,
			size_t									aMaxStringLength = 256)
		{
			size_t count;
			if(!ReadUInt(count))
				return false;

			if(count > aMaxArrayLength)
				return false;

			aOut.resize(count);
			for(size_t i = 0; i < count; i++)
			{
				if(!ReadString(aOut[i], aMaxStringLength))
					return false;
			}

			return true;
		}

		bool
		ReadBool(
			bool&									aOut)
		{
			return Read(&aOut, sizeof(aOut)) == sizeof(aOut);
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
		virtual bool						IsEnd() const = 0;
		virtual size_t						Read(
												void*	aBuffer,
												size_t	aBufferSize) = 0;

		// FIXME: this is weird, relevant FromStream methods should get these from somewhere else.
		virtual const AuraEffectFactory*	GetAuraEffectFactory() const { assert(false); return NULL; }
		virtual const ComponentManager*		GetComponentManager() const { assert(false); return NULL; }
		virtual const DirectEffectFactory*	GetDirectEffectFactory() const { assert(false); return NULL; }
		virtual const MapGeneratorFactory*	GetMapGeneratorFactory() const { assert(false); return NULL; }
		virtual const ObjectiveTypeFactory* GetObjectiveTypeFactory() const { assert(false); return NULL; }

	private:

		int32_t				m_tick = 0;
		bool				m_networkStream = false;
	};

}