#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <iostream>
using namespace std;

namespace
{
	int randomVelocity(int maxSpeed)
	{
		int value = 0;
		while (value == 0)
		{
			value = (rand() % (maxSpeed * 2 + 1)) - maxSpeed;
		}
		return value;
	}

	bool overlapsRect(const point& p, int width, int height, int left, int top, int right, int bottom)
	{
		return p.x < right && p.x + width > left &&
			p.y < bottom && p.y + height > top;
	}

	void getBlockedWarehouseBounds(int& left, int& top, int& right, int& bottom)
	{
		const int fieldBottom = config.windHeight - config.statusBarHeight;
		const int warehouseWidth = config.warehouseWidth;
		const int warehouseHeight = config.warehouseHeight + 120;
		const int warehouseLeft = config.windWidth - warehouseWidth - 35;
		const int warehouseRight = warehouseLeft + warehouseWidth;
		const int warehouseBottom = fieldBottom - 25;
		const int warehouseTop = warehouseBottom - warehouseHeight;

		left = warehouseLeft - 12;
		top = warehouseTop - 70;
		right = warehouseRight + 12;
		bottom = warehouseBottom + 4;
	}
}

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
	lastProductTick = GetTickCount64();
	productIntervalMs = 0;
	productType = PRODUCT_NONE;
	foodEatenCounter = 0;
}

void Animal::moveInsideField(int maxSpeed, int changeInterval)
{
	setChangeCounter(getChangeCounter() + 1);

	if (getChangeCounter() % changeInterval == 0 || getDx() == 0 || getDy() == 0)
	{
		setDx(randomVelocity(maxSpeed));
		setDy(randomVelocity(maxSpeed));
	}

	point oldPoint = RefPoint;
	RefPoint.x += getDx();
	RefPoint.y += getDy();

	const int minX = config.fieldPadding;
	const int minY = (2 * config.toolBarHeight) + config.fieldPadding;
	const int maxX = config.windWidth - config.fieldPadding - width;
	const int maxY = config.windHeight - config.statusBarHeight - config.fieldPadding - height;

	if (RefPoint.x < minX)
	{
		RefPoint.x = minX;
		setDx(abs(getDx()));
	}

	if (RefPoint.y < minY)
	{
		RefPoint.y = minY;
		setDy(abs(getDy()));
	}

	if (RefPoint.x > maxX)
	{
		RefPoint.x = maxX;
		setDx(-abs(getDx()));
	}

	if (RefPoint.y > maxY)
	{
		RefPoint.y = maxY;
		setDy(-abs(getDy()));
	}

	int warehouseLeft, warehouseTop, warehouseRight, warehouseBottom;
	getBlockedWarehouseBounds(warehouseLeft, warehouseTop, warehouseRight, warehouseBottom);
	if (overlapsRect(RefPoint, width, height, warehouseLeft, warehouseTop, warehouseRight, warehouseBottom))
	{
		RefPoint = oldPoint;
		if (oldPoint.x + width <= warehouseLeft || oldPoint.x >= warehouseRight)
		{
			setDx(-getDx());
		}
		if (oldPoint.y + height <= warehouseTop || oldPoint.y >= warehouseBottom)
		{
			setDy(-getDy());
		}
		if (overlapsRect(RefPoint, width, height, warehouseLeft, warehouseTop, warehouseRight, warehouseBottom))
		{
			RefPoint.x = warehouseLeft - width - 1;
			setDx(-abs(getDx()));
		}
	}
}

void Animal::draw() const
{
	//draw image of this object
	window* pWind = pGame->getWind();
	pWind->DrawImage(sprite, RefPoint.x, RefPoint.y, width, height);
}



bool Animal::isProductReady()
{
	if (productIntervalMs <= 0)
	{
		return false;
	}

	unsigned long currentTick = GetTickCount64();
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

	unsigned long currentTick = GetTickCount64();
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
	moveInsideField(2, 45);

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
	moveInsideField(1, 70);

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
	int level = pGame->getLevel();
	int speedRange = 1 + ((level - 1) / 3);
	if (speedRange > 4)
	{
		speedRange = 4;
	}
	moveInsideField(speedRange, 65);

	draw();
}

Dog::Dog(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	birthTick = GetTickCount64();
	setDx(1);
	setDy(1);
}

void Dog::moveStep()
{
	moveInsideField(2, 40);
	draw();
	drawLifetimeCounter();
}

void Dog::moveToward(point target)
{
	point dogPoint = getRefPoint();
	const int dogCenterX = dogPoint.x + 25;
	const int dogCenterY = dogPoint.y + 25;
	const int targetCenterX = target.x + 25;
	const int targetCenterY = target.y + 25;

	if (targetCenterX > dogCenterX)
	{
		setDx(2);
	}
	else if (targetCenterX < dogCenterX)
	{
		setDx(-2);
	}
	else
	{
		setDx(0);
	}

	if (targetCenterY > dogCenterY)
	{
		setDy(2);
	}
	else if (targetCenterY < dogCenterY)
	{
		setDy(-2);
	}
	else
	{
		setDy(0);
	}

	point oldPoint = RefPoint;
	RefPoint.x += getDx();
	RefPoint.y += getDy();

	if (RefPoint.x < config.fieldPadding ||
		RefPoint.y < (2 * config.toolBarHeight) + config.fieldPadding ||
		RefPoint.x > config.windWidth - config.fieldPadding - width ||
		RefPoint.y > config.windHeight - config.statusBarHeight - config.fieldPadding - height)
	{
		RefPoint = oldPoint;
	}

	draw();
	drawLifetimeCounter();
}

bool Dog::isExpired() const
{
	return GetTickCount64() - birthTick >= lifetimeMs;
}

int Dog::getRemainingLifetimeSeconds() const
{
	unsigned long elapsedMs = GetTickCount64() - birthTick;
	if (elapsedMs >= lifetimeMs)
	{
		return 0;
	}

	unsigned long remainingMs = lifetimeMs - elapsedMs;
	return static_cast<int>((remainingMs + 999) / 1000);
}

void Dog::drawLifetimeCounter() const
{
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 1);
	pWind->SetFont(12, BOLD, BY_NAME, "Arial");
	pWind->DrawString(RefPoint.x + 17, RefPoint.y - 14, to_string(getRemainingLifetimeSeconds()));
}

Sheep::Sheep(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	setProductIntervalMs(10000);
	setProductType(PRODUCT_WOOL);
}
void Sheep::moveStep()
{
	moveInsideField(1, 60);

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
	moveInsideField(2, 55);

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
	moveInsideField(3, 40);

	draw();
	drawCounter();
}

