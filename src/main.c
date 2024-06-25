#include <stdlib.h>
#include "logo.h"
#include "wasm4.h"

#define NUM_SPACES 400
#define EMPTY 0xFFFF

#define FRAME_DIV 10

#define UP -20
#define DOWN 20
#define LEFT -1
#define RIGHT 1

uint16_t map[NUM_SPACES];

uint16_t food;
uint16_t tail;
uint16_t head;
int8_t dir;
uint8_t t;
int8_t buffer[8];
uint8_t nbuffer;
uint8_t state;
#define STATE_TITLE 0
#define STATE_GAME 1
#define STATE_LOST 2

uint16_t score;

uint32_t why_doesnt_wasm4_just_let_me_use_time_ugh;

void game_init() {
	// fill map
	for (int i=0;i<NUM_SPACES;++i) map[i]=EMPTY;
	// set up snake
	score = 3;
	tail = 182;
	map[182] = 183;
	map[183] = 184;
	head = 184;
	// initial food
	food = 197;
	// seed RNG
	srand(why_doesnt_wasm4_just_let_me_use_time_ugh);
	// initial direction (not moving)
	dir = 0;
	// initial framecount (resets every FRAME_DIV frames)
	t = 0;
	// initialize buffer
	for (int i=0;i<8;++i) buffer[i]=0;
	nbuffer = 0;
}

void start() {
	// palette - white, red, green, black
	PALETTE[0] = 0xFFFFFF;
	PALETTE[1] = 0xFF0000;
	PALETTE[2] = 0x00FF00;
	PALETTE[3] = 0x000000;
	// init title screen frame counter
	why_doesnt_wasm4_just_let_me_use_time_ugh = 0;
	// start in title state
	state = STATE_TITLE;
	// init snake vars
	game_init();
}

void game_draw() {
	*DRAW_COLORS = 0x33;
	int w = tail;
	while (w<NUM_SPACES) {
		int x = (w%20)*8;
		int y = (w/20)*8;
		rect(x,y,8,8);
		w = map[w];
	}
	*DRAW_COLORS = 0x22;
	int x = (food%20)*8;
	int y = (food/20)*8;
	rect(x,y,8,8);
}

void buffer_dir(int8_t d) {
	if (nbuffer==8) return;
	if (nbuffer) {
		for (int i=0;i<nbuffer;++i) {
			if (buffer[i]==d) return;
		}
	}
	buffer[nbuffer++]=d;
}

void game_processInput() {
	uint8_t gamepad = *GAMEPAD1;
	if (gamepad & BUTTON_UP) {
		buffer_dir(UP);
	}
	if (gamepad & BUTTON_DOWN) {
		buffer_dir(DOWN);
	}
	if (gamepad & BUTTON_LEFT) {
		buffer_dir(LEFT);
	}
	if (gamepad & BUTTON_RIGHT) {
		buffer_dir(RIGHT);
	}
	if (t%FRAME_DIV) return;
	if (nbuffer) {
		int8_t bufferd = buffer[0];
		// don't let the player insta-die by pressing the opposite dir
		if ((bufferd==LEFT&&dir==RIGHT)||(bufferd==RIGHT&&dir==LEFT)||
		    (bufferd==UP&&dir==DOWN)||(bufferd==DOWN&&dir==UP)) {
			bufferd = dir;
		}
		dir = bufferd;
		for (int i=0;i<nbuffer;++i) {
			if (i<7) buffer[i]=buffer[i+1];
		}
		--nbuffer;
	}
}

void game_move() {
	if (dir==0) return;
	if (t%FRAME_DIV) return;
	int16_t newhead = (int16_t)(head+dir);
	// keep new head in playing field and aligned
	if ((head%20)==19&&(newhead%20)==0) {
		newhead-=20;
	}
	if ((newhead%20)==19&&(head%20)==0) {
		newhead+=20;
	}
	// special case: if we go left off 0, we have problems
	if (newhead==-1) newhead+=20;
	if (newhead>=NUM_SPACES) newhead-=NUM_SPACES;
	if (newhead<0) newhead+=NUM_SPACES;
	// if newhead is part of body, we lost
	if (map[newhead]<NUM_SPACES&&newhead!=tail) {
		state = STATE_LOST;
		return;
	}
	// if newhead is food, we get a new space
	int delay=0;
	if (newhead==food) {
		delay=1;
		++score;
		while (food==newhead||map[food]<NUM_SPACES) food=(uint16_t)(rand()%NUM_SPACES);
	}
	map[head]=(uint16_t)newhead;
	head=(uint16_t)newhead;
	if (delay) return; // skip tail shortening
	uint16_t tmp = map[tail];
	map[tail]=EMPTY;
	tail = tmp;
}

void game_update() {
	game_draw();
	game_processInput();
	game_move();
	t=(t+1)%FRAME_DIV;
}

void print_score_text(int x, int y) {
	char scratch[8];
	char _text[8];
	char *nums = "0123456789";
	int i=1;
	int q=score;
	int r=0;
	while (q>0) {
		r = q%10;
		q = (int)(q/10);
		scratch[i++]=nums[r];
		if (i==8) q=0;
	}
	--i;
	int j=0;
	while (i) {
		_text[j++]=scratch[i--];
	}
	_text[j]=(char)0;
	text(_text,x,y);
}

void lost_update() {
	game_draw();
	*DRAW_COLORS=0x41;
	rect(4*8,7*8,12*8,6*8);
	*DRAW_COLORS=0x4;
	text("YOU LOSE!",5*8,8*8);
	text("SCORE:",5*8,9*8);
	print_score_text(12*8,9*8);
	text("RESTART: Z",5*8,11*8);
	uint8_t gamepad = *GAMEPAD1;
	if (gamepad & BUTTON_2) {
		game_init();
		state=STATE_GAME;
	}
}

void title_update() {
	*DRAW_COLORS = 0x2341;
	blit(logo,0,43,logoWidth,logoHeight,logoFlags);
	*DRAW_COLORS = 0x4;
	text("PRESS Z TO START",16,109);
	uint8_t gamepad = *GAMEPAD1;
	if (gamepad & BUTTON_1) {
		*DRAW_COLORS = 0x2;
		text("Z",64,109);
		*DRAW_COLORS = 0x4;
	}
	if (gamepad & BUTTON_2) {
		game_init();
		state = STATE_GAME;
	}
}

void update() {
	++why_doesnt_wasm4_just_let_me_use_time_ugh;
	if (state==STATE_TITLE) title_update();
	if (state==STATE_GAME) game_update();
	if (state==STATE_LOST) lost_update();
}
