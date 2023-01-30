#include "Keyboard.h"
#include <linux/input.h>
#include <libevdev-1.0/libevdev/libevdev.h>
//#include "SDL_keyboard_c.h"
//#include "SDL_scancode_tables_c.h"

#define EV_KEY 0x01
#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define NBITS(x) ((((x)-1) / (sizeof(long) * 8)) + 1)

void Keyboard::init_keyboard()
{
	Index = 0;
	do
	{
		sprintf(KeyboardName, "/dev/input/event%d", Index);
		fd = open(KeyboardName, O_RDONLY | O_NONBLOCK);
		if (fd > 0)
		{
			int keyBitmask[KEY_MAX / (sizeof(int) * 8) + 1];
			unsigned long ev_bits[NBITS(EV_MAX)];

			char name[256] = "Unknown";
			ioctl(fd, EVIOCGNAME(sizeof(name)), name);
			printf("Input device name: \"%s\"\n", name);

			if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask) == -1)
			{
				printf("ioctl error\n");
				close(fd);
				Index++;
				fd = -1;
			}

			if (keyBitmask[KEY_A / (sizeof(int) * 8)] & (1 << (KEY_A % (sizeof(int) * 8))))
			{
				printf("Device is a keyboard\n");
				ioctl(fd, EVIOCGRAB, 1);

				if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) == -1)
				{
					printf("ioctl error\n");
					close(fd);
					Index++;
					fd = -1;
				}
				if (test_bit(EV_KEY, ev_bits) && test_bit(EV_REL, ev_bits))
				{
					printf("Keyboard also has a mouse\n");
					mouse = make_unique<Mouse>(fd);
				}
			}
			else
			{
				printf("Device is not a keyboard\n");
				close(fd);
				Index++;
				fd = -1;
			}
		}
		else
			Index++;
	} while (Index < 30 && fd == -1);

	int rc = libevdev_new_from_fd(fd, &dev);
	if (rc < 0)
		printf("cannot open evdev \n");

	//SDL_InitKeyboard();
	printf("end keyboard \n");
}

Keyboard::Keyboard()
{
	fd = -1;
}

Keyboard::~Keyboard()
{
	if (fd != -1)
	{
		ioctl(fd, EVIOCGRAB, 1);
		close(fd);
		fd = -1;
	}
}

bool Keyboard::Attached()
{
	if (fd != -1)
		return true;
	return false;
}

MouseState Keyboard::GetMouseState()
{
	MouseState m{0,0,false, false};
	if (mouse != nullptr)
		return mouse->GetMouseState();
	else
		return m;
}


std::string Keyboard::GetKeys()
{
	int bytes;
	std::string keys;
	SDL_KeyboardEvent sdl_keyboard_event;

	if (fd > 0)
	{
		struct input_event keyboard_event;
	
		bytes = read(fd, (void *)&keyboard_event, sizeof(struct input_event));
		while (bytes != -1)
		{
			if (keyboard_event.type == EV_KEY) // && keyboard_event.value == 1
			{
				SDL_Scancode scancode = kbTranslator.GetScancodeFromTable(SDL_SCANCODE_TABLE_LINUX, keyboard_event.code);
				//printf("type %d code %d value %d scancode %d\n", keyboard_event.type, keyboard_event.code, keyboard_event.value, scancode);

				if (scancode != SDL_SCANCODE_UNKNOWN)
				{
					auto KeyPresstime = std::chrono::high_resolution_clock::now();

					if (keyboard_event.value == 0)
					{
						kbTranslator.SendKeyboardKey(KeyPresstime.time_since_epoch(), SDL_RELEASED, scancode, sdl_keyboard_event);
						//printf("SDL_RELEASED char %c, scancode %s \n", sdl_keyboard_event.keysym.key, kbTranslator.GetScancodeName(scancode));
					}
					else if (keyboard_event.value == 1 || keyboard_event.value == 2 )
					{
						kbTranslator.SendKeyboardKey(KeyPresstime.time_since_epoch(), SDL_PRESSED, scancode, sdl_keyboard_event);
						//if (sdl_keyboard_event.keysym.mod & SDL_KMOD_CAPS)
						//	printf("CAPS_LOCK ");
						//if (sdl_keyboard_event.keysym.mod & SDL_KMOD_RSHIFT || sdl_keyboard_event.keysym.mod & SDL_KMOD_LSHIFT)
						//	printf("SHIFT ");

						//printf("SDL_PRESSED char %c, scancode %s \n", sdl_keyboard_event.keysym.key, kbTranslator.GetScancodeName(scancode));
						if (scancode > 3 && scancode < 57)
							keys.push_back((char)sdl_keyboard_event.keysym.key);
					}
				} 
			}
		bytes = read(fd, (void *)&keyboard_event, sizeof(struct input_event));
		}
	}
	return keys;
}