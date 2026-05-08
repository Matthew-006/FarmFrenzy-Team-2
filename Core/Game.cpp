#include "Game.h"
#include "../Config/GameConfig.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Product.h"
#include "../Entities/Animal.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace
{
	const int kFeedingAreaCenterX = 170;
	const int kFeedingAreaCenterY = (2 * config.toolBarHeight) + 145;
	const int kFeedingAreaRadius = 95;
	const char* kQuitUserName = "__QUIT__";
	const char* kLeaderboardFileName = "leaderboard.txt";

	bool rectanglesOverlap(const point& firstPoint, int firstWidth, int firstHeight, const point& secondPoint, int secondWidth, int secondHeight)
	{
		return firstPoint.x < secondPoint.x + secondWidth &&
			firstPoint.x + firstWidth > secondPoint.x &&
			firstPoint.y < secondPoint.y + secondHeight &&
			firstPoint.y + firstHeight > secondPoint.y;
	}

	bool compareScoresDescending(const std::pair<std::string, int>& first, const std::pair<std::string, int>& second)
	{
		return first.second > second.second;
	}

	void sanitizeLeaderboardName(std::string& name)
	{
		for (size_t i = 0; i < name.size(); i++)
		{
			if (name[i] == ',')
			{
				name[i] = '_';
			}
		}
	}

	void getWarehouseInventoryBounds(int& left, int& top, int& right, int& bottom)
	{
		const int fieldBottom = config.windHeight - config.statusBarHeight;
		const int warehouseWidth = config.warehouseWidth;
		const int warehouseHeight = config.warehouseHeight + 120;
		const int warehouseLeft = config.windWidth - warehouseWidth - 35;
		const int warehouseRight = warehouseLeft + warehouseWidth;
		const int warehouseBottom = fieldBottom - 25;
		const int warehouseTop = warehouseBottom - warehouseHeight;
		const int windowLeft = warehouseLeft + 14;
		const int windowRight = warehouseRight - 14;
		const int windowTop = warehouseTop + 52;
		const int windowBottom = warehouseBottom - 96;

		left = windowLeft + 10;
		top = windowTop + 10;
		right = windowRight - 10;
		bottom = windowBottom - 10;
	}

	void drawSellButton(window* pWind, int left, int top, bool enabled)
	{
		pWind->SetPen(enabled ? color(42, 104, 56) : color(116, 116, 116), 1);
		pWind->SetBrush(enabled ? color(96, 186, 101) : color(205, 205, 205));
		pWind->DrawRectangle(left, top, left + 31, top + 17, FILLED, 4, 4);
		pWind->SetPen(BLACK, 1);
		pWind->SetFont(10, BOLD, BY_NAME, "Arial");
		pWind->DrawString(left + 4, top + 3, "Sell");
	}

	bool isInsideSellButton(int x, int y, int left, int rowY)
	{
		const int sellButtonLeft = left + 109;
		const int sellButtonTop = rowY - 2;
		return x >= sellButtonLeft && x <= sellButtonLeft + 31 &&
			y >= sellButtonTop && y <= sellButtonTop + 17;
	}
}

Game::Game()
{
	wolfCount = 0;
	for (int i = 0; i < kMaxProducts; i++) {
		wolfList[i] = nullptr;
	}
	eggCount = 0;
	milkCount = 0;
	woolCount = 0;
	warehouseEgg = 0;
	warehouseMilk = 0;
	warehouseWool = 0;
	for (int i = 0; i < kMaxProducts; i++)
	{
		eggList[i] = nullptr;
		milkList[i] = nullptr;
		woolList[i] = nullptr;
	}

	budget = kStartingBudget;
	isPaused = false;
	timer = 60;
	level = 1;
	goal = 5;
	animals = 0;
	username = "Player";
	exitRequested = false;
	lastTime = GetTickCount();

	//1 - Create the main window
	pWind = CreateWind(config.windWidth, config.windHeight, config.wx, config.wy);
	pWind->SetBuffering(true);
	promptForUsername();

	pWind->SetPen(config.bkGrndColor, 1);
	pWind->SetBrush(config.bkGrndColor);
	pWind->DrawRectangle(0, 0, config.windWidth, config.windHeight);

	//2 - create and draw the toolbar
	createToolbar();
	createBudgetbar();
	//3 - create and draw the backgroundPlayingArea
	drawFieldBackground();
	drawWarehouse();


	//4- Create the Plane
	//TODO: Add code to create and draw the Plane

	//5- Create the Bullet
	//TODO: Add code to create and draw the Bullet

	//6- Create the enemies
	//TODO: Add code to create and draw enemies in random places

	//7- Create and clear the status bar
	clearStatusBar();
}

