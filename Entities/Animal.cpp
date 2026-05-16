#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <fstream>
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

	string resolveAssetPath(const string& path)
	{
		ifstream localFile(path.c_str());
		if (localFile)
		{
			return path;
		}

		const string parentPath = "..\\" + path;
		ifstream parentFile(parentPath.c_str());
		if (parentFile)
		{
			return parentPath;
		}

		return path;
	}

	int stepToward(int distance, int maxStep)
	{
		if (distance > maxStep)
		{
			return maxStep;
		}

		if (distance < -maxStep)
		{
			return -maxStep;
		}

		return distance;
	}
}

Animal::Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path, bool is_gold) : Drawable(r_pGame, r_point, r_width, r_height)
{
	happiness = maxHappiness; 
	lastHappinessTick = GetTickCount64();
	image_path = img_path;
	imageLoaded = false;
	isGolden = is_gold;
	if (!image_path.empty())
	{
		image_path = resolveAssetPath(image_path);
		try
		{
			sprite.Open(image_path);
			imageLoaded = true;
		}
		catch (...)
		{
			imageLoaded = false;
		}
	}
	curr_pos = r_point;
	curr_vel.x = 1;
	curr_vel.y = 1;
	dx = (rand() % 5) - 2;
	dy = (rand() % 5) - 2;
	changeCounter = 0;
	movementCounter = 0;
	lastProductTick = GetTickCount64();
	productIntervalMs = 0;
	productType = PRODUCT_NONE;
	foodEatenCounter = 0;
	productProducedThisStep = false;
}

bool Animal::shouldSkipMovementFrame()
{
	movementCounter++;
	return movementCounter % 5 == 0;
}

void Animal::moveInsideField(int maxSpeed, int changeInterval)
{
	if (shouldSkipMovementFrame())
	{
		return;
	}

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
	if (imageLoaded)
	{
		pWind->DrawImage(sprite, RefPoint.x, RefPoint.y, width, height);
	}
	else
	{
		pWind->SetPen(BLACK, 1);
		pWind->SetBrush(color(229, 214, 178));
		pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height, FILLED, 6, 6);
		pWind->SetBrush(color(226, 178, 122));
		pWind->DrawCircle(RefPoint.x + (width / 2), RefPoint.y + 14, 9);
		pWind->SetBrush(color(58, 117, 67));
		pWind->DrawRectangle(RefPoint.x + 14, RefPoint.y + 25, RefPoint.x + width - 14, RefPoint.y + height - 6);
	}
}



bool Animal::isProductReady()
{ 
		updateHappiness();

		if (happiness <= 0)
		{
			return false;
		}

		if (productIntervalMs <= 0)
		{
			return false;
	
	}
	if (productIntervalMs <= 0)
	{
		return false;
	}

	unsigned long long currentTick = GetTickCount64();
	if (currentTick - lastProductTick >= static_cast<unsigned long long>(productIntervalMs))
	{
		lastProductTick = currentTick;
		return true;
	}

	return false;
}

int Animal::getRemainingProductSeconds() const
{
	if (productIntervalMs <= 0)
	{
		return -1;
	}

	unsigned long long currentTick = GetTickCount64();
	unsigned long long elapsedMs = currentTick - lastProductTick;
	if (elapsedMs >= static_cast<unsigned long long>(productIntervalMs))
	{
		return 0;
	}

	unsigned long long remainingMs = static_cast<unsigned long long>(productIntervalMs) - elapsedMs;
	return static_cast<int>((remainingMs + 999) / 1000);
}

void Animal::drawCounter() const
{
	int remainingSeconds = getRemainingProductSeconds();
	window* pWind = pGame->getWind();
	pWind->SetFont(12, BOLD, BY_NAME, "Arial");

	if (remainingSeconds >= 0)
	{
		pWind->SetPen(BLACK, 1);
		pWind->DrawString(RefPoint.x + 8, RefPoint.y - 14, to_string(remainingSeconds));
	}
	pWind->SetPen(happiness > 20 ? color(0, 0, 255) : color(255, 0, 0), 1);
	string happinessText = "Happy: " + to_string(happiness) + "% " + (happiness <= 0 ? "CS" : " ");
	pWind->DrawString(RefPoint.x + 3, RefPoint.y + height + 14, happinessText);

	pWind->SetPen(color(21, 102, 45), 1);
	pWind->DrawString(RefPoint.x + 3, RefPoint.y + height + 2, "Food: " + to_string(foodEatenCounter));
}
bool Animal::isGoldenAnimal() const
{
	return isGolden;
}

bool Animal::didProduceProductThisStep() const
{
	return productProducedThisStep;
}

void Animal::resetProductProducedThisStep()
{
	productProducedThisStep = false;
}

void Animal::produceProduct()
{
	productProducedThisStep = false;
	point productPoint;
	productPoint.x = RefPoint.x + (width / 2);
	productPoint.y = RefPoint.y + height;

	if (productType == PRODUCT_EGG)
	{
		productProducedThisStep = pGame->addEgg(productPoint, isGolden);
	}
	else if (productType == PRODUCT_MILK)
	{
		productProducedThisStep = pGame->addMilk(productPoint);
	}
	else if (productType == PRODUCT_WOOL)
	{
		productProducedThisStep = pGame->addWool(productPoint);
	}
}

