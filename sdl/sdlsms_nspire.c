
/*
    Copyright (c) 2002 Gregory Montoir

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include "n2DLib.h"
#include "defines.h"
#include "shared.h"
#include "config.h"
#include "saves.h"


/* VIDEO */


#define SMS_SCREEN_WIDTH   256
#define SMS_SCREEN_HEIGHT  192
#define GG_SCREEN_WIDTH    160
#define GG_SCREEN_HEIGHT   144

static int sdl_video_init()
{
	unsigned short screen_width, screen_height;
	screen_width  = (IS_GG) ? GG_SCREEN_WIDTH  : SMS_SCREEN_WIDTH;
	screen_height = (IS_GG) ? GG_SCREEN_HEIGHT : SMS_SCREEN_HEIGHT;

	initBuffering();
	clearBufferB();
	updateScreen();

	memset(&bitmap, 0, sizeof(t_bitmap));
	bitmap.width  = 320;
	bitmap.height = 240;
	bitmap.depth  = 16;
	bitmap.pitch  = 320*2;
	bitmap.data   = (unsigned short *)BUFF_BASE_ADDRESS;
  
	return 1;
}

static void sdl_video_close()
{
	deinitBuffering();
}


void sdl_main_close()
{
  sdl_video_close();
  /* shutdown the virtual console emulation */
  /*save_sram(cfg.game_name);*/
  system_shutdown();
  exit(0);
}


static void sdl_video_update()
{
    bitmap.data = (unsigned short *)BUFF_BASE_ADDRESS + 8028;
    sms_frame(0);
    updateScreen();
}


static int sdl_controls_update_input(void)
{
	input.system &= (IS_GG) ? ~INPUT_START : ~INPUT_PAUSE;
	input.pad[0] &= ~INPUT_UP;
	input.pad[0] &= ~INPUT_DOWN;
	input.pad[0] &= ~INPUT_LEFT;
	input.pad[0] &= ~INPUT_RIGHT;
	input.pad[0] &= ~INPUT_BUTTON1;
	input.pad[0] &= ~INPUT_BUTTON2;
	
	if(isKeyPressed(KEY_NSPIRE_ESC)) 
	{
		sdl_main_close();
	}
  
	/*if(isKeyPressed(KEY_NSPIRE_L)) 
	{
		load_state(cfg.game_name, 0);
	}
	else if(isKeyPressed(KEY_NSPIRE_S)) 
	{
		save_state(cfg.game_name, 0);
	}*/
  
	if(isKeyPressed(KEY_NSPIRE_MENU)) 
	{
		input.system |= (IS_GG) ? INPUT_START : INPUT_PAUSE;
	}
  
	if (isKeyPressed(KEY_NSPIRE_UPRIGHT))
	{
		input.pad[0] |= INPUT_RIGHT;  
		input.pad[0] |= INPUT_UP;
	}
	else if (isKeyPressed(KEY_NSPIRE_LEFTUP))
	{
		input.pad[0] |= INPUT_LEFT;  
		input.pad[0] |= INPUT_UP;
	}
	
	if (isKeyPressed(KEY_NSPIRE_RIGHTDOWN))
	{
		input.pad[0] |= INPUT_RIGHT;  
		input.pad[0] |= INPUT_DOWN;
	}
	else if (isKeyPressed(KEY_NSPIRE_DOWNLEFT))
	{
		input.pad[0] |= INPUT_LEFT;  
		input.pad[0] |= INPUT_DOWN;
	}
	
	if(isKeyPressed(KEY_NSPIRE_UP)) 
	{
		input.pad[0] |= INPUT_UP;
	}
  
	if(isKeyPressed(KEY_NSPIRE_DOWN)) 
	{
		input.pad[0] |= INPUT_DOWN;
	}
      
	if(isKeyPressed(KEY_NSPIRE_LEFT)) 
	{
		input.pad[0] |= INPUT_LEFT;
	}
  
	if(isKeyPressed(KEY_NSPIRE_RIGHT)) 
	{
		input.pad[0] |= INPUT_RIGHT;
	}
  
	if(isKeyPressed(KEY_NSPIRE_CTRL)) 
	{
		input.pad[0] |= INPUT_BUTTON1;
	}
  
	if(isKeyPressed(KEY_NSPIRE_SHIFT)) 
	{
		input.pad[0] |= INPUT_BUTTON2;
	}
    
  return 1;
}

int sdl_main_init()
{
  /* set up the virtual console emulation */
  system_init(cfg.sound_rate);
  /*if(cfg.use_sram)
    load_sram(cfg.game_name);*/

  if(!sdl_video_init())
    return 0;

   return 1;
}


void sdl_main_loop()
{
  
  while(1) {
    sdl_controls_update_input();
    sdl_video_update();
  }
}


