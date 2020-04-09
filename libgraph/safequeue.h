#ifndef __SAFE_QUEUE_H__
#define __SAFE_QUEUE_H__

#include <mutex>
#include <queue>
#include <iostream>

// Thread safe implementation of a Queue using a std::queue
template <typename T>
class SafeQueue {
	private:
		std::queue<T> m_queue;
		std::mutex m_mutex;
	public:
		SafeQueue() {
		}

		SafeQueue(SafeQueue& other) {
			std::cout << "Copy constructor remians undefined." << std::endl;
		}

		~SafeQueue() {

		}

		void swap(SafeQueue& other) {
			std::unique_lock<std::mutex> lock(m_mutex);
			std::swap(m_queue, other.m_queue);
		}

		bool empty() {
			std::unique_lock<std::mutex> lock(m_mutex);
			return m_queue.empty();
		}

		int size() {
			std::unique_lock<std::mutex> lock(m_mutex);
			return m_queue.size();
		}

		void enqueue(T& t) {
			std::unique_lock<std::mutex> lock(m_mutex);
			m_queue.push(t);
		}

		bool dequeue(T& t) {
			std::unique_lock<std::mutex> lock(m_mutex);

			if (m_queue.empty()) {
				return false;
			}
			t = m_queue.front();

			m_queue.pop();
			return true;
		}
};

#endif
