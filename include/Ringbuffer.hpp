#pragma once
#include <atomic>

namespace ggb
{
	/// A lock free (at least on most platforms) ring buffer
	template<typename T, size_t MAX_SIZE>
	class SingleProducerSingleConsumerRingbuffer
	{
	public:
		constexpr size_t size() const 
		{
			return MAX_SIZE;
		}

		size_t push(const T& data)
		{
			T toPush = data;
			return push(std::move(toPush));
		}

		size_t push(T&& data)
		{
			auto writeIndex = m_lastWriteIndex.load();
			auto readIndex = m_lastReadIndex.load();
			auto buf = readIndex;
			if (writeIndex > readIndex)
				buf += MAX_SIZE;
			auto distance = buf - writeIndex;


			writeIndex = ((writeIndex + 1) % MAX_SIZE);
			if (writeIndex == readIndex) 
				return distance;

			m_buffer[writeIndex] = std::move(data);
			m_lastWriteIndex.store(writeIndex);
			return distance;
		}

		T pop(T defaultReturnValue)
		{
			pop(&defaultReturnValue);
			return defaultReturnValue;
		}

		bool pop(T* outValue)
		{
			auto writeIndex = m_lastWriteIndex.load();
			auto readIndex = m_lastReadIndex.load();
			if (writeIndex == readIndex)
				return false;

			readIndex = ((readIndex + 1) % MAX_SIZE);
			const auto& result = m_buffer[readIndex];
			m_lastReadIndex.store(readIndex);

			*outValue = result;
			return true;
		}

	private:
		std::atomic<size_t> m_lastReadIndex = 0;
		std::atomic<size_t> m_lastWriteIndex = 0;
		T m_buffer[MAX_SIZE] = {};
	};
}