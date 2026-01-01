#include "Shuttle.h"
#include "vfo.h"
#include <optional>
#include <iostream>
#include <iomanip>
#include "sdrberry.h"
#include "gui_bar.h"

Shuttle::Shuttle()
{
	step = 1; // steps managed by user
	wheel = 0;
	last_time = std::chrono::high_resolution_clock::now();
	device = std::make_unique<HIDDeviceMonitor>("ShuttleXpress", 0x0b33, 0x0020);
}

void Shuttle::start()
{
	device->start();
}

void printReport(const std::vector<unsigned char> &report)
{
	std::cout << "Report: ";
	for (uint8_t b : report)
	{
		std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)b << " ";
	}
	std::cout << std::dec << std::endl;
}

void Shuttle::step_vfo()
{
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);

	auto reports = device->tryPopReport();

	if (reports)
	{
		printReport(*reports);
		value = (int8_t)reports->at(0);
		uint8_t wheel_move = reports->at(1);
		decode_buttons(reports->at(3), reports->at(4));
		if (value == 0 && wheel_move != wheel)
		{
			if (wheel == 0 && wheel_move == 255)
			{
				vfo.step_vfo(step * -1);
			}
			else if (wheel == 255 && wheel_move == 0)
				{
					wheel = wheel_move;
					vfo.step_vfo(step);
				}
			else if (wheel_move > wheel)
					vfo.step_vfo(step);
				else
					vfo.step_vfo(step * -1);
			wheel = wheel_move;
		}
	}
	if (timePassed.count() > 20 && value == 1)
	{
		vfo.step_vfo(step);
		last_time = now;
	}
	if (timePassed.count() > 10 && value == 2)
	{
		vfo.step_vfo(step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == 3)
	{
		vfo.step_vfo(step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == 4)
	{
		vfo.step_vfo(1.5 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == 5)
	{
		vfo.step_vfo(3 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == 6)
	{
		vfo.step_vfo(9 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == 7)
	{
		vfo.step_vfo(18 * step);
		last_time = now;
	}
	if (timePassed.count() > 20 && value == -1)
	{
		vfo.step_vfo(-1 * step);
		last_time = now;
	}
	if (timePassed.count() > 10 && value == -2)
	{
		vfo.step_vfo(-1 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == -3)
	{
		vfo.step_vfo(-1 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == -4)
	{
		vfo.step_vfo(-1.5 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == -5)
	{
		vfo.step_vfo(-3 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == -6)
	{
		vfo.step_vfo(-9 * step);
		last_time = now;
	}
	if (timePassed.count() > 1 && value == -7)
	{
		vfo.step_vfo(-18 * step);
		last_time = now;
	}
	return;
}

void Shuttle::decode_buttons(unsigned char a, unsigned char b)
{
	if (a || b || button_a || button_b)
	{
		if (a & 0x10 || button_a & 0x10)
		{
			if (a & 0x10)
			{
					select_mode_tx(mode);
					gbar.set_tx(true);
			}
			else
			{
					select_mode(mode);
					gbar.set_tx(false);
			}
			button_a = a;
		}
	}
}