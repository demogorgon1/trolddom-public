#pragma once

namespace kaos_public
{

	class ComponentFactory;
	class EffectFactory;

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
		ReadBool(
			bool&									aOut)
		{
			return Read(&aOut, sizeof(aOut)) == sizeof(aOut);
		}

		// Virtual interface
		virtual bool					IsEnd() const = 0;
		virtual size_t					Read(
											void*	aBuffer,
											size_t	aBufferSize) = 0;
		virtual const ComponentFactory* GetComponentFactory() const = 0;
		virtual const EffectFactory*	GetEffectFactory() const = 0;
	};

}