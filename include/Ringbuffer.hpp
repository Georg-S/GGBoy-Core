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

		// Returns the amount of elements stored in the buffer AFTER the push was attempted.
		// If the buffer is full the element is dropped.
		size_t push(const T& data)
		{
			T toPush = data;
			return push(std::move(toPush));
		}

		size_t push(T&& data)
		{
			// Each index is loaded exactly once, every decision is made on the snapshot taken here
			const auto writeIndex = m_lastWriteIndex.load();
			const auto readIndex = m_lastReadIndex.load();

			auto storedCount = (writeIndex >= readIndex)
				? writeIndex - readIndex
				: writeIndex + MAX_SIZE - readIndex;

			const auto nextWriteIndex = ((writeIndex + 1) % MAX_SIZE);
			if (nextWriteIndex == readIndex)
				return storedCount; // Buffer full -> drop the element

			m_buffer[nextWriteIndex] = std::move(data);
			m_lastWriteIndex.store(nextWriteIndex);
			return ++storedCount;
		}

		T pop(T defaultReturnValue)
		{
			pop(&defaultReturnValue);
			return defaultReturnValue;
		}

		bool pop(T* outValue)
		{
			const auto lastReadIndex = m_lastReadIndex.load();

			if (m_lastWriteIndex.load() == lastReadIndex)
				return false; // Buffer empty

			const auto readIndex = ((lastReadIndex + 1) % MAX_SIZE);
			*outValue = m_buffer[readIndex];
			m_lastReadIndex.store(readIndex);
			return true;
		}

	private:
		std::atomic<size_t> m_lastReadIndex = 0;
		std::atomic<size_t> m_lastWriteIndex = 0;
		T m_buffer[MAX_SIZE] = {};
	};
}