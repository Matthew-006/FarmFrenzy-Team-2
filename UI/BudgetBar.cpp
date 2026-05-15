#include "Budgetbar.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <cmath>
#include <iostream>
using namespace std;

namespace
{
	const int feeding_area_center_x = 170;
	const int feeding_area_center_y = (2 * config.toolBarHeight) + 145;
	const int feeding_area_radius = 95;
	const int grass_tile_size = 28;
	const int grass_consume_distance = 42;
	const unsigned long grass_consume_delay_ms = 650;

	bool isInsideFieldTile(const point& p, int tileSize)
	{
		return p.x >= 0 &&
			p.y >= 2 * config.toolBarHeight &&
			p.x <= config.windWidth - tileSize &&
			p.y <= config.windHeight - config.statusBarHeight - tileSize;
	}

	bool isInsideFeedingArea(const point& p, int tileSize)
	{
		const int centerX = p.x + (tileSize / 2);
		const int centerY = p.y + (tileSize / 2);
		const int dx = centerX - feeding_area_center_x;
		const int dy = centerY - feeding_area_center_y;
		const int radius = feeding_area_radius - (tileSize / 2);
		return (dx * dx) + (dy * dy) <= (radius * radius);
	}

	bool isAnimalNearPoint(const Animal* animal, const point& p, int threshold)
	{
		if (animal == nullptr)
		{
			return false;
		}

		point animalPoint = animal->getRefPoint();
		const int animalCenterX = animalPoint.x + 25;
		const int animalCenterY = animalPoint.y + 25;
		const int grassCenterX = p.x + (grass_tile_size / 2);
		const int grassCenterY = p.y + (grass_tile_size / 2);
		const int dx = animalCenterX - grassCenterX;
		const int dy = animalCenterY - grassCenterY;
		return (dx * dx) + (dy * dy) <= (threshold * threshold);
	}

	bool canBuyItem(Game* game, int currentCount)
	{
		if (currentCount >= max_budget_items)
		{
			game->printMessage("Limit reached: max " + to_string(max_budget_items) + " of this item");
			return false;
		}
		
		return true;
	}

	void drawGrassTile(window* pWind, const point& p)
	{
		pWind->SetPen(color(56, 122, 48), 1);
		pWind->SetBrush(color(88, 176, 74));
		pWind->DrawCircle(p.x + 10, p.y + 12, 7);
		pWind->DrawCircle(p.x + 17, p.y + 10, 8);
		pWind->DrawCircle(p.x + 13, p.y + 18, 7);
	}

	void drawWaterPatch(window* pWind, const point& p)
	{
		pWind->SetPen(color(38, 108, 176), 2);
		pWind->SetBrush(color(101, 185, 234));
		pWind->DrawCircle(p.x + 18, p.y + 18, 15);
		pWind->SetBrush(color(163, 220, 248));
		pWind->DrawCircle(p.x + 13, p.y + 13, 5);
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
	for (int i = 0; i < max_budget_items; i++) {
		waterList[i] = nullptr;
		grassTileCounts[i] = 0;
		grassLastDecayTick[i] = 0;
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
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> angleDist(0, 359);
		std::uniform_int_distribution<int> radiusDist(0, feeding_area_radius - 18);

		double angle = angleDist(gen) * 3.14159265358979323846 / 180.0;
		int radius = radiusDist(gen);
		p.x = feeding_area_center_x + static_cast<int>(radius * cos(angle)) - 25;
		p.y = feeding_area_center_y + static_cast<int>(radius * sin(angle)) - 25;

		const int offsets[grass_tiles_per_water][2] =
		{
			{0, 0},
			{-grass_tile_size, 0}, {grass_tile_size, 0},
			{0, -grass_tile_size}, {0, grass_tile_size},
			{-20, -20}, {20, -20}, {0, 20}
		};

		grassTileCounts[count] = 0;
		for (int i = 0; i < grass_tiles_per_water; i++)
		{
			point grassPoint{ p.x + offsets[i][0], p.y + offsets[i][1] };
			if (isInsideFieldTile(grassPoint, grass_tile_size) && isInsideFeedingArea(grassPoint, grass_tile_size))
			{
				grassTiles[count][grassTileCounts[count]] = grassPoint;
				grassTileCounts[count]++;
			}
		}

		window* pWind = pGame->getWind();
		for (int i = 0; i < grassTileCounts[count]; i++)
		{
			drawGrassTile(pWind, grassTiles[count][i]);
		}

		waterList[count] = new Water(pGame, p, 50, 50, image_path);
		grassLastDecayTick[count] = GetTickCount();
		drawWaterPatch(pWind, p);
		count++;
	}
}

void WaterIcon::updateAnimals()
{
	window* pWind = pGame->getWind();
	unsigned long currentTick = GetTickCount();

	for (int i = 0; i < count; i++)
	{
		if (waterList[i] != nullptr)
		{
			if (grassTileCounts[i] <= 0)
			{
				delete waterList[i];
				waterList[i] = nullptr;
				continue;
			}

			for (int j = 0; j < grassTileCounts[i]; j++)
			{
				drawGrassTile(pWind, grassTiles[i][j]);
			}

			drawWaterPatch(pWind, waterList[i]->getRefPoint());
		}
	}

	drawFoodCounter();
}

void WaterIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete waterList[i];
		waterList[i] = nullptr;
		grassTileCounts[i] = 0;
		grassLastDecayTick[i] = 0;
	}
	count = 0;
}

