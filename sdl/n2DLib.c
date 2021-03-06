#include "n2DLib.h"
#include "n2DLib_font.h"

#ifdef __cplusplus
extern "C" {
#endif

/*             *
 *  Buffering  *
 *             */

unsigned short *BUFF_BASE_ADDRESS, *ALT_SCREEN_BASE_ADDRESS, *INV_BUFF, *temp;
void *SCREEN_BACKUP;
int swapped = 0;

void initBuffering()
{
	BUFF_BASE_ADDRESS = (unsigned short*)malloc(BUFF_BYTES_SIZE);
	if(!BUFF_BASE_ADDRESS) exit(0);
	
	SCREEN_BACKUP = *(void**)0xC0000010;
	
	// Handle monochrome screens-specific shit
	if(is_classic)
		*(int32_t*)(0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x08;
	
	ALT_SCREEN_BASE_ADDRESS = (unsigned short*)malloc(BUFF_BYTES_SIZE + 8);
	if(!ALT_SCREEN_BASE_ADDRESS)
	{
		free(BUFF_BASE_ADDRESS);
		*((int32_t*)0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x04;
		*(void**)0xC0000010 = SCREEN_BACKUP;
		exit(0);
	}
	
	INV_BUFF = (unsigned short*)malloc(BUFF_BYTES_SIZE);
	if(!INV_BUFF)
	{
		free(ALT_SCREEN_BASE_ADDRESS);
		free(BUFF_BASE_ADDRESS);
		*((int32_t*)0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x04;
		*(void**)0xC0000010 = SCREEN_BACKUP;
		exit(0);
	}
	
	*(void**)0xC0000010 = ALT_SCREEN_BASE_ADDRESS;
}

void updateScreen()
{
	unsigned int *dest, *src, i, c;
	// I use different methods for refreshing the screen for GS and color screens because according to my tests, the fastest for one isn't the fastest for the other
	if(has_colors)
	{
		dest = (unsigned int*)ALT_SCREEN_BASE_ADDRESS;
		src = (unsigned int*)BUFF_BASE_ADDRESS;
		for(i = 0; i < 160 * 240; i++)
			*dest++ = *src++;
	}
	else
	{
		dest = (unsigned int*)INV_BUFF;
		src = (unsigned int*)BUFF_BASE_ADDRESS;
		for(i = 0; i < 160 * 240; i++)
		{
			c = *src++;
			c = ~c;
			// c holds two 16-bits colors, decompose them while keeping them that way
			*dest++ = ((c & 0x1f) + (((c >> 5) & 0x3f) >> 1) + ((c >> 11) & 0x1f)) / 3
				+ ((((c >> 16) & 0x1f) + (((c >> 21) & 0x3f) >> 1) + ((c >> 27) & 0x1f)) / 3 << 16);
			
		}
		
		temp = *(void**)0xC0000010;
		*(void**)0xC0000010 = INV_BUFF;
		INV_BUFF = temp;
		swapped = !swapped;
	}
}

void deinitBuffering()
{
	// Handle monochrome screens-specific shit again
	if(is_classic)
		*((int32_t*)0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x04;
	*(void**)(0xC0000010) = SCREEN_BACKUP;
	if(swapped)
	{
		temp = *(void**)0xC0000010;
		*(void**)0xC0000010 = INV_BUFF;
		INV_BUFF = temp;
	}
	free(INV_BUFF);
	free(ALT_SCREEN_BASE_ADDRESS);
	free(BUFF_BASE_ADDRESS);
}

/*            *
 *  Graphics  *
 *            */

void clearBufferB()
{
	int i;
	for(i = 0; i < 160 * 240; i++)
		((unsigned int*)BUFF_BASE_ADDRESS)[i] = 0;
}


inline void setPixelUnsafe(unsigned int x, unsigned int y, unsigned short c)
{
	*((unsigned short*)BUFF_BASE_ADDRESS + x + y * 320) = c;
}


inline void setPixel(unsigned int x, unsigned int y, unsigned short c)
{
	if(x < 320 && y < 240)
		*((unsigned short*)BUFF_BASE_ADDRESS + x + y * 320) = c;
}


void fillRect(int x, int y, int w, int h, unsigned short c)
{
	unsigned int _x = max(x, 0), _y = max(y, 0), _w = min(320 - _x, w - _x + x), _h = min(240 - _y, h - _y + y), i, j;
	if(_x < 320 && _y < 240)
	{
		for(j = _y; j < _y + _h; j++)
			for(i = _x; i < _x + _w; i++)
				setPixelUnsafe(i, j, c);
	}
}

/*            *
 *  Geometry  *
 *            */
 
void drawLine(int x1, int y1, int x2, int y2, unsigned short c)
{
	int dx = abs(x2-x1);
	int dy = abs(y2-y1);
	int sx = (x1 < x2)?1:-1;
	int sy = (y1 < y2)?1:-1;
	int err = dx-dy;
	int e2;

	while (!(x1 == x2 && y1 == y2))
	{
		setPixel(x1,y1,c);
		e2 = 2*err;
		if (e2 > -dy)
		{		 
			err = err - dy;
			x1 = x1 + sx;
		}
		if (e2 < dx)
		{		 
			err = err + dx;
			y1 = y1 + sy;
		}
	}
}


/*        *
 *  Text  *
 *        */

int isOutlinePixel(unsigned char* charfont, int x, int y)
{
	int xis0 = !x, xis7 = x == 7, yis0 = !y, yis7 = y == 7;
	
	if(xis0)
	{
		if(yis0)
		{
			return !(*charfont & 0x80) && ((*charfont & 0x40) || (charfont[1] & 0x80) || (charfont[1] & 0x40));
		}
		else if(yis7)
		{
			return !(charfont[7] & 0x80) && ((charfont[7] & 0x40) || (charfont[6] & 0x80) || (charfont[6] & 0x40));
		}
		else
		{
			return !(charfont[y] & 0x80) && (
				(charfont[y - 1] & 0x80) || (charfont[y - 1] & 0x40) ||
				(charfont[y] & 0x40) ||
				(charfont[y + 1] & 0x80) || (charfont[y + 1] & 0x40));
		}
	}
	else if(xis7)
	{
		if(yis0)
		{
			return !(*charfont & 0x01) && ((*charfont & 0x02) || (charfont[1] & 0x01) || (charfont[1] & 0x02));
		}
		else if(yis7)
		{
			return !(charfont[7] & 0x01) && ((charfont[7] & 0x02) || (charfont[6] & 0x01) || (charfont[6] & 0x02));
		}
		else
		{
			return !(charfont[y] & 0x01) && (
				(charfont[y - 1] & 0x01) || (charfont[y - 1] & 0x02) ||
				(charfont[y] & 0x02) ||
				(charfont[y + 1] & 0x01) || (charfont[y + 1] & 0x02));
		}
	}
	else
	{
		char b = 1 << (7 - x);
		if(yis0)
		{
			return !(*charfont & b) && (
				(*charfont & (b << 1)) || (*charfont & (b >> 1)) ||
				(charfont[1] & (b << 1)) || (charfont[1] & b) || (charfont[1] & (b >> 1)));
		}
		else if(yis7)
		{
			return !(charfont[7] & b) && (
				(charfont[7] & (b << 1)) || (charfont[7] & (b >> 1)) ||
				(charfont[6] & (b << 1)) || (charfont[6] & b) || (charfont[6] & (b >> 1)));
		}
		else
		{
			return !(charfont[y] & b) && (
				(charfont[y] & (b << 1)) || (charfont[y] & (b >> 1)) ||
				(charfont[y - 1] & (b << 1)) || (charfont[y - 1] & b) || (charfont[y - 1] & (b >> 1)) ||
				(charfont[y + 1] & (b << 1)) || (charfont[y + 1] & b) || (charfont[y + 1] & (b >> 1)));
		}
	}
}

void drawChar(int *x, int *y, int margin, char ch, unsigned short fc, unsigned short olc)
{
	int i, j;
	unsigned char *charSprite;
	if(ch == '\n')
	{
		*x = margin;
		*y += 8;
	}
	else if(*y < 239)
	{
		charSprite = ch * 8 + n2DLib_font;
		// Draw charSprite as monochrome 8*8 image using given color
		for(i = 0; i < 8; i++)
		{
			for(j = 7; j >= 0; j--)
			{
				if((charSprite[i] >> j) & 1)
					setPixel(*x + (7 - j), *y + i, fc);
				else if(isOutlinePixel(charSprite, 7 - j, i))
					setPixel(*x + (7 - j), *y + i, olc);
			}
		}
		*x += 8;
	}
}

void drawString(int *x, int *y, int _x, const char *str, unsigned short fc, unsigned short olc)
{
	int i, max = strlen(str) + 1;
	for(i = 0; i < max; i++)
		drawChar(x, y, _x, str[i], fc, olc);
}


#ifdef __cplusplus
}
#endif