Game::~Game()
{
	for (int i = 0; i < eggCount; i++)
	{
		delete eggList[i];
	}

	for (int i = 0; i < milkCount; i++)
	{
		delete milkList[i];
	}

	for (int i = 0; i < woolCount; i++)
	{
		delete woolList[i];
	}

	clearWolves();

	delete gameToolbar;
	delete gameBudgetbar;
	delete pWind;
}

clicktype Game::getMouseClick(int& x, int& y) const
{
	return pWind->WaitMouseClick(x, y);	//Wait for mouse click

}

std::string Game::getSrting() const
{
	std::string Label;
	char Key;
	keytype ktype;
	pWind->FlushKeyQueue();
	while (1)
	{
		ktype = pWind->WaitKeyPress(Key);
		if (ktype == ESCAPE)	//ESCAPE key is pressed
			return "";	//returns nothing as user has cancelled label
		if (Key == 13)	//ENTER key is pressed
			return Label;
		if (Key == 8)	//BackSpace is pressed
			if (Label.size() > 0)
				Label.resize(Label.size() - 1);
			else
				Key = '\0';
		else
			Label += Key;
		printMessage(Label);
		pWind->UpdateBuffer();
	}
}

window* Game::CreateWind(int w, int h, int x, int y) const
{
	window* pW = new window(w, h, x, y);
	pW->SetWaitClose(false);
	pW->SetBrush(config.bkGrndColor);
	pW->SetPen(config.bkGrndColor, 1);
	pW->DrawRectangle(0, 0, w, h);
	return pW;
}

void Game::createToolbar() 
{
	point toolbarUpperleft;
	toolbarUpperleft.x = 0;
	toolbarUpperleft.y = 0;

	gameToolbar = new Toolbar(this, toolbarUpperleft, 0, config.toolBarHeight);
	gameToolbar->draw();
}

void Game::createBudgetbar()
{
	point budgetbarUpperleft;
	budgetbarUpperleft.x = 0;
	budgetbarUpperleft.y = config.toolBarHeight;

	gameBudgetbar = new Budgetbar(this, budgetbarUpperleft, 0, config.toolBarHeight);
	gameBudgetbar->draw();
}

void Game::drawFieldBackground() const
{
	const int fieldTop = 2 * config.toolBarHeight;
	const int fieldBottom = config.windHeight - config.statusBarHeight;

	pWind->SetPen(DARKGREEN, 3);
	pWind->SetBrush(color(168, 220, 120));
	pWind->DrawRectangle(0, fieldTop, config.windWidth, fieldBottom);

	pWind->SetPen(color(130, 94, 59), 4);
	pWind->SetBrush(color(206, 181, 120));
	pWind->DrawRectangle(
		config.fieldPadding,
		fieldTop + config.fieldPadding,
		config.windWidth - config.fieldPadding,
		fieldBottom - config.fieldPadding,
		FRAME
	);

	pWind->DrawLine(config.windWidth / 2, fieldTop + config.fieldPadding, config.windWidth / 2, fieldBottom - config.fieldPadding);
}

void Game::drawWarehouse() const
{
	const int fieldTop = 2 * config.toolBarHeight;
	const int fieldBottom = config.windHeight - config.statusBarHeight;
	const int warehouseWidth = config.warehouseWidth;
	const int warehouseHeight = config.warehouseHeight + 120;
	const int warehouseLeft = config.windWidth - warehouseWidth - 35;
	const int warehouseRight = warehouseLeft + warehouseWidth;
	const int warehouseBottom = fieldBottom - 25;
	const int warehouseTop = warehouseBottom - warehouseHeight;
	const int windowLeft = warehouseLeft + 14;
	const int windowRight = warehouseRight - 14;
	const int windowTop = warehouseTop + 52;
	const int windowBottom = warehouseBottom - 96;

	pWind->SetPen(BLACK, 3);
	pWind->SetBrush(color(196, 129, 79));
	pWind->DrawRectangle(warehouseLeft, warehouseTop, warehouseRight, warehouseBottom);

	int roofX[3] = { warehouseLeft - 10, warehouseLeft + (warehouseWidth / 2), warehouseRight + 10 };
	int roofY[3] = { warehouseTop, warehouseTop - 70, warehouseTop };
	pWind->SetBrush(color(145, 53, 40));
	pWind->DrawPolygon(roofX, roofY, 3);

	pWind->SetBrush(color(104, 67, 40));
	pWind->DrawRectangle(warehouseLeft + 56, warehouseBottom - 88, warehouseLeft + 126, warehouseBottom);

	pWind->SetPen(color(96, 68, 45), 3);
	pWind->SetBrush(color(188, 225, 255));
	pWind->DrawRectangle(windowLeft, windowTop, windowRight, windowBottom, FILLED, 10, 10);
	drawWarehouseInventory(windowLeft + 10, windowTop + 10, windowRight - 10, windowBottom - 10);

	pWind->SetPen(BLACK, 2);
	pWind->SetFont(22, BOLD, BY_NAME, "Arial");
	pWind->DrawString(warehouseLeft + 28, warehouseTop + 20, "Warehouse");
}

