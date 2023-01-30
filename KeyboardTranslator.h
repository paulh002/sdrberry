#pragma once
#include <stdint.h>
#include <time.h>
#include <ctime>
#include <chrono>
#include <string>
#include <string.h>
#include <sys/time.h>
#include "SDL_keycode.h"
#include "scancodes_linux.h"
#include "scancodes_ascii.h"
//#include "imKStoUCS.h"

#define SDL_arraysize(array) (sizeof(array) / sizeof(array[0]))
#define SDL_TABLESIZE(table) SDL_arraysize(table)
#define KEYBOARD_SOURCE_MASK (KEYBOARD_HARDWARE | KEYBOARD_AUTORELEASE)
#define SDL_FALSE 0
#define SDL_TRUE 1
#define SDL_RELEASED 0
#define SDL_PRESSED 1

#define SDL_EVENT_KEY_DOWN 0x300
#define SDL_EVENT_KEY_UP 0x301

typedef int32_t SDL_Window;
typedef int32_t SDL_Keycode;

typedef enum
{
	SDL_SCANCODE_TABLE_DARWIN,
	SDL_SCANCODE_TABLE_LINUX,
	SDL_SCANCODE_TABLE_XFREE86_1,
	SDL_SCANCODE_TABLE_XFREE86_2,
	SDL_SCANCODE_TABLE_XVNC,
} SDL_ScancodeTable;

static const struct
{
	SDL_ScancodeTable table;
	SDL_Scancode const *scancodes;
	int num_entries;
} SDL_scancode_tables[] = {
	{SDL_SCANCODE_TABLE_LINUX, linux_scancode_table, SDL_arraysize(linux_scancode_table)}
};

typedef struct SDL_Keysym
{
	SDL_Scancode scancode; /**< SDL physical key code - see ::SDL_Scancode for details */
	SDL_Keycode sym;	   /**< SDL virtual key code - see ::SDL_Keycode for details */
	SDL_Keycode key;	   /**< SDL virtual key code - see ::SDL_Keycode for details */
	uint32_t mod;			   /**< current key modifiers */
	uint32_t unused;
} SDL_Keysym;

typedef struct SDL_KeyboardEvent
{
	uint32_t type;	  /**< ::SDL_EVENT_KEY_DOWN or ::SDL_EVENT_KEY_UP */
	std::chrono::nanoseconds timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
					  //	SDL_WindowID windowID; /**< The window with keyboard focus, if any */
	uint8_t state;	  /**< ::SDL_PRESSED or ::SDL_RELEASED */
	uint8_t repeat;	 /**< Non-zero if this is a key repeat */
	uint8_t padding2;
	uint8_t padding3;
	SDL_Keysym keysym; /**< The key that was pressed or released */
} SDL_KeyboardEvent;

typedef enum
{
	KEYBOARD_HARDWARE = 0x01,
	KEYBOARD_AUTORELEASE = 0x02,
	KEYBOARD_IGNOREMODIFIERS = 0x04
} SDL_KeyboardFlags;

struct SDL_Keyboard
{
	/* Data common to all keyboards */
	SDL_Window *focus;
	uint16_t modstate;
	uint8_t keysource[SDL_NUM_SCANCODES];
	uint8_t keystate[SDL_NUM_SCANCODES];
	SDL_Keycode keymap[SDL_NUM_SCANCODES];
	SDL_Keycode keymapShift[SDL_NUM_SCANCODES];
	bool autorelease_pending;
};

class KeyboardTranslator
{
  public:
	KeyboardTranslator();
	
	SDL_Scancode GetScancodeFromTable(SDL_ScancodeTable table, int keycode);
	int SendKeyboardKey(std::chrono::nanoseconds timestamp, uint8_t state, SDL_Scancode scancode, SDL_KeyboardEvent &event);
	const char *GetScancodeName(SDL_Scancode scancode);

  private:
	SDL_Keyboard keyboard;
	void SDL_InvalidParamError(std::string param);
	int SDL_SendKeyboardKeyInternal(std::chrono::nanoseconds timestamp, SDL_KeyboardFlags flags, uint8_t state, SDL_Scancode scancode, SDL_Keycode keycode, SDL_KeyboardEvent &event);
	const SDL_Scancode *SDL_GetScancodeTable(SDL_ScancodeTable table, int *num_entries);
	void SetKeymap(int start, const SDL_Keycode *keys, const SDL_Keycode *shiftkeys, int length);
};

