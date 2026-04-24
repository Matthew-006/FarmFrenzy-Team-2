#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <iostream>
using namespace std;

Animal::Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
	curr_pos = r_point;
	curr_vel.x = 1;
	curr_vel.y = 1;
	dx = (rand() % 5) - 2;
	dy = (rand() % 5) - 2;
	changeCounter = 0;

}

void Animal::draw() const
{
	//draw image of this object
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
}

Chick::Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{}

void Chick::moveStep()
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % 20 == 0)
	{
		setDx((rand() % 7) - 3);
		setDy((rand() % 7) - 3);
	}

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	if (RefPoint.x < 0)
	{
		RefPoint.x = 0;
		setDx(-getDx());
	}

	if (RefPoint.y < 2 * config.toolBarHeight)
	{
		RefPoint.y = 2 * config.toolBarHeight;
		setDy(-getDy());
	}

	if (RefPoint.x > config.windWidth - width)
	{
		RefPoint.x = config.windWidth - width;
		setDx(-getDx());
	}

	if (RefPoint.y > config.windHeight - config.statusBarHeight - height)
	{
		RefPoint.y = config.windHeight - config.statusBarHeight - height;
		setDy(-getDy());
	}

	draw();
}

Cow::Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{}

void Cow::moveStep()
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % 20 == 0)
	{
		setDx((rand() % 7) - 3);
		setDy((rand() % 7) - 3);
	}

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	if (RefPoint.x < 0)
	{
		RefPoint.x = 0;
		setDx(-getDx());
	}

	if (RefPoint.y < 2 * config.toolBarHeight)
	{
		RefPoint.y = 2 * config.toolBarHeight;
		setDy(-getDy());
	}

	if (RefPoint.x > config.windWidth - width)
	{
		RefPoint.x = config.windWidth - width;
		setDx(-getDx());
	}

	if (RefPoint.y > config.windHeight - config.statusBarHeight - height)
	{
		RefPoint.y = config.windHeight - config.statusBarHeight - height;
		setDy(-getDy());
	}

	draw();

}

Wolf::Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{}

void Wolf::moveStep()
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % 20 == 0)
	{
		setDx((rand() % 7) - 3);
		setDy((rand() % 7) - 3);
	}

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	if (RefPoint.x < 0)
	{
		RefPoint.x = 0;
		setDx(-getDx());
	}

	if (RefPoint.y < 2 * config.toolBarHeight)
	{
		RefPoint.y = 2 * config.toolBarHeight;
		setDy(-getDy());
	}

	if (RefPoint.x > config.windWidth - width)
	{
		RefPoint.x = config.windWidth - width;
		setDx(-getDx());
	}

	if (RefPoint.y > config.windHeight - config.statusBarHeight - height)
	{
		RefPoint.y = config.windHeight - config.statusBarHeight - height;
		setDy(-getDy());
	}

	draw();
}

Sheep::Sheep(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}
void Sheep::moveStep()
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % 20 == 0)
	{
		setDx((rand() % 7) - 3);
		setDy((rand() % 7) - 3);
	}

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	if (RefPoint.x < 0)
	{
		RefPoint.x = 0;
		setDx(-getDx());
	}

	if (RefPoint.y < 2 * config.toolBarHeight)
	{
		RefPoint.y = 2 * config.toolBarHeight;
		setDy(-getDy());
	}

	if (RefPoint.x > config.windWidth - width)
	{
		RefPoint.x = config.windWidth - width;
		setDx(-getDx());
	}

	if (RefPoint.y > config.windHeight - config.statusBarHeight - height)
	{
		RefPoint.y = config.windHeight - config.statusBarHeight - height;
		setDy(-getDy());
	}

	draw();

}

Goat::Goat(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Goat::moveStep()
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % 20 == 0)
	{
		setDx((rand() % 7) - 3);
		setDy((rand() % 7) - 3);
	}

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	if (RefPoint.x < 0)
	{
		RefPoint.x = 0;
		setDx(-getDx());
	}

	if (RefPoint.y < 2 * config.toolBarHeight)
	{
		RefPoint.y = 2 * config.toolBarHeight;
		setDy(-getDy());
	}

	if (RefPoint.x > config.windWidth - width)
	{
		RefPoint.x = config.windWidth - width;
		setDx(-getDx());
	}

	if (RefPoint.y > config.windHeight - config.statusBarHeight - height)
	{
		RefPoint.y = config.windHeight - config.statusBarHeight - height;
		setDy(-getDy());
	}

	draw();

}

Water::Water(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Water::moveStep()
{
	draw();

}