void Game::drawWarehouseInventory(int left, int top, int right, int bottom) const
{
	const int itemX = left + 12;
	const int countX = left + 50;
	const int priceX = left + 81;
	const int sellX = left + 111;
	const int row1Y = top + 34;
	const int rowGap = 28;

	pWind->SetPen(color(120, 86, 54), 2);
	pWind->SetBrush(color(248, 239, 214));
	pWind->DrawRectangle(left, top, right, bottom);

	pWind->SetPen(color(147, 106, 68), 1);
	pWind->DrawLine(left, top + 26, right, top + 26);
	pWind->DrawLine(countX - 8, top + 2, countX - 8, bottom - 2);
	pWind->DrawLine(priceX - 8, top + 2, priceX - 8, bottom - 2);
	pWind->DrawLine(sellX - 4, top + 2, sellX - 4, bottom - 2);

	pWind->SetPen(BLACK, 1);
	pWind->SetFont(11, BOLD, BY_NAME, "Arial");
	pWind->DrawString(itemX, top + 7, "Item");
	pWind->DrawString(countX, top + 7, "Count");
	pWind->DrawString(priceX, top + 7, "Price");
	pWind->DrawString(sellX, top + 7, "Sell");

	pWind->SetFont(12, PLAIN, BY_NAME, "Arial");
	pWind->DrawString(itemX, row1Y, "Eggs");
	pWind->DrawString(countX, row1Y, to_string(warehouseEgg));
	pWind->DrawString(priceX, row1Y, to_string(kEggPrice) + "$");
	drawSellButton(pWind, left + 109, row1Y - 2, warehouseEgg > 0);

	pWind->DrawString(itemX, row1Y + rowGap, "Milk");
	pWind->DrawString(countX, row1Y + rowGap, to_string(warehouseMilk));
	pWind->DrawString(priceX, row1Y + rowGap, to_string(kMilkPrice) + "$");
	drawSellButton(pWind, left + 109, row1Y + rowGap - 2, warehouseMilk > 0);

	pWind->DrawString(itemX, row1Y + (2 * rowGap), "Wool");
	pWind->DrawString(countX, row1Y + (2 * rowGap), to_string(warehouseWool));
	pWind->DrawString(priceX, row1Y + (2 * rowGap), to_string(kWoolPrice) + "$");
	drawSellButton(pWind, left + 109, row1Y + (2 * rowGap) - 2, warehouseWool > 0);
}

void Game::drawFoodArea() const
{
	drawFieldBackground();
	drawWarehouse();

	const int fieldTop = (2 * config.toolBarHeight) + config.fieldPadding;
	pWind->SetPen(color(102, 150, 78), 3);
	pWind->SetBrush(color(192, 230, 150));
	pWind->DrawCircle(kFeedingAreaCenterX, kFeedingAreaCenterY, kFeedingAreaRadius);

	pWind->SetPen(DARKGREEN, 2);
	pWind->SetFont(20, BOLD, BY_NAME, "Arial");
	pWind->DrawString(25, fieldTop + 10, "Feeding Area");
}


void Game::clearBudget() const
{
	//Clear Status bar by drawing a filled rectangle
	pWind->SetPen(config.bkGrndColor, 1);
	pWind->SetBrush(config.bkGrndColor);
	pWind->DrawRectangle(config.windWidth - 500, config.toolBarHeight, config.windWidth, 2*config.toolBarHeight);
}

void Game::printBudget(std::string msg) const
{
	clearBudget();	//First clear the status bar

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(config.windWidth-200, config.toolBarHeight + 10, msg);

}

bool Game::spendBudget(int amount)
{
	if (budget < amount)
	{
		printMessage("Not enough budget");
		return false;
	}

	budget -= amount;
	printBudget("BUDGET = $" + to_string(budget));
	return true;
}

void Game::clearStatusBar() const
{
	//Clear Status bar by drawing a filled rectangle
	pWind->SetPen(config.statusBarColor, 1);
	pWind->SetBrush(config.statusBarColor);
	pWind->DrawRectangle(0, config.windHeight - config.statusBarHeight, config.windWidth, config.windHeight);
}

void Game::printMessage(std::string msg) const
{
	clearStatusBar();	//First clear the status bar

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(10, config.windHeight - (int)(0.85 * config.statusBarHeight), msg);

}

void Game::updateStatusBar() const
{
	clearStatusBar();

	std::string msg = "Player: " + username +
		" | Timer: " + to_string(timer) +
		" | Score: " + to_string(calculateScore()) +
		" | Goal: " + to_string(goal) +
		" | Level: " + to_string(level) +
		" | Animals: " + to_string(animals);

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(10, config.windHeight - (int)(0.85 * config.statusBarHeight), msg);
}

