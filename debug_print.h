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

#define DEBUG_CAT_PRINTF(fmt, ...)                       \
	do                                                   \
	{                                                    \
		if (cat_debug)                                 \
		{                                                \
			std::printf(fmt __VA_OPT__(, ) __VA_ARGS__); \
		}                                                \
	} while (0)
