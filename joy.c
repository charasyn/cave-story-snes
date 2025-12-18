#include "common.h"
//#include "memory.h"
#include "system.h"
#include "vdp.h"
#include "joy.h"

#include <snes.h>

//#include "gba.h"

/*const uint16_t btn[12] = {
	BUTTON_A, BUTTON_B, BUTTON_START, BUTTON_START,
	BUTTON_RIGHT, BUTTON_LEFT, BUTTON_UP, BUTTON_DOWN,
	BUTTON_Z, BUTTON_Y, BUTTON_X, BUTTON_MODE
};*/

const uint16_t btn[12] = {
	BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT,
	BUTTON_B, BUTTON_C, BUTTON_A, BUTTON_START,
	BUTTON_Z, BUTTON_Y, BUTTON_X, BUTTON_MODE
};

const char btnName[12][4] = {
	"Up", "Dn", "Lt", "Rt",
	"B", "C", "A", "St",
	"Z", "Y", "X", "Md"
};

uint8_t joytype = JOY_TYPE_PAD6;
uint16_t joystate, oldstate = 0;

void joy_init() {
	joystate = oldstate = 0;
	return;

}

void joy_update() {
	//iprintf("joy_update %d %d\n", joystate, oldstate);
	oldstate = joystate;
	joystate = padsCurrent(0);
	return;

}
