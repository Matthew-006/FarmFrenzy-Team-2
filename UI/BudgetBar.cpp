#include "Budgetbar.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <iostream>
using namespace std;

namespace
{
	bool isInsideFieldTile(const point& p, int tileSize)
	{
		return p.x >= 0 &&
			p.y >= 2 * config.toolBarHeight &&
			p.x <= config.windWidth - tileSize &&
			p.y <= config.windHeight - config.statusBarHeight - tileSize;
	}

	bool canBuyItem(Game* game, int currentCount)
	{
		if (currentCount >= max_budget_items)
		{
			game->printMessage("Placement limit reached for this item");
			return false;
		}
		
		return true;
	}
}


BudgetbarIcon::BudgetbarIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
	if (!image_path.empty())
	{
		iconImage.Open(image_path);
	}
}

void BudgetbarIcon::draw() const
{
	//draw image of this object
	window* pWind = pGame->getWind();
	pWind->DrawImage(iconImage, RefPoint.x, RefPoint.y, width, height);
}

ChickIcon::ChickIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	chickList = new Chick * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++) {
		chickList[i] = nullptr;
	}
}

ChickIcon::~ChickIcon()
{
	resetAnimals();
	delete[] chickList;
}

void ChickIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/
	
	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Chick Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(animal_cost)) {

		point p;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd1;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x, range_max_x);
		p.x = dist1(gen1);
		//std::cout << "P.X = " << p.x << endl;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd2;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y, range_max_y);
		p.y = dist2(gen2);
		//std::cout << "P.Y = " << p.y << endl;
		//p.x = 300;
		//p.y = 300;
		chickList[count]= new Chick(pGame, p, 50, 50, image_path);
		chickList[count]->draw();
		count++;
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void ChickIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (chickList[i] != nullptr)
			chickList[i]->moveStep();
	}
}

void ChickIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete chickList[i];
		chickList[i] = nullptr;
	}
	count = 0;
}

CowIcon::CowIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	cowList = new Cow * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++) {
		cowList[i] = nullptr;
	}
}

CowIcon::~CowIcon()
{
	resetAnimals();
	delete[] cowList;
}

void CowIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/

	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Cow Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(animal_cost)) {

		point p;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd1;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x, range_max_x);
		p.x = dist1(gen1);
		//std::cout << "P.X = " << p.x << endl;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd2;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y, range_max_y);
		p.y = dist2(gen2);
		//std::cout << "P.Y = " << p.y << endl;
		//p.x = 300;
		//p.y = 300;
		cowList[count] = new Cow(pGame, p, 50, 50, image_path);
		cowList[count]->draw();
		count++;
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void CowIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (cowList[i] != nullptr)
			cowList[i]->moveStep();
	}
}

void CowIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete cowList[i];
		cowList[i] = nullptr;
	}
	count = 0;
}

GoatIcon::GoatIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	goatList = new Goat * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++) {
		goatList[i] = nullptr;
	}
}

GoatIcon::~GoatIcon()
{
	resetAnimals();
	delete[] goatList;
}

void GoatIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/
	
	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Goat Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(animal_cost)) {

		point p;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd1;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x, range_max_x);
		p.x = dist1(gen1);
		//std::cout << "P.X = " << p.x << endl;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd2;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y, range_max_y);
		p.y = dist2(gen2);
		//std::cout << "P.Y = " << p.y << endl;
		//p.x = 300;
		//p.y = 300;
		goatList[count]= new Goat(pGame, p, 50, 50, image_path);
		goatList[count]->draw();
		count++;
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void GoatIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (goatList[i] != nullptr)
			goatList[i]->moveStep();
	}
}

void GoatIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete goatList[i];
		goatList[i] = nullptr;
	}
	count = 0;
}

SheepIcon::SheepIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	sheepList = new Sheep * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++) {
		sheepList[i] = nullptr;
	}
}

SheepIcon::~SheepIcon()
{
	resetAnimals();
	delete[] sheepList;
}

void SheepIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/

	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Sheep Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(animal_cost)) {

		point p;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd1;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x, range_max_x);
		p.x = dist1(gen1);
		//std::cout << "P.X = " << p.x << endl;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd2;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y, range_max_y);
		p.y = dist2(gen2);
		//std::cout << "P.Y = " << p.y << endl;
		//p.x = 300;
		//p.y = 300;
		sheepList[count] = new Sheep(pGame, p, 50, 50, image_path);
		sheepList[count]->draw();
		count++;
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void SheepIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (sheepList[i] != nullptr)
			sheepList[i]->moveStep();
	}
}

void SheepIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete sheepList[i];
		sheepList[i] = nullptr;
	}
	count = 0;
}

WaterIcon::WaterIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	waterList = new Water * [max_budget_items];
	grassImage.Open("images\\grass.jpeg");
	for (int i = 0; i < max_budget_items; i++) {
		waterList[i] = nullptr;
		grassTileCounts[i] = 0;
	}
}

WaterIcon::~WaterIcon()
{
	resetAnimals();
	delete[] waterList;
}

void WaterIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/

	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Water Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(animal_cost)) {

		point p;
		std::random_device rd1;
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x + 50, range_max_x - 50);
		p.x = dist1(gen1);
		std::random_device rd2;
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y + 50, range_max_y - 50);
		p.y = dist2(gen2);

		const int tileSize = 50;
		const int offsets[grass_tiles_per_water][2] =
		{
			{-tileSize, -tileSize}, {0, -tileSize}, {tileSize, -tileSize},
			{-tileSize, 0},                            {tileSize, 0},
			{-tileSize, tileSize},  {0, tileSize},   {tileSize, tileSize}
		};

		grassTileCounts[count] = 0;
		for (int i = 0; i < grass_tiles_per_water; i++)
		{
			point grassPoint{ p.x + offsets[i][0], p.y + offsets[i][1] };
			if (isInsideFieldTile(grassPoint, tileSize))
			{
				grassTiles[count][grassTileCounts[count]] = grassPoint;
				grassTileCounts[count]++;
			}
		}

		window* pWind = pGame->getWind();
		for (int i = 0; i < grassTileCounts[count]; i++)
		{
			pWind->DrawImage(grassImage, grassTiles[count][i].x, grassTiles[count][i].y, tileSize, tileSize);
		}

		waterList[count] = new Water(pGame, p, 50, 50, image_path);
		waterList[count]->draw();
		count++;
	}
}

void WaterIcon::updateAnimals()
{
	window* pWind = pGame->getWind();

	for (int i = 0; i < count; i++)
	{
		if (waterList[i] != nullptr)
		{
			for (int j = 0; j < grassTileCounts[i]; j++)
			{
				pWind->DrawImage(grassImage, grassTiles[i][j].x, grassTiles[i][j].y, 50, 50);
			}

			waterList[i]->draw();
		}
	}
}

void WaterIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete waterList[i];
		waterList[i] = nullptr;
		grassTileCounts[i] = 0;
	}
	count = 0;
}

WolfIcon::WolfIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	wolfList = new Wolf * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++) {
		wolfList[i] = nullptr;
	}
}

WolfIcon::~WolfIcon()
{
	resetAnimals();
	delete[] wolfList;
}

void WolfIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/

	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Wolf Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(animal_cost)) {

		point p;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd1;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x, range_max_x);
		p.x = dist1(gen1);
		//std::cout << "P.X = " << p.x << endl;
		// 1. Obtain a seed from a non-deterministic source (if available)
		std::random_device rd2;

		// 2. Seed the Mersenne Twister engine
		// std::mt19937 is a high-quality pseudo-random number generator
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y, range_max_y);
		p.y = dist2(gen2);
		//std::cout << "P.Y = " << p.y << endl;
		//p.x = 300;
		//p.y = 300;
		wolfList[count] = new Wolf(pGame, p, 50, 50, image_path);
		wolfList[count]->draw();
		count++;
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void WolfIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (wolfList[i] != nullptr)
			wolfList[i]->moveStep();
	}
}

void WolfIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete wolfList[i];
		wolfList[i] = nullptr;
	}
	count = 0;
}