void WaterIcon::drawFoodCounter() const
{
	int totalGrass = 0;
	for (int i = 0; i < count; i++)
	{
		if (waterList[i] != nullptr)
		{
			totalGrass += grassTileCounts[i];
		}
	}

	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 1);
	pWind->SetFont(16, BOLD, BY_NAME, "Arial");
	pWind->DrawString(52, (2 * config.toolBarHeight) + config.fieldPadding - 8, "Food: " + to_string(totalGrass));
}

bool WaterIcon::animalsNearGrass(int index) const
{
	Budgetbar* budgetbar = pGame->getBudgetbar();
	ChickIcon* chickIcon = budgetbar->getChickIcon();
	CowIcon* cowIcon = budgetbar->getCowIcon();
	GoatIcon* goatIcon = budgetbar->getGoatIcon();
	SheepIcon* sheepIcon = budgetbar->getSheepIcon();
	DuckIcon* duckIcon = budgetbar->getDuckIcon();

	for (int j = 0; j < grassTileCounts[index]; j++)
	{
		const point& grassPoint = grassTiles[index][j];

		for (int i = 0; i < chickIcon->count; i++)
		{
			if (isAnimalNearPoint(chickIcon->chickList[i], grassPoint, grass_consume_distance))
			{
				return true;
			}
		}

		for (int i = 0; i < cowIcon->count; i++)
		{
			if (isAnimalNearPoint(cowIcon->cowList[i], grassPoint, grass_consume_distance))
			{
				return true;
			}
		}

		for (int i = 0; i < goatIcon->count; i++)
		{
			if (isAnimalNearPoint(goatIcon->goatList[i], grassPoint, grass_consume_distance))
			{
				return true;
			}
		}

		for (int i = 0; i < sheepIcon->count; i++)
		{
			if (isAnimalNearPoint(sheepIcon->sheepList[i], grassPoint, grass_consume_distance))
			{
				return true;
			}
		}
		for (int i = 0; i < duckIcon->count; i++)
		{
			if (isAnimalNearPoint(duckIcon->duckList[i], grassPoint, grass_consume_distance))
			{
				return true;
			}
		}
	}

	return false;
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

DuckIcon::DuckIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	duckList = new Duck * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++) {
		duckList[i] = nullptr;
	}
}

DuckIcon::~DuckIcon()
{
	resetAnimals();
	delete[] duckList;
}

void DuckIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/

	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Duck Clicked" << endl;
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
		duckList[count] = new Duck(pGame, p, 50, 50, image_path);
		duckList[count]->draw();
		count++;
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void DuckIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (duckList[i] != nullptr)
			duckList[i]->moveStep();
	}
}

void DuckIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete duckList[i];
		duckList[i] = nullptr;
	}
	count = 0;
}

DogIcon::DogIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
	dogList = new Dog * [max_budget_items];
	for (int i = 0; i < max_budget_items; i++)
	{
		dogList[i] = nullptr;
	}
}

