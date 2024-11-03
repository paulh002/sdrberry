#include "Keyboard.h"

#define EV_KEY 0x01
#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define NBITS(x) ((((x)-1) / (sizeof(long) * 8)) + 1)

static std::chrono::milliseconds double_click_threshold(300);

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
					mouse_attached = true;
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
	printf("end keyboard \n");
}

Keyboard::Keyboard()
	: fd(-1), mouse_attached(false), click_count(0)
{
	last_click_time = std::chrono::system_clock::now();
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
	if (mousestates.empty())
		return m;
	m = mousestates.back();
	mousestates.pop();
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
		state.MouseActivity = false;
		state.Rotated = 0;
	
		bytes = read(fd, (void *)&keyboard_event, sizeof(struct input_event));
		while (bytes != -1)
		{
			//printf("type %x code %x value %d \n", keyboard_event.type, keyboard_event.code, keyboard_event.value);

			if (keyboard_event.type == EV_REL && keyboard_event.code == REL_X)
			{
				state.x = state.x + keyboard_event.value;
				if (state.x >= screenWidth)
					state.x = screenWidth - 1;
				if (state.x < 0)
					state.x = 0;
				state.MouseActivity = true;
				//printf("type %x code %x value %d \n", keyboard_event.type, keyboard_event.code, keyboard_event.value);
				//mousestates.push(state);
			}
			if (keyboard_event.type == EV_REL && keyboard_event.code == REL_Y)
			{
				state.y = state.y + keyboard_event.value;
				if (state.y >= screenHeight)
					state.y = screenHeight - 1;
				if (state.y < 0)
					state.y = 0;
				state.MouseActivity = true;
				//printf("type %x code %x value %d \n", keyboard_event.type, keyboard_event.code, keyboard_event.value);
				//mousestates.push(state);
			}

			if (keyboard_event.type == EV_KEY && keyboard_event.code == BTN_LEFT)
			{
				// printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);

				if (keyboard_event.value == 1)
				{
					state.pressed = LV_INDEV_STATE_PR;
					state.MouseActivity = true;
					std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
					if (current_time - last_click_time < double_click_threshold)
					{
						click_count++;
						if (click_count == 2)
						{
							// cout << "Double click detected!" << endl;
							click_count = 0;
							state.doubleclick = true;
						}
					}
					else
					{
						click_count = 1;
						state.doubleclick = false;
					}
					last_click_time = current_time;
				}
				if (keyboard_event.value == 0)
				{
					state.pressed = LV_INDEV_STATE_REL;
					state.MouseActivity = true;
				}
			}

			if (keyboard_event.type == EV_KEY && keyboard_event.code != BTN_LEFT) // && keyboard_event.value == 1
			{
				SDL_Scancode scancode = kbTranslator.GetScancodeFromTable(SDL_SCANCODE_TABLE_LINUX, keyboard_event.code);
				//printf("type %x code %x value %d scancode %d\n", keyboard_event.type, keyboard_event.code, keyboard_event.value, scancode);

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
//		if (state.MouseActivity)
//			printf("x %d y %d pressed %d activity %d \n", state.x, state.y, state.pressed, state.MouseActivity);
		mousestates.push(state);
	}
	return keys;
}