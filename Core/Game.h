#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"
#include "Drawable.h"
#include <string>

class Toolbar;
class Budgetbar;
class Egg;
class Milk;
class Wool;
class Wolf;

class Game
{
private:
	static constexpr int kMaxProducts = 50;
	static constexpr int kEggPrice = 15;
	static constexpr int kMilkPrice = 25;
	static constexpr int kWoolPrice = 20;
	window* pWind;	//Pointer to the CMU graphics window
	Toolbar* gameToolbar;
	Budgetbar* gameBudgetbar;
	Egg* eggList[kMaxProducts];
	Milk* milkList[kMaxProducts];
	Wool* woolList[kMaxProducts];
	Wolf* wolfList[kMaxProducts];
	int wolfCount;
	int eggCount;
	int milkCount;
	int woolCount;
	int warehouseEgg;
	int warehouseMilk;
	int warehouseWool;
	bool isPaused;
	unsigned long lastTime;
	int timer;
	int level;
	int goal;
	int animals;
	void DrawProducts() const;
	void clearProducts();
	void clearWolves();
	void drawWarehouseInventory(int left, int top, int right, int bottom) const;
	bool isProductAreaFree(const point& location, int width, int height) const;
	bool findFreeProductSpot(point& location, int width, int height) const;
	void handleProductClick(int x, int y);
	void showRandomWolf();

public:
	static constexpr int kStartingBudget = 2000;
	int budget = kStartingBudget;
	Game();
	~Game();

	clicktype getMouseClick(int& x, int& y) const; //Get coordinate where user clicks and returns click type (left/right)
	std::string getSrting() const;	 //Returns a string entered by the user


	window* CreateWind(int, int, int, int) const; //creates the game window
	void createToolbar();
	void createBudgetbar();
	void drawFieldBackground() const;
	void drawWarehouse() const;
	void drawFoodArea() const;
	void clearBudget() const;
	void printBudget(std::string msg) const;
	bool spendBudget(int amount);
	void clearStatusBar() const;	//Clears the status bar
	void updateTimer();
	void updateStatusBar() const;
	void restartGame();
	void pauseGame();
	void resumeGame();
	void saveGame() const;
	void loadGame();
	bool addEgg(point location);
	bool addMilk(point location);
	bool addWool(point location);


	void printMessage(std::string msg) const;	//Print a message on Status bar

	void go();

	window* getWind() const;		//returns a pointer to the graphics window
	Budgetbar* getBudgetbar() const;
};

