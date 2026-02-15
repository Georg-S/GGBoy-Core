#pragma once

#include <vector>
#include <fstream>
#include <filesystem>

#include "Utility.hpp"

namespace ggb
{
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

	class BinaryStream 
	{
	public:
		BinaryStream(const std::vector<std::byte>& vec) 
		{
			m_current = reinterpret_cast<const char*>(vec.data());
			m_remainingSize = vec.size();
		}

		template<typename T>
		void deserialize(T& outPod)
		{
			static_assert(std::is_trivially_copyable_v<T> == true);
			if (sizeof(T) > m_remainingSize)
				throw std::runtime_error("Tried to read beyond the end of the binary stream");

            memcpySecure(static_cast<void*>(&outPod), sizeof(T), m_current, sizeof(T));
			m_current += sizeof(T);
			m_remainingSize -= sizeof(T);
		}

		template<typename T>
		void deserialize(std::vector<T>& outVec)
		{
			size_t size = 0;
			deserialize(size);
			outVec.resize(size);
			for (auto& elem : outVec)
				deserialize(elem);
		}

	private:
		const char* m_current = nullptr;
		size_t m_remainingSize = 0;
	};

	class Serialization
	{
	public:
		enum Type 
		{
			Serialize, 
			Deserialize, 
			DeserializeVector,
		};

		Serialization(const std::filesystem::path& path, bool serialize) 
		{
			if (serialize) 
			{
				m_serializeStream.open(path, std::ios::binary);
				m_type = Serialize;
			}
			else 
			{
				m_deserializeStream.open(path, std::ios::binary);
				m_type = Deserialize;
			}
		}

		Serialization(const std::vector<std::byte>& binaryData) 
			: m_type(DeserializeVector)
		{
			m_binStream = std::make_unique<BinaryStream>(binaryData);
		}

		template<typename T>
		void read_write(T& data)
		{
			static_assert(!std::is_pointer_v<T>, "Pointers are not allowed for serialization");

			if (m_type == Serialize)
				serialize(m_serializeStream, data);
			else if (m_type == Deserialize)
				deserialize(m_deserializeStream, data);
			else
				m_binStream->deserialize(data);
		}

	protected:
		// The Serialization class is just an interface that shouldn't be used directly,
		// therfore the constructor is protected
		Serialization() = default;

		Type m_type = Serialize;
		std::ofstream m_serializeStream;
		std::ifstream m_deserializeStream;
		std::unique_ptr<BinaryStream> m_binStream;
	};
}