Chick::Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path,bool is_gold) : Animal(r_pGame, r_point, r_width, r_height, img_path,is_gold)
{
	if (isGolden)
	{
		this->setDx(this->getDx() * 3);
		this->setDy(this->getDy() * 3);
	}
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
	updateHappiness();
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
	updateHappiness();
	int level = pGame->getLevel();
	int speedRange = 1 + ((level - 1) / 3);
	if (speedRange > 4)
	{
		speedRange = 4;
	}
	moveInsideField(speedRange, 65);

	draw();
	drawCounter();
}

Dog::Dog(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	birthTick = GetTickCount64();
	setDx(1);
	setDy(1);
}

void Dog::moveStep()
{
	updateHappiness();
	moveInsideField(3, 35);
	draw();
	drawCounter();
	drawLifetimeCounter();
}

void Dog::moveToward(point target)
{
	
	point dogPoint = getRefPoint();
	const int dogCenterX = dogPoint.x + 25;
	const int dogCenterY = dogPoint.y + 25;
	const int targetCenterX = target.x + 25;
	const int targetCenterY = target.y + 25;
	const int chaseSpeed = 6;

	setDx(stepToward(targetCenterX - dogCenterX, chaseSpeed));
	setDy(stepToward(targetCenterY - dogCenterY, chaseSpeed));

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	const int minX = config.fieldPadding;
	const int minY = (2 * config.toolBarHeight) + config.fieldPadding;
	const int maxX = config.windWidth - config.fieldPadding - width;
	const int maxY = config.windHeight - config.statusBarHeight - config.fieldPadding - height;

	if (RefPoint.x < minX)
	{
		RefPoint.x = minX;
	}
	else if (RefPoint.x > maxX)
	{
		RefPoint.x = maxX;
	}

	if (RefPoint.y < minY)
	{
		RefPoint.y = minY;
	}
	else if (RefPoint.y > maxY)
	{
		RefPoint.y = maxY;
	}

	draw();
	drawCounter();
	drawLifetimeCounter();
}

bool Dog::isExpired() const
{
	return GetTickCount64() - birthTick >= lifetimeMs;
}

int Dog::getRemainingLifetimeSeconds() const
{
	unsigned long long elapsedMs = GetTickCount64() - birthTick;
	if (elapsedMs >= lifetimeMs)
	{
		return 0;
	}

	unsigned long long remainingMs = lifetimeMs - elapsedMs;
	return static_cast<int>((remainingMs + 999) / 1000);
}

void Dog::drawLifetimeCounter() const
{
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 1);
	pWind->SetFont(12, BOLD, BY_NAME, "Arial");
	pWind->DrawString(RefPoint.x + 17, RefPoint.y - 14, to_string(getRemainingLifetimeSeconds()));
}

Farmer::Farmer(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
	birthTick = GetTickCount64();
	setDx(1);
	setDy(1);
}

void Farmer::moveStep()
{
	moveInsideField(2, 40);
	draw();
	drawLifetimeCounter();
}

void Farmer::moveToward(point target)
{
	point farmerPoint = getRefPoint();
	const int farmerCenterX = farmerPoint.x + (width / 2);
	const int farmerCenterY = farmerPoint.y + (height / 2);
	const int targetCenterX = target.x + 20;
	const int targetCenterY = target.y + 20;
	const int walkSpeed = 5;

	setDx(stepToward(targetCenterX - farmerCenterX, walkSpeed));
	setDy(stepToward(targetCenterY - farmerCenterY, walkSpeed));

	RefPoint.x += getDx();
	RefPoint.y += getDy();

	const int minX = config.fieldPadding;
	const int minY = (2 * config.toolBarHeight) + config.fieldPadding;
	const int maxX = config.windWidth - config.fieldPadding - width;
	const int maxY = config.windHeight - config.statusBarHeight - config.fieldPadding - height;

	if (RefPoint.x < minX)
	{
		RefPoint.x = minX;
	}
	else if (RefPoint.x > maxX)
	{
		RefPoint.x = maxX;
	}

	if (RefPoint.y < minY)
	{
		RefPoint.y = minY;
	}
	else if (RefPoint.y > maxY)
	{
		RefPoint.y = maxY;
	}

	draw();
	drawLifetimeCounter();
}

bool Farmer::isExpired() const
{
	return GetTickCount64() - birthTick >= lifetimeMs;
}

int Farmer::getRemainingLifetimeSeconds() const
{
	unsigned long long elapsedMs = GetTickCount64() - birthTick;
	if (elapsedMs >= lifetimeMs)
	{
		return 0;
	}

	unsigned long long remainingMs = lifetimeMs - elapsedMs;
	return static_cast<int>((remainingMs + 999) / 1000);
}

void Farmer::drawLifetimeCounter() const
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


void Animal::decreaseHappiness(int amount) {
	happiness -= amount;
	if (happiness < 0) happiness = 0;
}

void Animal::increaseHappiness(int amount) {
	happiness += amount;
	if (happiness > maxHappiness) happiness = maxHappiness;
}

void Animal::updateHappiness() {
	unsigned long long currentTick = GetTickCount64();

	if (currentTick - lastHappinessTick >= 15000) {
		decreaseHappiness(5);
		lastHappinessTick = currentTick;
	}
}
