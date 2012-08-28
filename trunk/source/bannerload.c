#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bannerload.h"
#include "bitmap.h"

/***
	All of the code in bannerload.c is thanks in large part to the Dolphin Wii/GcN Emulator
	Project. These functions serve to decode the RGB5A3 format that GCN Banner/Icon Files use
	and create an array of BGR values, used later by the ShowBanner function.
	ShowBanner is a simple modification of ShowBMP.
	
	Files from Dolphin Project that were used as the basis for this code:
	BannerLoaderGC.cpp, ColorUtil.cpp, and TextureDecoder.cpp
	(http://dolphin-emu.googlecode.com/svn/trunk/)
***/

u8 convert5to8(u16 v) {
	return (v<<3)|(v>>2);
}

u8 convert3to8(u16 v) {
	return (v<<5)|(v<<2)|(v>>1);
}

u8 convert4to8(u16 v) {
	return (v<<4)|v;
}

u32 Decode5A3(u16 val) {
	static u32 bg_color = 0x00000000;

	int r, g, b, a;
	//use 0x8000 as a bit mask
	if ((val & 0x8000)) {
		r = convert5to8((val >> 10) & 0x1f);
		g = convert5to8((val >> 5) & 0x1f);
		b = convert5to8((val) & 0x1f);
		a = 0xFF;
	}
	else {
		a = convert3to8((val >> 12) & 0x7);
		r = (convert4to8((val >> 8) & 0xf) *a+(bg_color & 0xFF) * (255 - a)) / 255;
		g = (convert4to8((val >> 4) & 0xf)*a+((bg_color >> 8) & 0xFF) * (255 - a)) / 255;
		b = (convert4to8(val & 0xf) * a + ((bg_color >> 16) & 0xFF) * (255 - a)) / 255;
		a = 0xFF;
	}
	//pack into 32 bits and return (b,g,r order)
	return (a<<24)|(b<<16)|(g<<8)|r;
}

void bannerloadRGB(u16 *gamebanner) {
	int y, x, iy, ix;
	u32 RGBA;
	u16 *src; //we need this for pointer arithmetic
	u32 dst[CARD_BANNER_H*CARD_BANNER_W];
	u8 m_pImage[CARD_BANNER_H*CARD_BANNER_W*3];
	
	src = gamebanner;
	
	for (y = 0; y < CARD_BANNER_H; y += 4)
	{
		for (x = 0; x < CARD_BANNER_W; x += 4)
		{
			for (iy = 0; iy < 4; iy++, src+=4)
			{
				for (ix = 0; ix < 4; ix++)
				{
					//Decode for straight RGB
					RGBA = Decode5A3(src[ix]);
					dst[ (y + iy) * CARD_BANNER_W + (x + ix)] = RGBA;
				}
			}
		}
	}
	
	//Build the final array; 3 pixel values = 3*3072 or 9216 size bmp info
	for (y = 0; y < 3072; y++)
	{
		//b pixel
		m_pImage[y * 3 + 0] = (dst[y] & 0xFF0000) >> 16;
		//g pixel
		m_pImage[y * 3 + 1] = (dst[y] & 0x00FF00) >>  8;
		//r pixel
		m_pImage[y * 3 + 2] = (dst[y] & 0x0000FF) >>  0;
	}
	
	ShowBanner(m_pImage);
	
	return;
}

void bannerloadCI(u8 *gamebanner, u16* lookupdata) {
	int y, x, iy, ix;
	u8 *src; //we need this for pointer arithmetic
	u32 dst[CARD_BANNER_H*CARD_BANNER_W];
	u8 m_pImage[CARD_BANNER_H*CARD_BANNER_W*3];
	u8 temp;
	
	src = gamebanner;
	
	for (y = 0; y < CARD_BANNER_H; y += 4)
	{
		for (x = 0; x < CARD_BANNER_W; x += 8)
		{
			for (iy = 0; iy < 4; iy++, src+=8)
			{
				for (ix = 0; ix < 8; ix++)
				{
					//Decode for straight RGB
					temp = src[ix];
					dst[(y + iy) * CARD_BANNER_W + (x + ix)] = Decode5A3(lookupdata[temp]);
				}
			}
		}
	}
	
	//Build the final array; 3 pixel values = 3*3072 or 9216 size bmp info
	for (y = 0; y < CARD_BANNER_W*CARD_BANNER_H; y++)
	{
		//b pixel
		m_pImage[y * 3 + 0] = (dst[y] & 0xFF0000) >> 16;
		//g pixel
		m_pImage[y * 3 + 1] = (dst[y] & 0x00FF00) >>  8;
		//r pixel
		m_pImage[y * 3 + 2] = (dst[y] & 0x0000FF) >>  0;
	}
	
	ShowBanner(m_pImage);
	
	return;
}

void iconloadRGB(u16 *gameicon) {
	int y, x, iy, ix;
	u16 *src; //we need this for pointer arithmetic
	u32 dst[CARD_ICON_H*CARD_ICON_W];
	u8 m_pImage[CARD_ICON_H*CARD_ICON_W*3];
	
	src = gameicon;
	
	for (y = 0; y < CARD_ICON_H; y += 4)
	{
		for (x = 0; x < CARD_ICON_W; x += 4)
		{
			for (iy = 0; iy < 4; iy++, src+=4)
			{
				for (ix = 0; ix < 4; ix++)
				{
					//Decode for straight RGB
					dst[(y + iy) * CARD_ICON_W + (x + ix)] = Decode5A3(src[ix]);
				}
			}
		}
	}
	
	//Build the final array; 3 pixel values = 3*3072 or 9216 size bmp info
	for (y = 0; y < CARD_ICON_W*CARD_ICON_H; y++)
	{
		//b pixel
		m_pImage[y * 3 + 0] = (dst[y] & 0xFF0000) >> 16;
		//g pixel
		m_pImage[y * 3 + 1] = (dst[y] & 0x00FF00) >>  8;
		//r pixel
		m_pImage[y * 3 + 2] = (dst[y] & 0x0000FF) >>  0;
	}
	
	ShowIcon(m_pImage);
	
	return;
}

void iconloadCI(u8 *gameicon, u16* lookupdata) {
	int y, x, iy, ix;
	u8 *src; //we need this for pointer arithmetic
	u32 dst[CARD_ICON_H*CARD_ICON_W];
	u8 m_pImage[CARD_ICON_H*CARD_ICON_W*3];
	u8 temp;
	
	src = gameicon;
	
	for (y = 0; y < CARD_ICON_H; y += 4)
	{
		for (x = 0; x < CARD_ICON_W; x += 8)
		{
			for (iy = 0; iy < 4; iy++, src+=8)
			{
				for (ix = 0; ix < 8; ix++)
				{
					//Decode for straight RGB
					temp = src[ix];
					dst[(y + iy) * CARD_ICON_W + (x + ix)] = Decode5A3(lookupdata[temp]);
				}
			}
		}
	}
	
	//Build the final array; 3 pixel values = 3*3072 or 9216 size bmp info
	for (y = 0; y < CARD_ICON_W*CARD_ICON_H; y++)
	{
		//b pixel
		m_pImage[y * 3 + 0] = (dst[y] & 0xFF0000) >> 16;
		//g pixel
		m_pImage[y * 3 + 1] = (dst[y] & 0x00FF00) >>  8;
		//r pixel
		m_pImage[y * 3 + 2] = (dst[y] & 0x0000FF) >>  0;
	}
	
	ShowIcon(m_pImage);
	
	return;
}
