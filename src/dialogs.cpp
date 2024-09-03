#pragma once
#include "globals.cpp"
#include "raylib.h"

struct DialogBox
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
struct Dialog
{
	enum SpeakerId
	{
		NARRATOR = 0,
		OLDMAN,
	};
	static const u32 maxLineLen = 128;
	struct Line
	{
		SpeakerId speaker;
		char	  text[maxLineLen] = {};
		s32		  nextLine		   = 0;
	};
	s32	 nLines	   = 0;
	Line lines[32] = {};
	s32	 current   = -1;
	s32	 last	   = 0;

	void addLine(SpeakerId speaker, const char* text, s32 nextLine = -1)
	{
		current				   = 0;
		lines[nLines].speaker  = speaker;
		lines[nLines].nextLine = nextLine < 0 ? nLines + 1 : nextLine;
		strncpy(lines[nLines++].text, text, maxLineLen - 1);
		last = nLines;
	}
	bool update(bool nextLine);
};
struct Narrative
{
	struct Speaker
	{
		Dialog::SpeakerId id;
		char			  name[32];
		Vector2			  poritaitOffset;
	};
	Speaker speakers[32] = {};
	Dialog	dialogs[32]	 = {};
	s32		active		 = -1;
	void	init()
	{
		speakers[Dialog::NARRATOR] = {Dialog::NARRATOR, "Narrator", {0, 0}};
		speakers[Dialog::OLDMAN]   = {Dialog::OLDMAN, "Old man", {256, 0}};

		dialogs[0].addLine(Dialog::NARRATOR, "Hello Sailor!");
		dialogs[0].addLine(Dialog::NARRATOR, "Whats up?");
		dialogs[0].addLine(Dialog::NARRATOR, "...");
		dialogs[0].addLine(Dialog::NARRATOR, "Maybe try talking to the dude over here.");

		dialogs[1].addLine(Dialog::NARRATOR, "An old man stands here and enjoys his day...");
		dialogs[1].addLine(Dialog::OLDMAN, "Hello sir, how ya doin?");
	}
	bool start(s32 dialogId)
	{
		if (active >= 0)
			return false;
		active = dialogId;
		return true;
	}
	bool update()
	{
		if (active < 0)
			return false;
		if (dialogs[active].update(F.progressDialog))
			active = -1;
		return true;
	}
};
extern DialogBox DIALOGBOX;
extern Narrative NARRATIVE;

// Returns true if dialog ended
bool Dialog::update(bool progressDialog)
{
	if (current >= nLines)
		return true;
	if (progressDialog || current == 0)
	{
		Narrative::Speaker& s = NARRATIVE.speakers[lines[current].speaker];
		DIALOGBOX.pushText(lines[current].text, s.name, s.poritaitOffset);
		current++;
		return false;
	}
	return false;
}
