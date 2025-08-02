#include "tempSensor.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/inotify.h>

tempSensor::tempSensor()
{
	discover_tempSensors();
}

bool tempSensor::discover_tempSensors()
{
	DIR *dir = opendir(TEMP_DISCOVERY_PATH);
	if (dir == NULL)
	{
		if (errno != ENOENT)
		{
			LV_LOG_ERROR("opendir failed: %s", strerror(errno));
		}
		return false;
	}
	while (1)
	{
		struct dirent *dirent = readdir(dir);
		if (dirent == NULL)
			break; /* only possible error is EBADF, so no errno check needed */
		open_tempsensor(dirent->d_name);
	}
	closedir(dir);
	return true;
}

bool tempSensor::open_tempsensor(char *filename)
{
	char dev_path[TEMP_DISCOVERY_PATH_BUF_SIZE];
	if (strstr(filename, TEMP_IDENTIFICATION) != NULL && tempsensors.size() < MAX_AMOUNT_SENSORS)
	{
		lv_snprintf(dev_path, sizeof(dev_path), TEMP_DISCOVERY_PATH "%s", filename);
		tempsensors.push_back(std::string(dev_path) + "/temperature");
	}
	return true;
}

int tempSensor::read_tempsensor(int sensor)
{
	if (tempsensors.size() > sensor)
	{
		char buf[80];
		FILE *fd = fopen(tempsensors.at(sensor).c_str(), "r");
		if (fd != NULL)
		{
			fgets(buf, 20, fd);
			fclose(fd);
			return atoi(buf);
		}
	}
	return 99999;
}

int tempSensor::count_tempsensors()
{
	return tempSensor::ptr->tempsensors.size();
}

void tempSensor::operator()()
{
	stop_flag = false;
	while (!stop_flag.load())
	{
		for (int i = 0; i < tempsensors.size(); i++)
		{
			temperature.at(i) = read_tempsensor(i);
		}
		usleep(1000 * read_sleep_time);
	}
}

void tempSensor::start_read_out()
{
	if (tempSensor::ptr != nullptr)
		return;
	tempSensor::ptr = std::make_shared<tempSensor>();
	read_out_thread = std::thread(&tempSensor::operator(), tempSensor::ptr);
}

void tempSensor::stop_read_out()
{
	stop_flag = true;
	read_out_thread.join();
	ptr.reset();
}

int tempSensor::get_temperature(int sensor)
{
	return tempSensor::ptr->get_temperature_readout(sensor);
}

int tempSensor::get_temperature_readout(int sensor)
{
	return temperature.at(sensor);
}