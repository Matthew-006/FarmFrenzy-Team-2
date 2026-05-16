#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"
#include "Drawable.h"
#include <string>
#include <utility>
#include <vector>

class Toolbar;
class Budgetbar;
class Egg;
class Milk;
class Wool;
class Wolf;
class duck;
class Farmer;

class Game
{
private:
	enum GameOverChoice
	{
		PLAY_AGAIN,
		DIFFERENT_USER,
		EXIT_GAME
	};

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
	bool goldenChickSpawned;
	int warehouseEgg;
	int warehouseMilk;
	int warehouseWool;
	bool isPaused;
	unsigned long long lastTime;
	int timer;
	int level;
	int goal;
	int score;
	int animals;
	std::string username;
	bool exitRequested;
	bool soundMuted;
	bool helperActive;
	Farmer* helperFarmer;
	void DrawProducts() const;
	void clearProducts();
	void clearWolves();
	void drawWarehouseInventory(int left, int top, int right, int bottom) const;
	bool isProductAreaFree(const point& location, int width, int height) const;
	bool findFreeProductSpot(point& location, int width, int height) const;
	void handleProductClick(int x, int y);
	bool handleWarehouseClick(int x, int y);
	void sellWarehouseProduct(int& productCount, int price, const std::string& productName);
	void updateHelper();
	bool getNearestProductPoint(point fromPoint, point& productPoint) const;
	bool collectProductAt(point farmerPoint, int farmerWidth, int farmerHeight);
	void showRandomWolf();
	void promptForUsername();
	void drawStartScreen(const std::string& typedName, bool isGameOver = false, int lastScore = 0) const;
	void drawStartLeaderboard(int left, int top, const std::vector<std::pair<std::string, int>>& leaderboard) const;
	std::string getUsernameFromStartScreen() const;
	int calculateScore() const;
	GameOverChoice handleGameOver();
	std::vector<std::pair<std::string, int>> loadLeaderboard() const;
	void saveLeaderboard(const std::vector<std::pair<std::string, int>>& leaderboard) const;
	void updateLeaderboard(int score);
	void resetGameState();
	int lastMilestoneBudget;
public:
	void playInternalMusic();
	void triggerGameOver();
	void setTimerByLevel();
	void checkLevelUp();
	void showWarehouseWindow();
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
	void activateHelper();
	void clearStatusBar() const;	//Clears the status bar
	void startBackgroundMusic() const;
	void stopBackgroundMusic() const;
	void toggleSound();
	bool isSoundMuted() const;
	void updateTimer();
	void updateStatusBar() const;
	void restartGame();
	void pauseGame();
	void resumeGame();
	void saveGame() const;
	void loadGame();
	bool addEgg(point location, bool is_gold = false);
	bool addMilk(point location);
	bool addWool(point location);
	void setEggCount(int count);
	int getLevel() const;
	bool getNearestWolfPoint(point fromPoint, point& wolfPoint) const;
	bool removeWolfAt(point dogPoint, int dogWidth, int dogHeight);


	void printMessage(std::string msg) const;	//Print a message on Status bar

	void go();

	window* getWind() const;		//returns a pointer to the graphics window
	Budgetbar* getBudgetbar() const;
	void handleFeedingLogic();
	void checkBudgetMilestone();
};

