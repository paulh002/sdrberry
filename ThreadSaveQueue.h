#pragma once
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

template <typename T>
class ThreadSafeQueue
{
  public:
	void push(T item)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(std::move(item));
		cond_.notify_one();
	}

	std::optional<T> try_pop()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty())
		{
			return std::nullopt;
		}
		T item = std::move(queue_.front());
		queue_.pop();
		return item;
	}

	// Optional: blocking pop (not used here, but handy)
	T wait_and_pop()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] { return !queue_.empty(); });
		T item = std::move(queue_.front());
		queue_.pop();
		return item;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

  private:
	mutable std::mutex mutex_;
	std::condition_variable cond_;
	std::queue<T> queue_;
};