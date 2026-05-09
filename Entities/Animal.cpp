#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <iostream>
using namespace std;

Animal::Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
	if (!image_path.empty())
	{
		sprite.Open(image_path);
	}
	curr_pos = r_point;
	curr_vel.x = 1;
	curr_vel.y = 1;
	dx = (rand() % 5) - 2;
	dy = (rand() % 5) - 2;
	changeCounter = 0;
	lastProductTick = GetTickCount();
	productIntervalMs = 0;
	productType = PRODUCT_NONE;
	foodEatenCounter = 0;
}

void Animal::draw() const
{
	//draw image of this object
	window* pWind = pGame->getWind();
	pWind->DrawImage(sprite, RefPoint.x, RefPoint.y, width, height);
	pWind->SetPen(BLACK); 
	pWind->DrawString(RefPoint.x, RefPoint.y - 20, to_string(foodEatenCounter));
}



bool Animal::isProductReady()
{
	if (productIntervalMs <= 0)
	{
		return false;
	}

	unsigned long currentTick = GetTickCount();
	if (currentTick - lastProductTick < static_cast<unsigned long>(productIntervalMs))
	{
		return false;
	}

	lastProductTick = currentTick;
	return true;
}

int Animal::getRemainingProductSeconds() const
{
	if (productIntervalMs <= 0)
	{
		return -1;
	}

	unsigned long currentTick = GetTickCount();
	unsigned long elapsedMs = currentTick - lastProductTick;
	if (elapsedMs >= static_cast<unsigned long>(productIntervalMs))
	{
		return 0;
	}

	unsigned long remainingMs = static_cast<unsigned long>(productIntervalMs) - elapsedMs;
	return static_cast<int>((remainingMs + 999) / 1000);
}

void Animal::drawCounter() const
{
	int remainingSeconds = getRemainingProductSeconds();
	if (remainingSeconds < 0)
	{
		return;
	}

	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 1);
	pWind->SetFont(12, BOLD, BY_NAME, "Arial");
	pWind->DrawString(RefPoint.x + 8, RefPoint.y - 14, to_string(remainingSeconds));
}

void Animal::produceProduct()
{
	point productPoint;
	productPoint.x = RefPoint.x + (width / 2);
	productPoint.y = RefPoint.y + height;

	if (productType == PRODUCT_EGG)
	{
		pGame->addEgg(productPoint);
	}
	else if (productType == PRODUCT_MILK)
	{
		pGame->addMilk(productPoint);
	}
	else if (productType == PRODUCT_WOOL)
	{
		pGame->addWool(productPoint);
	}
}

Chick::Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	setProductIntervalMs(10000);
	setProductType(PRODUCT_EGG);
}

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

	if (isProductReady())
	{
		produceProduct();
	}

	draw();
	drawCounter();
}

Cow::Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	setProductIntervalMs(10000);
	setProductType(PRODUCT_MILK);
}

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

	if (isProductReady())
	{
		produceProduct();
	}

	draw();
	drawCounter();

}

Wolf::Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	disapclicks = 5;
}

void Wolf::moveStep()
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % 20 == 0)
	{
		int level = pGame->getLevel();
		int speedRange = (level / 2) + 2;
		setDx((rand() % (speedRange * 2 + 1)) - speedRange);
		setDy((rand() % (speedRange * 2 + 1)) - speedRange);
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
	setProductIntervalMs(10000);
	setProductType(PRODUCT_WOOL);
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

	if (isProductReady())
	{
		produceProduct();
	}

	draw();
	drawCounter();

}

Goat::Goat(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	setProductIntervalMs(10000);
	setProductType(PRODUCT_MILK);
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

	if (isProductReady())
	{
		produceProduct();
	}

	draw();
	drawCounter();

}

Water::Water(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Water::moveStep()
{
	draw();

}
Duck::Duck(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{

}

void Duck::moveStep()
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
	drawCounter();
}