void Game::promptForUsername()
{
	std::string enteredName = getUsernameFromStartScreen();
	if (enteredName == kQuitUserName)
	{
		exitRequested = true;
		return;
	}

	if (enteredName.empty())
	{
		enteredName = "Player";
	}
	sanitizeLeaderboardName(enteredName);

	exitRequested = false;
	username = enteredName;
	drawStartScreen(username);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->SetPen(color(19, 84, 45), 1);
	pWind->DrawString(430, 398, "Welcome, " + username + "!");
	pWind->UpdateBuffer();
	Sleep(700);
}

void Game::drawStartScreen(const std::string& typedName, bool isGameOver, int lastScore) const
{
	pWind->SetPen(color(111, 178, 230), 1);
	pWind->SetBrush(color(111, 178, 230));
	pWind->DrawRectangle(0, 0, config.windWidth, config.windHeight);

	pWind->SetPen(color(255, 205, 80), 1);
	pWind->SetBrush(color(255, 220, 95));
	pWind->DrawCircle(1015, 95, 54);

	pWind->SetPen(color(68, 145, 61), 1);
	pWind->SetBrush(color(96, 181, 82));
	pWind->DrawRectangle(0, 335, config.windWidth, config.windHeight);

	pWind->SetPen(color(74, 133, 58), 3);
	for (int x = 0; x < config.windWidth; x += 60)
	{
		pWind->DrawLine(x, 600, x + 190, 335);
	}

	pWind->SetPen(BLACK, 3);
	pWind->SetBrush(color(194, 91, 62));
	pWind->DrawRectangle(112, 265, 300, 405);
	int roofX[3] = { 90, 206, 322 };
	int roofY[3] = { 265, 180, 265 };
	pWind->SetBrush(color(130, 48, 39));
	pWind->DrawPolygon(roofX, roofY, 3);
	pWind->SetBrush(color(99, 64, 42));
	pWind->DrawRectangle(172, 325, 240, 405);
	pWind->SetBrush(color(245, 222, 150));
	pWind->DrawRectangle(130, 288, 174, 326);
	pWind->DrawRectangle(238, 288, 282, 326);

	pWind->SetPen(color(37, 92, 48), 1);
	pWind->SetFont(58, BOLD, BY_NAME, "Arial");
	pWind->DrawString(360, 62, "FARM FRENZY");

	pWind->SetFont(22, BOLD, BY_NAME, "Arial");
	if (isGameOver)
	{
		pWind->SetPen(color(128, 40, 34), 1);
		pWind->SetFont(38, BOLD, BY_NAME, "Arial");
		pWind->DrawString(448, 126, "GAME OVER");
		pWind->SetPen(BLACK, 1);
		pWind->SetFont(21, BOLD, BY_NAME, "Arial");
		pWind->DrawString(400, 180, username + "'s score: " + to_string(lastScore));
	}
	else
	{
		pWind->DrawString(405, 138, "Build your farm. Collect products. Beat the board.");
	}

	if (isGameOver)
	{
		pWind->SetPen(color(42, 105, 55), 3);
		pWind->SetBrush(color(248, 239, 214));
		pWind->DrawRectangle(365, 235, 790, 382, FILLED, 16, 16);

		pWind->SetPen(color(37, 92, 48), 1);
		pWind->SetFont(24, BOLD, BY_NAME, "Arial");
		pWind->DrawString(448, 260, "Score Saved");

		pWind->SetPen(BLACK, 1);
		pWind->SetFont(19, BOLD, BY_NAME, "Arial");
		pWind->DrawString(420, 305, "Player: " + username);
		pWind->DrawString(420, 335, "Final Score: " + to_string(lastScore));
	}
	else
	{
		pWind->SetPen(color(42, 105, 55), 3);
		pWind->SetBrush(color(248, 239, 214));
		pWind->DrawRectangle(365, 225, 790, 392, FILLED, 16, 16);

		pWind->SetPen(color(37, 92, 48), 1);
		pWind->SetFont(24, BOLD, BY_NAME, "Arial");
		pWind->DrawString(438, 250, "Enter your username");

		pWind->SetPen(color(120, 86, 54), 3);
		pWind->SetBrush(WHITE);
		pWind->DrawRectangle(410, 298, 745, 345, FILLED, 8, 8);

		pWind->SetPen(BLACK, 1);
		pWind->SetFont(23, BOLD, BY_NAME, "Arial");
		if (typedName.empty())
		{
			pWind->SetPen(color(120, 120, 120), 1);
			pWind->DrawString(430, 310, "Player name");
		}
		else
		{
			pWind->DrawString(430, 310, typedName);
		}

		pWind->SetPen(color(37, 92, 48), 1);
		pWind->SetFont(16, BOLD, BY_NAME, "Arial");
		pWind->DrawString(470, 358, "Press Enter to start");
	}

	std::vector<std::pair<std::string, int>> leaderboard = loadLeaderboard();
	std::sort(leaderboard.begin(), leaderboard.end(), compareScoresDescending);
	drawStartLeaderboard(860, 170, leaderboard);

	if (isGameOver)
	{
		pWind->SetPen(color(37, 92, 48), 3);
		pWind->SetBrush(color(255, 238, 170));
		pWind->DrawRectangle(310, 430, 500, 482, FILLED, 12, 12);
		pWind->SetBrush(color(235, 250, 230));
		pWind->DrawRectangle(520, 430, 750, 482, FILLED, 12, 12);
		pWind->SetBrush(color(245, 214, 205));
		pWind->DrawRectangle(475, 500, 585, 548, FILLED, 12, 12);

		pWind->SetPen(BLACK, 1);
		pWind->SetFont(18, BOLD, BY_NAME, "Arial");
		pWind->DrawString(358, 447, "Play Again");
		pWind->DrawString(547, 447, "Different User");
		pWind->DrawString(508, 515, "Quit");
	}
	else
	{
		pWind->SetPen(color(128, 40, 34), 3);
		pWind->SetBrush(color(245, 214, 205));
		pWind->DrawRectangle(505, 445, 650, 497, FILLED, 12, 12);

		pWind->SetPen(BLACK, 1);
		pWind->SetFont(18, BOLD, BY_NAME, "Arial");
		pWind->DrawString(553, 462, "Quit");
	}
}

