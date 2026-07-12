#pragma once
#include <iostream>
#include <vector>
#include <complex>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>

// Optimized Thread-Safe Queue for Audio Blocks
template <typename T>
	class AudioBlockQueue {
		std::queue<T> q;
		std::mutex m;
		std::condition_variable cv;
		bool is_ended = false;
    
		// C++20 optimization: atomic size counter so size() doesn't need to lock the mutex
		std::atomic<size_t> atomic_size{0}
		; 

	public:
		// Called by the Audio Producer
		void push_audio_block(T val) {
			{
				std::lock_guard<std::mutex> lock(m);
				q.push(val);
				atomic_size.fetch_add(1, std::memory_order_relaxed);
			}
			// Wake up the consumer IMMEDIATELY. 
			// notify_one is faster than notify_all for a single consumer.
			cv.notify_one(); 
		}

		void push_end() {
			{
				std::lock_guard<std::mutex> lock(m);
				is_ended = true;
			}
			cv.notify_one(); // Wake consumer so it can see the end flag
		}

		// Called by the Audio Consumer
		int pop_audio_block() {
			std::unique_lock<std::mutex> lock(m);
        
			// The thread SLEEPS here. It consumes 0% CPU.
			// It wakes up ONLY when notify_one() is called by the producer.
			cv.wait(lock, [this] { return !q.empty() || is_ended; });
        
			if (!q.empty()) {
				int val = q.front();
				q.pop();
				atomic_size.fetch_sub(1, std::memory_order_relaxed);
				return val;
			}
        
			return -1; // End of stream
		}

		// Lock-free size check using the atomic counter
		size_t size() const {
			return atomic_size.load(std::memory_order_relaxed);
		}
	};
