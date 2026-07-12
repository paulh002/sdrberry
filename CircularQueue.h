#pragma once
#include <iostream>
#include <vector>
#include <complex>
#include <span>       // Required for std::span
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <atomic>
#include <chrono>

template <typename T, size_t Capacity>
	class CircularQueue {
		std::array<std::vector<T>, Capacity> pool;
    
		size_t head = 0; 
		size_t tail = 0; 
		std::atomic<size_t> count{0}; 
    
		std::mutex m;
		std::condition_variable cv;
		bool is_ended = false;

	public:
		CircularQueue() {
		}
		
		CircularQueue(size_t reserve_size) {
			for (auto& vec : pool) {
				vec.reserve(reserve_size);
			}
		}

		void reserve(size_t reserve_size)
		{
			for (auto& vec : pool) {
				vec.reserve(reserve_size);
			}
			
		}
		
		void push_block(size_t actual_size, auto&& filler) {
			{
				std::lock_guard<std::mutex> lock(m);
				auto& vec = pool[head];
				vec.resize(actual_size);
				
				// 1. Create a writable view of the buffer
				std::span<T> write_view(vec);
            
				// 2. Call the user-provided lambda to fill the data
				filler(write_view);
            
				// 3. Advance the circular queue pointers
				head = (head + 1) % Capacity;
            
				if (count.load(std::memory_order_relaxed) == Capacity) {
					tail = (tail + 1) % Capacity; 
				}
				else {
					count.fetch_add(1, std::memory_order_relaxed);
				}
			}
			cv.notify_one(); 
		}

		int pop_block() {
			std::unique_lock<std::mutex> lock(m);
        
			cv.wait(lock,
				[this] { 
					return count.load(std::memory_order_relaxed) > 0 || is_ended; 
				});
        
			if (count.load(std::memory_order_relaxed) > 0) {
				int idx = tail;
				tail = (tail + 1) % Capacity;
				count.fetch_sub(1, std::memory_order_relaxed);
				return idx;
			}
        
			return -1; 
		}

		// NEW: Clear function to reset the queue
		void clear() {
			{
				std::lock_guard<std::mutex> lock(m);
				head = 0;
				tail = 0;
				count.store(0, std::memory_order_relaxed);
            
				// Optional: Resize all vectors to 0 (calls destructors but keeps capacity)
				// This ensures a "clean" state, but isn't strictly necessary since
				// push_block will resize anyway.
				for (auto& vec : pool) {
					vec.resize(0);
				}
			}
			// Wake up all waiting threads so they can see the cleared state
			cv.notify_all();
		}
		
		// UPDATED ACCESSOR: Returns a std::span instead of a vector reference
		std::span<T> get_buffer_view(size_t idx) {
			// std::vector implicitly converts to std::span in C++20!
			return pool[idx]; 
		}

		void push_end() {
			{
				std::lock_guard<std::mutex> lock(m);
				is_ended = true;
			}
			cv.notify_one();
		}

		size_t size() const {
			return count.load(std::memory_order_relaxed);
		}
	};