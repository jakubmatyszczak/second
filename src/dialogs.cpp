#pragma once
#include "globals.cpp"
#include "raylib.h"

struct Dialog
{
	static const u32 nLines		  = 3;
	static const u32 nCharsInLine = 56;
	static const u32 nMaxChars	  = nCharsInLine * nLines + 1;

	u32		linesToShow					   = 0;
	char	line[nLines][nCharsInLine + 1] = {};
	char	talker[nCharsInLine + 1]	   = {};
	bool	busy;
	Vector2 portraitOffset;

	// returns true if busy
	bool input(bool progressToNext)
	{
        F.progressDialog = progressToNext;
		if (busy && progressToNext)
			busy = !busy;
		return busy;
	}
	void pushText(const char* text, const char* person, Vector2 portraitOffset = {})
	{
		strncpy(talker, person, nCharsInLine);
		this->portraitOffset = portraitOffset;

		u32 len		= strlen(text);
		linesToShow = math::divCeil(len, nCharsInLine);
		if (linesToShow > nLines)
			linesToShow = nLines;
		for (s32 i = 0; i < linesToShow; i++)
			strncpy(this->line[i], &text[32 * i], nCharsInLine - 1);
		busy = true;
	}
	void draw()
	{
		if (!busy)
			return;
		Font& font		  = C.fonts[C.FONT_ID_PIXEL];
		s32	  fontSize	  = G.screenY / 18;
		s32	  fontSpacing = fontSize / 10;
		v2f	  windowSize(G.screenX * 0.8f, G.screenY * 0.3f);
		v2f	  windowOrigin = {(G.screenX - windowSize.x) / 2.f, G.screenY - windowSize.y * 1.1f};
		v2f	  textOrigin   = windowOrigin + v2f(windowSize.x * 0.15f, windowSize.y * 0.35f);
		v2f	  talkerOrigin = windowOrigin + v2f(windowSize.x * 0.2f, windowSize.y * 0.1f);
		DrawRectangleV(windowOrigin.toVector2(), windowSize.toVector2(), BLACK_CLEAR);
		DrawTextPro(font,
					talker,
					talkerOrigin.toVector2(),
					{},
					0.f,
					fontSize * 1.5f,
					fontSpacing * 1.5f,
					RED);
		for (s32 i = 0; i < linesToShow; i++)
			DrawTextPro(font,
						line[i],
						(textOrigin + v2f(0, i * (fontSize + fontSpacing))).toVector2(),
						{},
						0.f,
						fontSize,
						fontSpacing,
						WHITE);
		DrawRectangleV((windowOrigin - v2f(132, 132)).toVector2(), {256 + 8, 256 + 8}, GRAY);
		DrawTextureRec(C.textures[C.TEX_PORTRAITS],
					   {portraitOffset.x, portraitOffset.y, 256, 256},
					   (windowOrigin - v2f(128, 128)).toVector2(),
					   WHITE);
	}
};
extern Dialog DIALOG;