void Game::drawStartLeaderboard(int left, int top, const std::vector<std::pair<std::string, int>>& leaderboard) const
{
	pWind->SetPen(color(76, 95, 58), 3);
	pWind->SetBrush(color(248, 239, 214));
	pWind->DrawRectangle(left, top, left + 285, top + 315, FILLED, 10, 10);

	pWind->SetPen(color(37, 92, 48), 1);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(left + 62, top + 18, "Leaderboard");

	pWind->SetFont(14, BOLD, BY_NAME, "Arial");
	pWind->SetPen(BLACK, 1);
	pWind->DrawString(left + 20, top + 65, "Rank");
	pWind->DrawString(left + 80, top + 65, "Player");
	pWind->DrawString(left + 210, top + 65, "Best");

	pWind->SetPen(color(111, 75, 42), 2);
	pWind->DrawLine(left + 15, top + 88, left + 270, top + 88);

	int rowsToDraw = static_cast<int>(leaderboard.size());
	if (rowsToDraw > 8)
	{
		rowsToDraw = 8;
	}

	if (rowsToDraw == 0)
	{
		pWind->SetFont(15, BOLD, BY_NAME, "Arial");
		pWind->DrawString(left + 62, top + 145, "No scores yet");
		return;
	}

	for (int i = 0; i < rowsToDraw; i++)
	{
		std::string nameText = leaderboard[i].first;
		if (nameText.size() > 13)
		{
			nameText = nameText.substr(0, 13);
		}

		const int rowY = top + 108 + (i * 25);
		pWind->SetFont(14, PLAIN, BY_NAME, "Arial");
		if (leaderboard[i].first == username)
		{
			pWind->SetPen(color(26, 109, 57), 1);
			pWind->SetFont(14, BOLD, BY_NAME, "Arial");
		}
		else
		{
			pWind->SetPen(BLACK, 1);
		}

		pWind->DrawString(left + 30, rowY, to_string(i + 1));
		pWind->DrawString(left + 80, rowY, nameText);
		pWind->DrawString(left + 215, rowY, to_string(leaderboard[i].second));
	}
}

std::string Game::getUsernameFromStartScreen() const
{
	std::string typedName;
	char key;
	keytype keyType;
	int x = 0;
	int y = 0;

	pWind->FlushKeyQueue();
	pWind->FlushMouseQueue();
	drawStartScreen(typedName);
	pWind->UpdateBuffer();

	while (true)
	{
		if (pWind->GetButtonState(LEFT_BUTTON, x, y))
		{
			if (x >= 505 && x <= 650 && y >= 445 && y <= 497)
			{
				while (pWind->GetButtonState(LEFT_BUTTON, x, y))
				{
					Sleep(10);
				}
				return kQuitUserName;
			}

			while (pWind->GetButtonState(LEFT_BUTTON, x, y))
			{
				Sleep(10);
			}
		}

		keyType = pWind->GetKeyPress(key);
		if (keyType != NO_KEYPRESS)
		{
			if (keyType == ESCAPE)
			{
				return "";
			}
			if (key == 13)
			{
				return typedName;
			}
			if (key == 8)
			{
				if (!typedName.empty())
				{
					typedName.resize(typedName.size() - 1);
				}
			}
			else if (key >= 32 && key <= 126 && typedName.size() < 18)
			{
				typedName += key;
			}

			drawStartScreen(typedName);
			pWind->UpdateBuffer();
		}

		Sleep(20);
	}
}

