#pragma once
#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <mutex>
#include <queue>
#include <vector>
#include <complex>

/** Buffer to move sample data between threads. */
template <class Element>
class DataBuffer
{
  public:
	/** Constructor. initialize members */
	DataBuffer(std::string name = "")
		: m_qlen(0), m_end_marked(false)
	{
	}

	/** Add samples to the queue. */
	void push(std::vector<Element> &&samples)
	{
		if (!samples.empty())
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_qlen += samples.size();
			m_queue.push(move(samples));
			lock.unlock();
			m_cond.notify_all();
		}
	}

	/** Mark the end of the data stream. */
	void push_end()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_end_marked = true;
		lock.unlock();
		m_cond.notify_all();
	}

	void clear()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		std::vector<Element> ret;
		while (!m_queue.empty())
		{
			m_qlen -= m_queue.front().size();
			swap(ret, m_queue.front());
			m_queue.pop();
			ret.clear();
		}
		m_end_marked = false;
		lock.unlock();
		m_cond.notify_all();
	}

	/** Return number of samples in queue. */
	size_t queued_samples()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_qlen;
	}

	/**
		 * If the queue is non-empty, remove a block from the queue and
		 * return the samples. If the end marker has been reached, return
		 * an empty std::vector. If the queue is empty, wait until more data is pushed
		 * or until the end marker is pushed.
		 */
	std::vector<Element> pull()
	{
		std::vector<Element> ret;
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_queue.empty() && !m_end_marked)
		{
			m_cond.wait(lock); // conditional wait unlocks the mutex!
		}
		if (!m_queue.empty())
		{
			m_qlen -= m_queue.front().size();
			swap(ret, m_queue.front());
			m_queue.pop();
			lock.unlock();
			m_cond.notify_all();
		}
		return ret;
	}

	/** Return true if the end has been reached at the Pull side. */
	bool pull_end_reached()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_qlen == 0 && m_end_marked;
	}

	/** Wait until the buffer contains minfill samples or an end marker. */
	void wait_buffer_fill(size_t minfill)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_qlen < minfill && !m_end_marked)
			m_cond.wait(lock);
	}

	/** Wait until the queue is reaching bottom or an end marker. */
	void wait_queue_empty(size_t maxfill)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_queue.size() > maxfill && !m_end_marked)
			m_cond.wait(lock);
	}

	void restart_queue()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_end_marked = false;
	}

	int size()
	{
		return m_queue.size();
	}

	size_t get_qlen()
	{
		return m_qlen;
	}

  private:
	size_t m_qlen;
	bool m_end_marked;
	std::queue<std::vector<Element>> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond;
};

template <class Element>
class DataQueue
{
  public:
	/** Add samples to the queue. */
	void push(Element samples)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_queue.push(samples);
	}

	bool pull(Element &sample)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (!m_queue.empty())
		{
			sample = m_queue.front();
			m_queue.pop();
			return true;
		}
		return false;
	}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

  private:
	std::queue<Element> m_queue;
	std::mutex m_mutex;
};

