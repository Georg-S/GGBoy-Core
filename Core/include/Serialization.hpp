#pragma once

#include <vector>
#include <fstream>
#include <filesystem>

namespace ggb {

	template<typename T>
	void serialize(std::ostream& outStream, const T& pod)
	{
		static_assert(std::is_trivially_copyable_v<T> == true);
		outStream.write(reinterpret_cast<const char*>(&pod), sizeof(T));
	}

	template<typename T>
	void deserialize(std::istream& inStream, T& outPod)
	{
		static_assert(std::is_trivially_copyable_v<T> == true);
		inStream.read(reinterpret_cast<char*>(&outPod), sizeof(T));
	}

	template<typename T>
	void serialize(std::ostream& outStream, const std::vector<T>& toSerialize)
	{
		serialize(outStream, toSerialize.size());
		for (const auto& elem : toSerialize)
			serialize(outStream, elem);
	}

	template<typename T>
	void deserialize(std::istream& inStream, std::vector<T>& outVec)
	{
		size_t size = 0;
		deserialize(inStream, size);
		outVec.resize(size);
		for (auto& elem : outVec)
			deserialize(inStream, elem);
	}

	class Serialization 
	{
	public:
		virtual ~Serialization() = default;

		template<typename T>
		void read_write(T& data)
		{
			static_assert(!std::is_pointer_v<T>, "Pointers are not allowed for serialization");

			if (m_serialize)
				serialize(m_serializeStream, data);
			else 
				deserialize(m_deserializeStream, data);
		}

	protected:
		// The Serialization class is just an interface that shouldn't be used directly,
		// therfore the constructor is protected
		Serialization() = default; 

		bool m_serialize = true;
		std::ofstream m_serializeStream;
		std::ifstream m_deserializeStream;
	};

	class Serialize : public Serialization
	{
	public:
		Serialize(const std::filesystem::path& outPath) 
		{
			m_serializeStream.open(outPath, std::ios::binary);
			m_serialize = true;
		}
	};

	class Deserialize : public Serialization 
	{
	public:
		Deserialize(const std::filesystem::path& inPath) 
		{
			m_deserializeStream.open(inPath, std::ios::binary);
			m_serialize = false;
		}
	};
}