int Game::calculateScore() const
{
	return (warehouseEgg * kEggPrice) + (warehouseMilk * kMilkPrice) + (warehouseWool * kWoolPrice);
}

std::vector<std::pair<std::string, int>> Game::loadLeaderboard() const
{
	std::vector<std::pair<std::string, int>> leaderboard;
	std::ifstream input(kLeaderboardFileName);
	std::string line;

	while (std::getline(input, line))
	{
		std::stringstream parser(line);
		std::string name;
		int score = 0;

		if (std::getline(parser, name, ',') && parser >> score && !name.empty())
		{
			leaderboard.push_back(std::make_pair(name, score));
		}
	}

	return leaderboard;
}

void Game::saveLeaderboard(const std::vector<std::pair<std::string, int>>& leaderboard) const
{
	std::ofstream output(kLeaderboardFileName);
	for (size_t i = 0; i < leaderboard.size(); i++)
	{
		output << leaderboard[i].first << "," << leaderboard[i].second << std::endl;
	}
}

void Game::updateLeaderboard(int score)
{
	std::vector<std::pair<std::string, int>> leaderboard = loadLeaderboard();
	bool foundUser = false;

	for (size_t i = 0; i < leaderboard.size(); i++)
	{
		if (leaderboard[i].first == username)
		{
			if (score > leaderboard[i].second)
			{
				leaderboard[i].second = score;
			}
			foundUser = true;
			break;
		}
	}

	if (!foundUser)
	{
		leaderboard.push_back(std::make_pair(username, score));
	}

	std::sort(leaderboard.begin(), leaderboard.end(), compareScoresDescending);
	saveLeaderboard(leaderboard);
}

Game::GameOverChoice Game::handleGameOver()
{
	int score = calculateScore();
	updateLeaderboard(score);
	drawStartScreen(username, true, score);
	pWind->UpdateBuffer();

	int x = 0;
	int y = 0;
	pWind->FlushMouseQueue();
	while (pWind->GetButtonState(LEFT_BUTTON, x, y))
	{
		Sleep(10);
	}

	while (pWind->IsOpen())
	{
		pWind->WaitMouseClick(x, y);
		if (x >= 310 && x <= 500 && y >= 430 && y <= 482)
		{
			return PLAY_AGAIN;
		}
		if (x >= 520 && x <= 750 && y >= 430 && y <= 482)
		{
			return DIFFERENT_USER;
		}
		if (x >= 475 && x <= 585 && y >= 500 && y <= 548)
		{
			return EXIT_GAME;
		}
	}

	return EXIT_GAME;
}

void Game::DrawProducts() const
{
	for (int i = 0; i < eggCount; i++)
	{
		if (eggList[i] != nullptr)
		{
			eggList[i]->draw();
		}
	}

	for (int i = 0; i < milkCount; i++)
	{
		if (milkList[i] != nullptr)
		{
			milkList[i]->draw();
		}
	}

	for (int i = 0; i < woolCount; i++)
	{
		if (woolList[i] != nullptr)
		{
			woolList[i]->draw();
		}
	}
}

void Game::clearProducts()
{
	for (int i = 0; i < eggCount; i++)
	{
		delete eggList[i];
		eggList[i] = nullptr;
	}

	for (int i = 0; i < milkCount; i++)
	{
		delete milkList[i];
		milkList[i] = nullptr;
	}

	for (int i = 0; i < woolCount; i++)
	{
		delete woolList[i];
		woolList[i] = nullptr;
	}

	eggCount = 0;
	milkCount = 0;
	woolCount = 0;
}

void Game::clearWolves()
{
	for (int i = 0; i < wolfCount; i++)
	{
		delete wolfList[i];
		wolfList[i] = nullptr;
	}

	wolfCount = 0;
}

bool Game::isProductAreaFree(const point& location, int width, int height) const
{
	const int fieldLeft = config.fieldPadding;
	const int fieldTop = (2 * config.toolBarHeight) + config.fieldPadding;
	const int fieldRight = config.windWidth - config.fieldPadding;
	const int fieldBottom = config.windHeight - config.statusBarHeight - config.fieldPadding;

	if (location.x < fieldLeft || location.y < fieldTop || location.x + width > fieldRight || location.y + height > fieldBottom)
	{
		return false;
	}

	for (int i = 0; i < eggCount; i++)
	{
		if (eggList[i] != nullptr && rectanglesOverlap(location, width, height, eggList[i]->getRefPoint(), 30, 30))
		{
			return false;
		}
	}

	for (int i = 0; i < milkCount; i++)
	{
		if (milkList[i] != nullptr && rectanglesOverlap(location, width, height, milkList[i]->getRefPoint(), 40, 40))
		{
			return false;
		}
	}

	for (int i = 0; i < woolCount; i++)
	{
		if (woolList[i] != nullptr && rectanglesOverlap(location, width, height, woolList[i]->getRefPoint(), 36, 28))
		{
			return false;
		}
	}

	return true;
}