Budgetbar::Budgetbar(Game* r_pGame, point r_point, int r_width, int r_height) : Drawable(r_pGame, r_point, r_width, r_height)
{
	//First prepare List of images for each icon
	//To control the order of these images in the menu, reoder them in enum ICONS above	
	iconsImages[ICON_CHICK] = "images\\chick (3).JPEG";
	iconsImages[ICON_COW] = "images\\cow.JPEG";
	iconsImages[ICON_GOAT] = "images\\goat.JPEG";
	iconsImages[ICON_SHEEP] = "images\\sheep.JPEG";
	iconsImages[ICON_WATER] = "images\\water.JPEG";
	iconsImages[ICON_WOLF] = "images\\wolf.JPEG";

	point p;
	p.x = 0;
	p.y = config.toolBarHeight;

	iconsList = new BudgetbarIcon * [ANIMAL_COUNT];

	//For each icon in the tool bar create an object 
	iconsList[ICON_CHICK] = new ChickIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_CHICK]);
	p.x += config.iconWidth;

	iconsList[ICON_COW] = new CowIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_COW]);
	p.x += config.iconWidth;

	iconsList[ICON_GOAT] = new GoatIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_GOAT]);
	p.x += config.iconWidth;

	iconsList[ICON_SHEEP] = new SheepIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_SHEEP]);
	p.x += config.iconWidth;

	iconsList[ICON_WATER] = new WaterIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_WATER]);
	p.x += config.iconWidth;
	iconsList[ICON_WOLF] = new WolfIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_WOLF]);
	p.x += config.iconWidth;
	//p.x += config.iconWidth;
	//iconsList[ICON_CHICK] = new ChickIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_CHICK]);
}

Budgetbar::~Budgetbar()
{
	for (int i = 0; i < ANIMAL_COUNT; i++)
		delete iconsList[i];
	delete[] iconsList;
}

void Budgetbar::draw() const
{
	for (int i = 0; i < ANIMAL_COUNT; i++)
		iconsList[i]->draw();
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 3);
	pWind->DrawLine(0, 2*config.toolBarHeight, pWind->GetWidth(), 2*config.toolBarHeight);
}

bool Budgetbar::handleClick(int x, int y)
{
	if (x > ANIMAL_COUNT * config.iconWidth)	//click outside toolbar boundaries
		return false;


	//Check whick icon was clicked
	//==> This assumes that menu icons are lined up horizontally <==
	//Divide x co-ord of the point clicked by the icon width (int division)
	//if division result is 0 ==> first icon is clicked, if 1 ==> 2nd icon and so on

	int clickedIconIndex = (x / config.iconWidth);
	iconsList[clickedIconIndex]->onClick();	//execute onClick action of clicled icon

	//if (clickedIconIndex == ICON_EXIT) return true;

	return false;

}

void Budgetbar::updateAnimals()
{
	((ChickIcon*)iconsList[ICON_CHICK])->updateAnimals();
	((CowIcon*)iconsList[ICON_COW])->updateAnimals();
	((GoatIcon*)iconsList[ICON_GOAT])->updateAnimals();
	((SheepIcon*)iconsList[ICON_SHEEP])->updateAnimals();
	((WaterIcon*)iconsList[ICON_WATER])->updateAnimals();
	((WolfIcon*)iconsList[ICON_WOLF])->updateAnimals();
}

void Budgetbar::resetAnimals()
{
	((ChickIcon*)iconsList[ICON_CHICK])->resetAnimals();
	((CowIcon*)iconsList[ICON_COW])->resetAnimals();
	((GoatIcon*)iconsList[ICON_GOAT])->resetAnimals();
	((SheepIcon*)iconsList[ICON_SHEEP])->resetAnimals();
	((WaterIcon*)iconsList[ICON_WATER])->resetAnimals();
	((WolfIcon*)iconsList[ICON_WOLF])->resetAnimals();
}

int Budgetbar::getAnimalCount() const
{
	return ((ChickIcon*)iconsList[ICON_CHICK])->count +
		((CowIcon*)iconsList[ICON_COW])->count +
		((GoatIcon*)iconsList[ICON_GOAT])->count +
		((SheepIcon*)iconsList[ICON_SHEEP])->count +
		((WolfIcon*)iconsList[ICON_WOLF])->count;
}
