#pragma once

extern bool debug_print;

#define DEBUG_PRINTF(fmt, ...) \
	do \
	{ \
		if (debug_print) \
		{ \
			std::printf(fmt __VA_OPT__(, ) __VA_ARGS__); \
		} \
	} while (0)
