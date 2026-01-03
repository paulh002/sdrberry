// hid_device_monitor.hpp
#pragma once

#include <atomic>
#include <chrono>
#include <cstring> // for std::strncmp
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/hidraw.h>
#include <poll.h>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <optional>
#include "ThreadSaveQueue.h"

class HIDDeviceMonitor
{
  public:
	using Report = std::vector<unsigned char>;

	HIDDeviceMonitor(const std::string &device_name,
					 unsigned short vid,
					 unsigned short pid)
		: device_name_(device_name), vid_(vid), pid_(pid), running_(false) {}

	void start()
	{
		if (running_)
			return;
		running_ = true;
		monitor_thread_ = std::thread(&HIDDeviceMonitor::monitorLoop, this);
	}

	void stop()
	{
		if (!running_)
			return;
		running_ = false;
		if (monitor_thread_.joinable())
		{
			monitor_thread_.join();
		}
	}

	~HIDDeviceMonitor()
	{
		stop();
	}

	// Non-blocking: get one pending report, or nullopt if none
	std::optional<Report> tryPopReport()
	{
		return report_queue_.try_pop();
	}

	// Get all pending reports
	std::vector<Report> drainReports()
	{
		std::vector<Report> reports;
		while (auto report = report_queue_.try_pop())
		{
			reports.push_back(std::move(*report));
		}
		return reports;
	}

  private:
	std::string device_name_;
	unsigned short vid_;
	unsigned short pid_;
	std::atomic<bool> running_;
	std::thread monitor_thread_;
	ThreadSafeQueue<Report> report_queue_;

	bool isTargetDevice(const std::string &hidraw_name)
	{
		std::string uevent_path = "/sys/class/hidraw/" + hidraw_name + "/device/uevent";
		std::ifstream uevent(uevent_path);
		if (!uevent)
			return false;

		std::string line;
		while (std::getline(uevent, line))
		{
			if (line.find("HID_ID=") == 0)
			{
				unsigned int bus, v, p;
				if (sscanf(line.c_str(), "HID_ID=%x:%x:%x", &bus, &v, &p) == 3)
				{
					return (static_cast<unsigned short>(v) == vid_ &&
							static_cast<unsigned short>(p) == pid_);
				}
			}
		}
		return false;
	}

	int openMatchingHIDRaw()
	{
		DIR *dir = opendir("/dev");
		if (!dir)
			return -1;

		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
		{
			if (std::strncmp(entry->d_name, "hidraw", 6) == 0)
			{
				if (isTargetDevice(entry->d_name))
				{
					std::string dev_path = "/dev/" + std::string(entry->d_name);
					int fd = open(dev_path.c_str(), O_RDONLY | O_NONBLOCK);
					if (fd >= 0)
					{
						std::cout << "[" << device_name_ << "] Opened " << dev_path << std::endl;
						closedir(dir);
						return fd;
					}
				}
			}
		}
		closedir(dir);
		return -1;
	}

	void monitorLoop()
	{
		int fd = -1;
		bool connected = false;
		constexpr size_t MAX_REPORT_SIZE = 64;
		std::vector<unsigned char> buffer(MAX_REPORT_SIZE);

		while (running_)
		{
			if (!connected)
			{
				fd = openMatchingHIDRaw();
				if (fd >= 0)
				{
					connected = true;
					std::cout << "[" << device_name_ << "] Connected." << std::endl;
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::seconds(2));
					continue;
				}
			}

			struct pollfd pfd = {fd, POLLIN, 0};
			int ret = poll(&pfd, 1, 2000);

			if (ret < 0 || ret == 0)
			{
				// Check if device vanished
				ssize_t n = read(fd, buffer.data(), buffer.size());
				if (n < 0 && (errno == ENODEV || errno == EIO))
				{
					std::cout << "[" << device_name_ << "] Unplugged." << std::endl;
					close(fd);
					fd = -1;
					connected = false;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
				continue;
			}

			ssize_t n = read(fd, buffer.data(), buffer.size());
			if (n < 0)
			{
				if (errno == ENODEV || errno == EIO)
				{
					std::cout << "[" << device_name_ << "] Lost during read." << std::endl;
					close(fd);
					fd = -1;
					connected = false;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
			else if (n > 0 && running_)
			{
				Report report(buffer.begin(), buffer.begin() + n);
				report_queue_.push(std::move(report));
			}
		}

		if (fd >= 0)
			close(fd);
	}
};