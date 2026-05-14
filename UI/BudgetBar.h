#pragma once
#include "../Core/Drawable.h"
#include "../Entities/Animal.h"
#include "../Config/GameConfig.h"
#include <random>
#include <limits>

//Ranges for generating a random location for the animals
const int range_min_x = 50;
const int range_max_x = config.windWidth - config.warehouseWidth - 80;
const int range_min_y = (config.toolBarHeight * 2) + 50;
const int range_max_y = config.windHeight - config.statusBarHeight - 50;
const int grass_tiles_per_water = 8;
const int max_budget_items = 15;
const int animal_cost = 100;
const int dog_cost = 500;

//Base class for all toolbar icons 
class BudgetbarIcon :public Drawable
{
private:
	//string image_path;
public:
	string image_path;
	image iconImage;
	BudgetbarIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void draw() const override;
	virtual void onClick() = 0;   //The action that should be taken when this icon is clicked
};

class ChickIcon : public BudgetbarIcon
{
public:
	Chick** chickList; //an array of Chick pointers
	int count = 0;
	ChickIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~ChickIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};

class CowIcon : public BudgetbarIcon
{
public:
	Cow** cowList; //an array of Chick pointers
	int count = 0;
	CowIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~CowIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};

class GoatIcon : public BudgetbarIcon
{
public:
	Goat** goatList; //an array of Chick pointers
	int count = 0;
	GoatIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~GoatIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};

class SheepIcon : public BudgetbarIcon
{
public:
	Sheep** sheepList; //an array of Chick pointers
	int count = 0;
	SheepIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~SheepIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};

class WaterIcon : public BudgetbarIcon
{
public:
	Water** waterList; //an array of Chick pointers
	image grassImage;
	point grassTiles[max_budget_items][grass_tiles_per_water];
	int grassTileCounts[max_budget_items];
	unsigned long grassLastDecayTick[max_budget_items];
	int count = 0;
	WaterIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~WaterIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
	void drawFoodCounter() const;
	bool animalsNearGrass(int index) const;
};

class WolfIcon : public BudgetbarIcon
{
public:
	Wolf** wolfList;
	int count = 0;
	WolfIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~WolfIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};

class DuckIcon : public BudgetbarIcon
{
public:
	Duck** duckList; // an array of Duck pointers
	int count = 0;
	DuckIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~DuckIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};

class DogIcon : public BudgetbarIcon
{
public:
	Dog** dogList;
	int count = 0;
	DogIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	~DogIcon();
	virtual void onClick();
	void updateAnimals();
	void resetAnimals();
};


// TO DO: The rest of icons in the toolbar

enum ANIMAL_ICONS //The icons of the toolbar (you should add more icons)
{
	//Note: Icons are ordered here as they appear in menu
	//If you want to change the menu icons order, change the order here
	ICON_CHICK,
	ICON_COW,
	ICON_GOAT,
	ICON_SHEEP,
	ICON_WATER,
	ICON_DUCK,
	ICON_DOG,

	//TODO: Add more icons names here

	//Cow icon

	ANIMAL_COUNT		//no. of menu icons ==> This should be the last line in this enum
};

class Budgetbar : public Drawable
{
private:
	BudgetbarIcon** iconsList; //an array of toolbarIcon pointers
	string iconsImages[ANIMAL_COUNT];

public:
	Budgetbar(Game* r_pGame, point r_point, int r_width, int r_height);
	~Budgetbar();
	void draw() const override;
	bool handleClick(int x, int y);	//handles clicks on toolbar icons, returns true if exit is clicked
	void updateAnimals();
	void resetAnimals();
	int getAnimalCount() const;
	ChickIcon* getChickIcon() const;
	CowIcon* getCowIcon() const;
	GoatIcon* getGoatIcon() const;
	SheepIcon* getSheepIcon() const;
	WaterIcon* getWaterIcon() const;
	DuckIcon* getDuckIcon() const;
	DogIcon* getDogIcon() const;

};