bool Game::findFreeProductSpot(point& location, int width, int height) const
{
	static const int offsets[][2] =
	{
		{0, 0}, {20, 0}, {-20, 0}, {0, 20}, {0, -20},
		{20, 20}, {-20, 20}, {20, -20}, {-20, -20},
		{40, 0}, {-40, 0}, {0, 40}, {0, -40}
	};

	point baseLocation = location;
	for (int i = 0; i < static_cast<int>(sizeof(offsets) / sizeof(offsets[0])); i++)
	{
		point candidate;
		candidate.x = baseLocation.x + offsets[i][0];
		candidate.y = baseLocation.y + offsets[i][1];
		if (isProductAreaFree(candidate, width, height))
		{
			location = candidate;
			return true;
		}
	}

	return false;
}

void Game::updateTimer()
{
	unsigned long currentTime = GetTickCount();

	if (currentTime - lastTime >= 1000 && timer > 0)
	{
		timer--;
		lastTime = currentTime;
	}
}

void Game::handleProductClick(int x, int y)
{
	for (int i = 0; i < wolfCount; i++)
	{
		if (wolfList[i] != nullptr)
		{
			point p = wolfList[i]->getRefPoint();
			if (x >= p.x && x <= p.x + 50 && y >= p.y && y <= p.y + 50)
			{
				if (wolfList[i]->clicked())
				{
					delete wolfList[i];
					wolfList[i] = wolfList[wolfCount - 1];
					wolfList[wolfCount - 1] = nullptr;
					wolfCount--;
					printMessage("Wolf got disappeared");
				}
				else
				{
					printMessage("Keep clicking");
				}
				return;
			}
		}
	}
	for (int i = 0; i < eggCount; i++)
	{
		if (eggList[i] != nullptr && eggList[i]->isClicked(x, y))
		{
			warehouseEgg++;
			delete eggList[i];
			eggList[i] = eggList[eggCount - 1];
			eggList[eggCount - 1] = nullptr;
			eggCount--;
			printMessage("Egg in warehouse: " + to_string(warehouseEgg));
			return;
		}
	}

	for (int i = 0; i < milkCount; i++)
	{
		if (milkList[i] != nullptr && milkList[i]->isClicked(x, y))
		{
			warehouseMilk++;
			delete milkList[i];
			milkList[i] = milkList[milkCount - 1];
			milkList[milkCount - 1] = nullptr;
			milkCount--;
			printMessage("Milk in warehouse: " + to_string(warehouseMilk));
			return;
		}
	}

	for (int i = 0; i < woolCount; i++)
	{
		if (woolList[i] != nullptr && woolList[i]->isClicked(x, y))
		{
			warehouseWool++;
			delete woolList[i];
			woolList[i] = woolList[woolCount - 1];
			woolList[woolCount - 1] = nullptr;
			woolCount--;
			printMessage("Wool in warehouse: " + to_string(warehouseWool));
			return;
		}
	}
}

bool Game::handleWarehouseClick(int x, int y)
{
	int left, top, right, bottom;
	getWarehouseInventoryBounds(left, top, right, bottom);

	if (x < left || x > right || y < top || y > bottom)
	{
		return false;
	}

	const int row1Y = top + 34;
	const int rowGap = 28;
	if (isInsideSellButton(x, y, left, row1Y))
	{
		sellWarehouseProduct(warehouseEgg, kEggPrice, "Egg");
		return true;
	}

	if (isInsideSellButton(x, y, left, row1Y + rowGap))
	{
		sellWarehouseProduct(warehouseMilk, kMilkPrice, "Milk");
		return true;
	}

	if (isInsideSellButton(x, y, left, row1Y + (2 * rowGap)))
	{
		sellWarehouseProduct(warehouseWool, kWoolPrice, "Wool");
		return true;
	}

	return false;
}

void Game::sellWarehouseProduct(int& productCount, int price, const std::string& productName)
{
	if (productCount <= 0)
	{
		printMessage("No " + productName + " to sell");
		return;
	}

	productCount--;
	budget += price;
	printBudget("BUDGET = $" + to_string(budget));
	drawWarehouse();
	printMessage(productName + " sold for $" + to_string(price));
}

void Game::restartGame()
{
	resetGameState();
	pWind->SetPen(config.bkGrndColor, 1);
	pWind->SetBrush(config.bkGrndColor);
	pWind->DrawRectangle(0, 0, config.windWidth, config.windHeight);
	gameToolbar->draw();
	gameBudgetbar->draw();
	drawFieldBackground();
	drawWarehouse();
	clearStatusBar();
	printBudget("BUDGET = $" + to_string(budget));
	printMessage("Game restarted for " + username);
}

