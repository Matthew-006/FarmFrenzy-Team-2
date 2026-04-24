#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Product.h"

class Game
{
private:
	static constexpr int kMaxProducts = 50;
	window* pWind;	//Pointer to the CMU graphics window
	Toolbar* gameToolbar;
	Budgetbar* gameBudgetbar;
	Egg* eggList[kMaxProducts];
	Milk* milkList[kMaxProducts];
	int eggCount;
	int milkCount;
	bool isPaused;
	unsigned long lastTime;
	int timer;
	int level;
	int goal;
	int animals;
	void DrawProducts() const;

public:
	static constexpr int kStartingBudget = 2000;
	int budget = kStartingBudget;
	Game();
	~Game();

	clicktype getMouseClick(int& x, int& y) const; //Get coordinate where user clicks and returns click type (left/right)
	string getSrting() const;	 //Returns a string entered by the user


	window* CreateWind(int, int, int, int) const; //creates the game window
	void createToolbar();
	void createBudgetbar();
	void drawFieldBackground() const;
	void drawWarehouse() const;
	void drawFoodArea() const;
	void clearBudget() const;
	void printBudget(string msg) const;
	bool spendBudget(int amount);
	void clearStatusBar() const;	//Clears the status bar
	void updateTimer();
	void updateStatusBar() const;
	void restartGame();
	void pauseGame();
	void resumeGame();
	void saveGame() const;
	void loadGame();


	void printMessage(string msg) const;	//Print a message on Status bar

	void go();

	window* getWind() const;		//returns a pointer to the graphics window
};

