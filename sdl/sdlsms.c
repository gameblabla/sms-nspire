
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
#include <SDL.h>
#include "defines.h"
#include "shared.h"
#include "config.h"
#include "saves.h"


/* VIDEO */


#define SMS_SCREEN_WIDTH   256
#define SMS_SCREEN_HEIGHT  192
#define GG_SCREEN_WIDTH    160
#define GG_SCREEN_HEIGHT   144

static SDL_Surface *sdl_screen = NULL;

static int sdl_video_init()
{
  unsigned short screen_width, screen_height;
  screen_width  = (IS_GG) ? GG_SCREEN_WIDTH  : SMS_SCREEN_WIDTH;
  screen_height = (IS_GG) ? GG_SCREEN_HEIGHT : SMS_SCREEN_HEIGHT;

  SDL_Init(SDL_INIT_VIDEO);

  sdl_screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_SWSURFACE);

  memset(&bitmap, 0, sizeof(t_bitmap));
  bitmap.width  = sdl_screen->w;
  bitmap.height = sdl_screen->h;
  bitmap.depth  = 16;
  bitmap.pitch  = sdl_screen->pitch;
  bitmap.data   = (unsigned char *)sdl_screen->pixels;
  

  return 1;
}

static void sdl_video_close()
{
  if(sdl_screen) SDL_FreeSurface(sdl_screen);
}


static void sdl_video_take_screenshot()
{
	int status;
	char ssname[0x100];

	strcpy(ssname, cfg.game_name);
	snprintf(strrchr(ssname, '.'), sizeof(ssname), "./-%03d.bmp.tns", cfg.current_screenshot);
	++cfg.current_screenshot;
	SDL_LockSurface(sdl_screen);
	status = SDL_SaveBMP(sdl_screen, ssname);
	SDL_UnlockSurface(sdl_screen);
	if(status == 0)
		printf("[INFO] Screenshot written to '%s'.\n", ssname);
}


static void sdl_video_prepare_update()
{
	SDL_LockSurface(sdl_screen);
	bitmap.data = (unsigned char*)sdl_screen->pixels;
}



static void sdl_video_blit_center(void)
{
	SDL_Flip(sdl_screen);
}


static void sdl_video_finish_update()
{
	SDL_UnlockSurface(sdl_screen);
    sdl_video_blit_center();
}


static void sdl_video_update()
{
    sdl_video_prepare_update();
    sms_frame(0);
    sdl_video_finish_update();
}



/* CONTROLS */
static void sdl_controls_init()
{
  cfg.pad[0].up = SDLK_UP;
  cfg.pad[0].down = SDLK_DOWN;
  cfg.pad[0].left = SDLK_LEFT;
  cfg.pad[0].right = SDLK_RIGHT;
  cfg.pad[0].b1 = SDLK_c;
  cfg.pad[0].b2 = SDLK_v;
  cfg.pad[0].start = SDLK_RETURN;

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}


static int sdl_controls_update_input(SDLKey k, int p)
{
  if(!p) {
	  switch(k) {
	  case SDLK_ESCAPE:
		  return 0;
	  case SDLK_F1:
		  sdl_video_take_screenshot();
		  break;
	  case SDLK_F2:
		  if(save_state(cfg.game_name, cfg.state_slot))
  		  printf("[INFO] Saved state to slot #%d.\n", cfg.state_slot);
		  break;
	  case SDLK_F3:
		  if(load_state(cfg.game_name, cfg.state_slot))
		    printf("[INFO] Loaded state from slot #%d.\n", cfg.state_slot);
		  break;
    case SDLK_F4:
    case SDLK_F5:
    case SDLK_F6:
    case SDLK_F7:
    case SDLK_F8:
      cfg.frame_skip = k - SDLK_F4 + 1;
      if(cfg.frame_skip == 1)
        printf("[INFO] Frame skip disabled.\n");
      else
        printf("[INFO] Frame skip set to %d.\n", cfg.frame_skip - 1);
      break;
	  case SDLK_0:
    case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
		case SDLK_9:
	    cfg.state_slot = k - SDLK_0;
		  printf("[INFO] Slot changed to #%d.\n", cfg.state_slot);
      break;
    }
  }
  if(k == cfg.pad[0].start) {
    if(p) input.system |= (IS_GG) ? INPUT_START : INPUT_PAUSE;
    else  input.system &= (IS_GG) ? ~INPUT_START : ~INPUT_PAUSE;
  }
  else if(k == cfg.pad[0].up) {
    if(p) input.pad[0] |= INPUT_UP;
    else  input.pad[0] &= ~INPUT_UP;
  }
  else if(k == cfg.pad[0].down) {
    if(p) input.pad[0] |= INPUT_DOWN;
    else  input.pad[0] &= ~INPUT_DOWN;
  }
  else if(k == cfg.pad[0].left) {
    if(p) input.pad[0] |= INPUT_LEFT;
    else  input.pad[0] &= ~INPUT_LEFT;
  }
  else if(k == cfg.pad[0].right) {
    if(p) input.pad[0] |= INPUT_RIGHT;
    else  input.pad[0] &= ~INPUT_RIGHT;
  }
  else if(k == cfg.pad[0].b1) {
    if(p) input.pad[0] |= INPUT_BUTTON1;
    else  input.pad[0] &= ~INPUT_BUTTON1;
  }
  else if(k == cfg.pad[0].b2) {
    if(p) input.pad[0] |= INPUT_BUTTON2;
    else  input.pad[0] &= ~INPUT_BUTTON2;
  }
  else if(k == SDLK_TAB) {
    if(p) input.system |= INPUT_HARD_RESET;
    else  input.system &= ~INPUT_HARD_RESET;
  }
  return 1;
}

int sdl_main_init()
{
  printf("Initializing virtual console emulation... ");
  /* set up the virtual console emulation */
  system_init(cfg.sound_rate);
  /*if(cfg.use_sram)
    load_sram(cfg.game_name);*/
  printf("Ok.\n");

  printf("Initializing SDL... ");
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("ERROR: %s.\n", SDL_GetError());
    return 0;
  }
  printf("Ok.\n");

  printf("Initializing SDL VIDEO SUBSYSTEM... ");
  if(!sdl_video_init())
    return 0;
  printf("Ok.\n");

  printf("Initializing SDL CONTROLS SUBSYSTEM... ");
  sdl_controls_init();
  printf("Ok.\n");
  
   return 1;
}


void sdl_main_loop()
{
  printf("Starting emulation...\n");
  
  while(1) {
    /* pump SDL events */
    SDL_Event event;
    if(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_KEYUP:
        if(!sdl_controls_update_input(event.key.keysym.sym, 0))
          return;
        break;
      case SDL_KEYDOWN:
        if(!sdl_controls_update_input(event.key.keysym.sym, 1))
          return;
        break;
      case SDL_QUIT:
        return;
      }
    }

    sdl_video_update();
  }
}



void sdl_main_close()
{
  printf("Stopping emulation...\n");

  sdl_video_close();
  SDL_Quit();
  /* shutdown the virtual console emulation */
  save_sram(cfg.game_name);
  system_shutdown();
}