void Game::resetGameState()
{
	clearProducts();
	clearWolves();
	budget = kStartingBudget;
	isPaused = false;
	timer = 60;
	level = 1;
	goal = 5;
	animals = 0;
	warehouseEgg = 0;
	warehouseMilk = 0;
	warehouseWool = 0;
	lastTime = GetTickCount();
	gameBudgetbar->resetAnimals();
}

void Game::pauseGame()
{
	isPaused = true;
	printMessage("Game paused");
}

void Game::resumeGame()
{
	isPaused = false;
	printMessage("Game resumed");
}

void Game::saveGame() const
{
	printMessage("Save is ready to be connected to file logic");
}

void Game::loadGame()
{
	printMessage("Load is ready to be connected to file logic");
}

bool Game::addEgg(point location)
{
	if (!findFreeProductSpot(location, 30, 30))
	{
		return false;
	}

	if (eggCount >= kMaxProducts)
	{
		return false;
	}

	eggList[eggCount] = new Egg(this, location);
	eggCount++;
	return true;
}

bool Game::addMilk(point location)
{
	if (!findFreeProductSpot(location, 40, 40))
	{
		return false;
	}

	if (milkCount >= kMaxProducts)
	{
		return false;
	}

	milkList[milkCount] = new Milk(this, location);
	milkCount++;
	return true;
}

bool Game::addWool(point location)
{
	if (!findFreeProductSpot(location, 36, 28))
	{
		return false;
	}

	if (woolCount >= kMaxProducts)
	{
		return false;
	}

	woolList[woolCount] = new Wool(this, location);
	woolCount++;
	return true;
}

window* Game::getWind() const
{
	return pWind;
}
int Game::getLevel() const
{
	return level;
}
Budgetbar* Game::getBudgetbar() const
{
	return gameBudgetbar;
}

void Game::go()
{
	int x = 0, y = 0;
	bool isExit = false;

	pWind->ChangeTitle("- - - - - - - - - - Farm Frenzy (CIE101-project) - - - - - - - - - -");
	if (exitRequested)
	{
		return;
	}

	do
	{
		if (!pWind->IsOpen())
		{
			break;
		}

		printBudget("BUDGET = $" + to_string(budget));

		if (!isPaused)
		{
			updateTimer();
			if (timer <= 0)
			{
				GameOverChoice choice = handleGameOver();
				if (choice == PLAY_AGAIN)
				{
					restartGame();
				}
				else if (choice == DIFFERENT_USER)
				{
					promptForUsername();
					if (exitRequested)
					{
						isExit = true;
					}
					else
					{
						restartGame();
					}
				}
				else
				{
					isExit = true;
				}
				continue;
			}

			drawFoodArea();
			DrawProducts();
			showRandomWolf();
			for (int i = 0; i < wolfCount; i++) {
				if (wolfList[i] != nullptr) {
					wolfList[i]->moveStep();
				}
			}
			gameBudgetbar->updateAnimals();
			animals = gameBudgetbar->getAnimalCount();
			updateStatusBar();
		}

		pWind->UpdateBuffer();

		Sleep(60);

		if (pWind->GetButtonState(LEFT_BUTTON, x, y))
		{
			if (y >= 0 && y < config.toolBarHeight)
			{
				isExit = gameToolbar->handleClick(x, y);
			}
			else if (y >= config.toolBarHeight && y < 2 * config.toolBarHeight)
			{
				isExit = gameBudgetbar->handleClick(x, y);
			}
			else if (y >= 2 * config.toolBarHeight)
			{
				if (!handleWarehouseClick(x, y))
				{
					handleProductClick(x, y);
				}
			}

			while (pWind->GetButtonState(LEFT_BUTTON, x, y))
			{
				Sleep(10);
			}

			if (isExit && pWind->IsOpen())
			{
				GameOverChoice choice = handleGameOver();
				if (choice == PLAY_AGAIN)
				{
					restartGame();
					isExit = false;
				}
				else if (choice == DIFFERENT_USER)
				{
					promptForUsername();
					if (exitRequested)
					{
						isExit = true;
					}
					else
					{
						restartGame();
						isExit = false;
					}
				}
			}
		}
	} while (!isExit && pWind->IsOpen());
}
void Game::showRandomWolf() {
	int showChance = 30 + (level * 5);

	if (wolfCount < kMaxProducts && (rand() % 10000) < showChance) {
		int x = config.fieldPadding + (rand() % (config.windWidth - 2 * config.fieldPadding));
		int usableHeight = config.playingAreaHeight - 2 * config.fieldPadding; 
		int y = (config.toolBarHeight + config.fieldPadding) + (rand() % usableHeight);
		point p = { x, y };
		wolfList[wolfCount] = new Wolf(this, p, 50, 50, "images\\wolf.JPEG");
		wolfCount++;
	}
}
