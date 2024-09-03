#include "commons.cpp"
#include "../globals.cpp"

struct Input
{
	enum class Controller
	{
		KEYBOARD,
		GAMEPAD,
		MOUSE,
	};
	Controller controller;
	v2f		   previousHeading = {};

	v2f getHeading(v2f point)
	{
		if (v2f(GetMouseDelta()).getLength() > 0)
			controller = Controller::MOUSE;
		else
			controller = Controller::KEYBOARD;
		v2f heading = previousHeading;
		switch (controller)
		{
			case Controller::KEYBOARD:
			case Controller::GAMEPAD:
			{
				bool up	   = IsKeyPressed(KEY_W);
				bool down  = IsKeyPressed(KEY_S);
				bool left  = IsKeyPressed(KEY_A);
				bool right = IsKeyPressed(KEY_D);
				if (up | down | left | right)
					heading += {(f32)(right - left), (f32)(down - up)};
				break;
			}
			case Controller::MOUSE:
			{
				heading = F.mousePosWorld - point;
				if (heading.getLength() < G.tileSize)
					heading = 0;
				break;
			}
		}
		heading			= heading.norm().round();
		previousHeading = heading;
		return heading;
	}
	bool getAction()
	{
		if (IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
			return true;
		return false;
	}
	bool getMove()
	{
		if (IsKeyPressed(KEY_G) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			return true;
		return false;
	}
	bool getHit()
	{
		if (IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			return true;
		return false;
	}
};