DogIcon::~DogIcon()
{
	resetAnimals();
	delete[] dogList;
}

void DogIcon::onClick()
{
	cout << "Icon Dog Clicked" << endl;
	if (canBuyItem(pGame, count) && pGame->spendBudget(dog_cost))
	{
		point p;
		std::random_device rd1;
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(range_min_x, range_max_x);
		p.x = dist1(gen1);

		std::random_device rd2;
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(range_min_y, range_max_y);
		p.y = dist2(gen2);

		dogList[count] = new Dog(pGame, p, 50, 50, image_path);
		dogList[count]->draw();
		count++;
		pGame->printMessage("Dog bought for $500");
	}
}

void DogIcon::updateAnimals()
{
	for (int i = 0; i < count; i++)
	{
		if (dogList[i] == nullptr)
		{
			continue;
		}

		if (dogList[i]->isExpired())
		{
			delete dogList[i];
			dogList[i] = dogList[count - 1];
			dogList[count - 1] = nullptr;
			count--;
			i--;
			continue;
		}

		point wolfPoint;
		if (pGame->getNearestWolfPoint(dogList[i]->getRefPoint(), wolfPoint))
		{
			dogList[i]->moveToward(wolfPoint);
			if (pGame->removeWolfAt(dogList[i]->getRefPoint(), 50, 50))
			{
				dogList[i]->moveStep();
			}
		}
		else
		{
			dogList[i]->moveStep();
		}
	}
}

void DogIcon::resetAnimals()
{
	for (int i = 0; i < count; i++)
	{
		delete dogList[i];
		dogList[i] = nullptr;
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
	iconsImages[ICON_DUCK] = "images\\duck.jpg";
	iconsImages[ICON_DOG] = "images\\dog.jpg";

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
	
	iconsList[ICON_DUCK] = new DuckIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_DUCK]);
	p.x += config.iconWidth;
	
	iconsList[ICON_DOG] = new DogIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_DOG]);
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
	if (x >= ANIMAL_COUNT * config.iconWidth)	//click outside toolbar boundaries
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
	((DuckIcon*)iconsList[ICON_DUCK])->updateAnimals();
	((DogIcon*)iconsList[ICON_DOG])->updateAnimals();

}

void Budgetbar::resetAnimals()
{
	((ChickIcon*)iconsList[ICON_CHICK])->resetAnimals();
	((CowIcon*)iconsList[ICON_COW])->resetAnimals();
	((GoatIcon*)iconsList[ICON_GOAT])->resetAnimals();
	((SheepIcon*)iconsList[ICON_SHEEP])->resetAnimals();
	((WaterIcon*)iconsList[ICON_WATER])->resetAnimals();
	((DuckIcon*)iconsList[ICON_DUCK])->resetAnimals();
	((DogIcon*)iconsList[ICON_DOG])->resetAnimals();
}

int Budgetbar::getAnimalCount() const
{
	return ((ChickIcon*)iconsList[ICON_CHICK])->count +
		((CowIcon*)iconsList[ICON_COW])->count +
		((GoatIcon*)iconsList[ICON_GOAT])->count +
		((SheepIcon*)iconsList[ICON_SHEEP])->count+
		((DuckIcon*)iconsList[ICON_DUCK])->count;
}


ChickIcon* Budgetbar::getChickIcon() const
{
	return (ChickIcon*)iconsList[ICON_CHICK];
}

CowIcon* Budgetbar::getCowIcon() const
{
	return (CowIcon*)iconsList[ICON_COW];
}

GoatIcon* Budgetbar::getGoatIcon() const
{
	return (GoatIcon*)iconsList[ICON_GOAT];
}

SheepIcon* Budgetbar::getSheepIcon() const
{
	return (SheepIcon*)iconsList[ICON_SHEEP];
}

WaterIcon* Budgetbar::getWaterIcon() const
{
	return (WaterIcon*)iconsList[ICON_WATER];
}
DuckIcon* Budgetbar::getDuckIcon() const
{
	return (DuckIcon*)iconsList[ICON_DUCK];
}

DogIcon* Budgetbar::getDogIcon() const
{
	return (DogIcon*)iconsList[ICON_DOG];
}
