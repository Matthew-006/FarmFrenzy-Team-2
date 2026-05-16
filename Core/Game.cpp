#include "Game.h"
#include "../Config/GameConfig.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Product.h"
#include "../Entities/Animal.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <windows.h>   // ـ Beep
namespace
{
	const int kFeedingAreaCenterX = 170;
	const int kFeedingAreaCenterY = (2 * config.toolBarHeight) + 145;
	const int kFeedingAreaRadius = 95;
	const unsigned long long kFoodConsumeDelayMs = 650;
	const unsigned long long kHelperDurationMs = 60000;
	const char* kQuitUserName = "__QUIT__";
	const char* kLeaderboardFileName = "leaderboard.txt";
	const char* kLegacySaveGameFileName = "savegame.txt";

	std::string makeSaveGameFileName(const std::string& playerName)
	{
		std::string safeName;
		for (size_t i = 0; i < playerName.size(); i++)
		{
			const char ch = playerName[i];
			const bool isLetter = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
			const bool isDigit = ch >= '0' && ch <= '9';
			if (isLetter || isDigit)
			{
				safeName += ch;
			}
			else if (!safeName.empty() && safeName[safeName.size() - 1] != '_')
			{
				safeName += '_';
			}
		}

		if (safeName.empty())
		{
			safeName = "Player";
		}

		return "savegame_" + safeName + ".txt";
	}

	bool rectanglesOverlap(const point& firstPoint, int firstWidth, int firstHeight, const point& secondPoint, int secondWidth, int secondHeight)
	{
		return firstPoint.x < secondPoint.x + secondWidth &&
			firstPoint.x + firstWidth > secondPoint.x &&
			firstPoint.y < secondPoint.y + secondHeight &&
			firstPoint.y + firstHeight > secondPoint.y;
	}

	template <typename AnimalType>
	bool feedCollidingAnimal(AnimalType* const* animalList, int animalCount, int animalWidth, int animalHeight, const point& foodPos, const std::string& animalName, Game* game)
	{
		for (int i = 0; i < animalCount; i++)
		{
			if (animalList[i] != nullptr && rectanglesOverlap(animalList[i]->getRefPoint(), animalWidth, animalHeight, foodPos, 50, 50))
			{
				animalList[i]->increaseFoodCounter();
				game->printMessage(animalName + " ate! Total: " + std::to_string(animalList[i]->getFoodCounter()));
				return true;
			}
		}

		return false;
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

	void getWarehouseBounds(int& left, int& top, int& right, int& bottom)
	{
		const int fieldBottom = config.windHeight - config.statusBarHeight;
		const int warehouseWidth = config.warehouseWidth;
		const int warehouseHeight = config.warehouseHeight + 120;
		const int warehouseLeft = config.windWidth - warehouseWidth - 35;
		const int warehouseRight = warehouseLeft + warehouseWidth;
		const int warehouseBottom = fieldBottom - 25;
		const int warehouseTop = warehouseBottom - warehouseHeight;

		left = warehouseLeft - 10;
		top = warehouseTop - 70;
		right = warehouseRight + 10;
		bottom = warehouseBottom;
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

	template <typename ProductType>
	void writeProductListWithType(std::ofstream& output, const std::string& countLabel, const std::string& productType, ProductType* const* list, int count)
	{
		output << countLabel << " " << count << std::endl;
		for (int i = 0; i < count; i++)
		{
			if (list[i] != nullptr)
			{
				point p = list[i]->getRefPoint();
				output << productType << " " << p.x << " " << p.y << std::endl;
			}
		}
	}

	template <typename AnimalType>
	void writeAnimalList(std::ofstream& output, const std::string& label, AnimalType* const* list, int count)
	{
		output << label << " " << count << std::endl;
		for (int i = 0; i < count; i++)
		{
			if (list[i] != nullptr)
			{
				point p = list[i]->getRefPoint();
				output << p.x << " " << p.y << " "
					<< list[i]->getDx() << " " << list[i]->getDy() << " "
					<< list[i]->getChangeCounter() << std::endl;
			}
		}
	}

	template <typename AnimalType>
	void writeAnimalRows(std::ofstream& output, const std::string& animalType, AnimalType* const* list, int count)
	{
		for (int i = 0; i < count; i++)
		{
			if (list[i] != nullptr)
			{
				point p = list[i]->getRefPoint();
				output << animalType << " " << p.x << " " << p.y << " "
					<< list[i]->getDx() << " " << list[i]->getDy() << " "
					<< list[i]->getChangeCounter() << std::endl;
			}
		}
	}

	std::string trimCopy(const std::string& value)
	{
		size_t first = value.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
		{
			return "";
		}

		size_t last = value.find_last_not_of(" \t\r\n");
		return value.substr(first, last - first + 1);
	}

	bool readDataLine(std::ifstream& input, std::string& line)
	{
		while (std::getline(input, line))
		{
			size_t commentStart = line.find("//");
			if (commentStart != std::string::npos)
			{
				line = line.substr(0, commentStart);
			}

			line = trimCopy(line);
			if (!line.empty())
			{
				return true;
			}
		}

		return false;
	}

	bool parseLabelIntLine(const std::string& line, const std::string& expectedLabel, int& value)
	{
		std::stringstream parser(line);
		std::string label;
		parser >> label >> value;
		return parser && label == expectedLabel;
	}

	bool readLabelIntLine(std::ifstream& input, const std::string& expectedLabel, int& value)
	{
		std::string line;
		return readDataLine(input, line) && parseLabelIntLine(line, expectedLabel, value);
	}

	bool readSectionLine(std::ifstream& input, const std::string& expectedLabel)
	{
		std::string line;
		if (!readDataLine(input, line))
		{
			return false;
		}

		std::stringstream parser(line);
		std::string label;
		parser >> label;
		return parser && label == expectedLabel;
	}

	template <typename ProductType>
	bool readTypedProductList(std::ifstream& input, const std::string& countLabel, const std::string& rowLabel, ProductType** list, int& count, int maxCount, Game* game)
	{
		if (!readLabelIntLine(input, countLabel, count) || count < 0 || count > maxCount)
		{
			return false;
		}

		for (int i = 0; i < count; i++)
		{
			std::string line;
			if (!readDataLine(input, line))
			{
				return false;
			}

			std::stringstream parser(line);
			std::string label;
			point p;
			parser >> label >> p.x >> p.y;
			if (!parser || label != rowLabel)
			{
				return false;
			}

			list[i] = new ProductType(game, p);
		}

		return true;
	}

	template <typename AnimalType>
	bool loadAnimalRow(const std::string& line, const std::string& expectedLabel, AnimalType** list, int& count, int maxCount, Game* game, const std::string& imagePath)
	{
		if (count >= maxCount)
		{
			return false;
		}

		std::stringstream parser(line);
		std::string label;
		point p;
		int dx = 0;
		int dy = 0;
		int changeCounter = 0;
		parser >> label >> p.x >> p.y >> dx >> dy >> changeCounter;
		if (!parser || label != expectedLabel)
		{
			return false;
		}

		list[count] = new AnimalType(game, p, 50, 50, imagePath);
		list[count]->setDx(dx);
		list[count]->setDy(dy);
		list[count]->setChangeCounter(changeCounter);
		count++;
		return true;
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
	timer = 90;
	level = 1;
	goal = 700;
	score = 0;
	animals = 0;
	username = "Player";
	exitRequested = false;
	soundMuted = false;
	helperActive = false;
	helperFarmer = nullptr;
	lastTime = GetTickCount64();

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
	startBackgroundMusic();
	
}

Game::~Game()
{
	stopBackgroundMusic();

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
	delete helperFarmer;

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
void Game::handleFeedingLogic()
{
	WaterIcon* waterIcon = gameBudgetbar->getWaterIcon();
	unsigned long long currentTick = GetTickCount64();

	for (int j = 0; j < waterIcon->count; j++)
	{
		if (waterIcon->waterList[j] == nullptr) continue;

		if (waterIcon->grassTileCounts[j] <= 0)
		{
			delete waterIcon->waterList[j];
			waterIcon->waterList[j] = waterIcon->waterList[waterIcon->count - 1];
			waterIcon->grassTileCounts[j] = waterIcon->grassTileCounts[waterIcon->count - 1];
			waterIcon->grassLastDecayTick[j] = waterIcon->grassLastDecayTick[waterIcon->count - 1];
			waterIcon->waterList[waterIcon->count - 1] = nullptr;
			waterIcon->count--;
			j--;
			continue;
		}

		if (currentTick - waterIcon->grassLastDecayTick[j] < kFoodConsumeDelayMs)
		{
			continue;
		}

		point foodPos = waterIcon->waterList[j]->getRefPoint();
		ChickIcon* chickIcon = gameBudgetbar->getChickIcon();
		CowIcon* cowIcon = gameBudgetbar->getCowIcon();
		SheepIcon* sheepIcon = gameBudgetbar->getSheepIcon();
		GoatIcon* goatIcon = gameBudgetbar->getGoatIcon();
		DuckIcon* duckIcon = gameBudgetbar->getDuckIcon();

		bool animalAte =
			feedCollidingAnimal(chickIcon->chickList, chickIcon->count, 50, 50, foodPos, "Chick", this) ||
			feedCollidingAnimal(cowIcon->cowList, cowIcon->count, 50, 50, foodPos, "Cow", this) ||
			feedCollidingAnimal(sheepIcon->sheepList, sheepIcon->count, 50, 50, foodPos, "Sheep", this) ||
			feedCollidingAnimal(goatIcon->goatList, goatIcon->count, 50, 50, foodPos, "Goat", this) ||
			feedCollidingAnimal(duckIcon->duckList, duckIcon->count, 50, 50, foodPos, "Duck", this);

		if (!animalAte)
		{
			continue;
		}

		waterIcon->grassTileCounts[j]--;
		waterIcon->grassLastDecayTick[j] = currentTick;

		if (waterIcon->grassTileCounts[j] <= 0)
		{
			delete waterIcon->waterList[j];
			waterIcon->waterList[j] = waterIcon->waterList[waterIcon->count - 1];
			waterIcon->grassTileCounts[j] = waterIcon->grassTileCounts[waterIcon->count - 1];
			waterIcon->grassLastDecayTick[j] = waterIcon->grassLastDecayTick[waterIcon->count - 1];
			waterIcon->waterList[waterIcon->count - 1] = nullptr;
			waterIcon->count--;
			j--;
		}
	}
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

	if (gameBudgetbar != nullptr)
	{
		gameBudgetbar->drawPrices();
	}
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
		printMessage("Not enough budget: need $" + to_string(amount));
		return false;
	}

	budget -= amount;
	printBudget("BUDGET = $" + to_string(budget));
	return true;
}

void Game::activateHelper()
{
	if (helperFarmer != nullptr && !helperFarmer->isExpired())
	{
		printMessage("Farmer already working: " + to_string(helperFarmer->getRemainingLifetimeSeconds()) + "s left");
		return;
	}

	if (!spendBudget(helper_cost))
	{
		return;
	}

	delete helperFarmer;
	point startPoint;
	startPoint.x = config.fieldPadding + 30;
	startPoint.y = (2 * config.toolBarHeight) + config.fieldPadding + 30;
	helperFarmer = new Farmer(this, startPoint, 50, 50, "images\\farmer.jpg");
	helperActive = true;
	printMessage("Farmer hired for " + to_string(static_cast<int>(kHelperDurationMs / 1000)) + "s");
}

void Game::clearStatusBar() const
{
	//Clear Status bar by drawing a filled rectangle
	pWind->SetPen(config.statusBarColor, 1);
	pWind->SetBrush(config.statusBarColor);
	pWind->DrawRectangle(0, config.windHeight - config.statusBarHeight, config.windWidth, config.windHeight);
}

void Game::startBackgroundMusic() const
{
	if (!soundMuted)
	{
		if (GetFileAttributes(TEXT("bg.wav")) != INVALID_FILE_ATTRIBUTES)
		{
			PlaySound(TEXT("bg.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		}
		else if (GetFileAttributes(TEXT("..\\bg.wav")) != INVALID_FILE_ATTRIBUTES)
		{
			PlaySound(TEXT("..\\bg.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		}
	}
}

void Game::stopBackgroundMusic() const
{
	PlaySound(NULL, NULL, 0);
}

void Game::toggleSound()
{
	soundMuted = !soundMuted;
	if (soundMuted)
	{
		stopBackgroundMusic();
		printMessage("Sound muted");
	}
	else
	{
		startBackgroundMusic();
		printMessage("Sound on");
	}
	gameToolbar->draw();
}

bool Game::isSoundMuted() const
{
	return soundMuted;
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
	return score;
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
void Game::triggerGameOver() {
	
	PlaySound(TEXT("loser.wav"), NULL, SND_FILENAME | SND_ASYNC);

	pWind->SetPen(RED, 5); // 
	pWind->SetFont(80, BOLD, BY_NAME, "Arial"); 

	
	int xPos = config.windWidth / 2 - 200;
	int yPos = config.windHeight / 2 - 50;

	
	pWind->DrawString(xPos, yPos, "GAME OVER");

	
	pWind->UpdateBuffer();

	
	Sleep(2000);
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
	unsigned long long currentTime = GetTickCount64();

	if (currentTime - lastTime >= 1000 && timer > 0)
	{
		timer--;
		lastTime = currentTime;
	}
}

void Game::updateHelper()
{
	if (helperFarmer == nullptr)
	{
		return;
	}

	if (helperFarmer->isExpired())
	{
		delete helperFarmer;
		helperFarmer = nullptr;
		helperActive = false;
		printMessage("Farmer finished collecting products");
		return;
	}

	point productPoint;
	if (getNearestProductPoint(helperFarmer->getRefPoint(), productPoint))
	{
		helperFarmer->moveToward(productPoint);
		if (collectProductAt(helperFarmer->getRefPoint(), 50, 50))
		{
			drawWarehouse();
		}
		return;
	}

	helperFarmer->moveStep();
}

bool Game::getNearestProductPoint(point fromPoint, point& productPoint) const
{
	bool foundProduct = false;
	int bestDistance = 0;

	for (int i = 0; i < eggCount; i++)
	{
		if (eggList[i] == nullptr)
		{
			continue;
		}

		point candidate = eggList[i]->getRefPoint();
		const int dx = candidate.x - fromPoint.x;
		const int dy = candidate.y - fromPoint.y;
		const int distance = (dx * dx) + (dy * dy);
		if (!foundProduct || distance < bestDistance)
		{
			foundProduct = true;
			bestDistance = distance;
			productPoint = candidate;
		}
	}

	for (int i = 0; i < milkCount; i++)
	{
		if (milkList[i] == nullptr)
		{
			continue;
		}

		point candidate = milkList[i]->getRefPoint();
		const int dx = candidate.x - fromPoint.x;
		const int dy = candidate.y - fromPoint.y;
		const int distance = (dx * dx) + (dy * dy);
		if (!foundProduct || distance < bestDistance)
		{
			foundProduct = true;
			bestDistance = distance;
			productPoint = candidate;
		}
	}

	for (int i = 0; i < woolCount; i++)
	{
		if (woolList[i] == nullptr)
		{
			continue;
		}

		point candidate = woolList[i]->getRefPoint();
		const int dx = candidate.x - fromPoint.x;
		const int dy = candidate.y - fromPoint.y;
		const int distance = (dx * dx) + (dy * dy);
		if (!foundProduct || distance < bestDistance)
		{
			foundProduct = true;
			bestDistance = distance;
			productPoint = candidate;
		}
	}

	return foundProduct;
}

bool Game::collectProductAt(point farmerPoint, int farmerWidth, int farmerHeight)
{
	for (int i = 0; i < eggCount; i++)
	{
		if (eggList[i] != nullptr && rectanglesOverlap(farmerPoint, farmerWidth, farmerHeight, eggList[i]->getRefPoint(), 30, 30))
		{
			warehouseEgg++;
			delete eggList[i];
			eggList[i] = eggList[eggCount - 1];
			eggList[eggCount - 1] = nullptr;
			eggCount--;
			printMessage("Farmer collected an egg");
			return true;
		}
	}

	for (int i = 0; i < milkCount; i++)
	{
		if (milkList[i] != nullptr && rectanglesOverlap(farmerPoint, farmerWidth, farmerHeight, milkList[i]->getRefPoint(), 40, 40))
		{
			warehouseMilk++;
			delete milkList[i];
			milkList[i] = milkList[milkCount - 1];
			milkList[milkCount - 1] = nullptr;
			milkCount--;
			printMessage("Farmer collected milk");
			return true;
		}
	}

	for (int i = 0; i < woolCount; i++)
	{
		if (woolList[i] != nullptr && rectanglesOverlap(farmerPoint, farmerWidth, farmerHeight, woolList[i]->getRefPoint(), 36, 28))
		{
			warehouseWool++;
			delete woolList[i];
			woolList[i] = woolList[woolCount - 1];
			woolList[woolCount - 1] = nullptr;
			woolCount--;
			printMessage("Farmer collected wool");
			return true;
		}
	}

	return false;
}

void Game::checkLevelUp()
{
	if (score >= goal)
	{
		level++;
		goal = 700 + ((level - 1) * 800);
		setTimerByLevel();

		printMessage("LEVEL UP! Welcome to Level " + to_string(level));
		updateStatusBar();
	}
}
void Game::setTimerByLevel()
{
	timer = 90 + ((level - 1) * 20);
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
bool Game::handleWarehouseClick(int x, int y) {
	int left, top, right, bottom;
	getWarehouseInventoryBounds(left, top, right, bottom);
	int warehouseLeft, warehouseTop, warehouseRight, warehouseBottom;
	getWarehouseBounds(warehouseLeft, warehouseTop, warehouseRight, warehouseBottom);
	const int row1Y = top + 34;
	const int rowGap = 28;

	if (x >= warehouseLeft && x <= warehouseRight && y >= warehouseTop && y <= warehouseBottom)
	{
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

		showWarehouseWindow();
		printMessage("Warehouse - Eggs: " + to_string(warehouseEgg) +
			" Milk: " + to_string(warehouseMilk) +
			" Wool: " + to_string(warehouseWool));
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
	score += price;
	printBudget("BUDGET = $" + to_string(budget));
	drawWarehouse();
	printMessage(productName + " sold for $" + to_string(price));
	checkLevelUp();
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
	timer = 90;
	level = 1;
	goal = 700;
	score = 0;
	animals = 0;
	warehouseEgg = 0;
	warehouseMilk = 0;
	warehouseWool = 0;
	helperActive = false;
	delete helperFarmer;
	helperFarmer = nullptr;
	lastTime = GetTickCount64();
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
	const std::string saveFileName = makeSaveGameFileName(username);
	std::ofstream output(saveFileName.c_str());
	if (!output)
	{
		printMessage("Could not save game");
		return;
	}

	ChickIcon* chickIcon = gameBudgetbar->getChickIcon();
	CowIcon* cowIcon = gameBudgetbar->getCowIcon();
	GoatIcon* goatIcon = gameBudgetbar->getGoatIcon();
	SheepIcon* sheepIcon = gameBudgetbar->getSheepIcon();
	DuckIcon* duckIcon = gameBudgetbar->getDuckIcon();
	WaterIcon* waterIcon = gameBudgetbar->getWaterIcon();
	const int savedAnimalCount = chickIcon->count + cowIcon->count + goatIcon->count + sheepIcon->count + duckIcon->count;

	output << "LEVEL " << level << "    // Current level" << std::endl;
	output << "BUDGET " << budget << "    // Current player budget" << std::endl;
	output << "SCORE " << score << "    // Current player score from sold products" << std::endl;
	output << "TIMER " << timer << "    // Remaining time in seconds" << std::endl;
	output << "GOAL " << goal << "    // Level goal" << std::endl;
	output << "PAUSED " << (isPaused ? 1 : 0) << "    // 1 if game is paused, 0 otherwise" << std::endl;
	output << "USERNAME " << username << "    // Current player name" << std::endl;
	output << std::endl;

	output << "ANIMALS " << savedAnimalCount << "    // Number of saved animals" << std::endl;
	output << "// TYPE X Y DX DY COUNTER    Animal type, position, velocity, and movement counter" << std::endl;
	writeAnimalRows(output, "CHICK", chickIcon->chickList, chickIcon->count);
	writeAnimalRows(output, "COW", cowIcon->cowList, cowIcon->count);
	writeAnimalRows(output, "GOAT", goatIcon->goatList, goatIcon->count);
	writeAnimalRows(output, "SHEEP", sheepIcon->sheepList, sheepIcon->count);
	writeAnimalRows(output, "DUCK", duckIcon->duckList, duckIcon->count);
	output << std::endl;

	output << "WOLVES " << wolfCount << "    // Number of alive wolves" << std::endl;
	output << "// WOLF X Y DX DY COUNTER    Wolf label, position, velocity, and movement counter" << std::endl;
	writeAnimalRows(output, "WOLF", wolfList, wolfCount);
	output << std::endl;

	output << "FOODAREAS " << waterIcon->count << "    // Number of active food areas" << std::endl;
	output << "// FOOD X Y COUNT    Food label, position, and remaining food tile count" << std::endl;
	for (int i = 0; i < waterIcon->count; i++)
	{
		if (waterIcon->waterList[i] != nullptr)
		{
			point p = waterIcon->waterList[i]->getRefPoint();
			output << "FOOD " << p.x << " " << p.y << " " << waterIcon->grassTileCounts[i] << std::endl;
			for (int j = 0; j < waterIcon->grassTileCounts[i]; j++)
			{
				output << "GRASS " << waterIcon->grassTiles[i][j].x << " " << waterIcon->grassTiles[i][j].y << std::endl;
			}
		}
	}
	output << std::endl;

	output << "WAREHOUSE    // Start of warehouse data" << std::endl;
	output << "EGGS " << warehouseEgg << "    // Eggs stored in warehouse" << std::endl;
	output << "MILK " << warehouseMilk << "    // Milk bottles stored in warehouse" << std::endl;
	output << "WOOL " << warehouseWool << "    // Wool stored in warehouse" << std::endl;
	output << std::endl;

	output << "PRODUCTS    // Products still on the field" << std::endl;
	writeProductListWithType(output, "EGGS", "EGG", eggList, eggCount);
	writeProductListWithType(output, "MILK", "MILK", milkList, milkCount);
	writeProductListWithType(output, "WOOL", "WOOL", woolList, woolCount);

	if (!output)
	{
		printMessage("Could not finish saving game");
		return;
	}

	printMessage("Game saved to " + saveFileName);
}

void Game::loadGame()
{
	std::string saveFileName = makeSaveGameFileName(username);
	std::ifstream input(saveFileName.c_str());
	if (!input)
	{
		input.clear();
		saveFileName = kLegacySaveGameFileName;
		input.open(saveFileName.c_str());
	}

	if (!input)
	{
		printMessage("No saved game found for " + username);
		return;
	}

	int loadedBudget = 0;
	int loadedPaused = 0;
	int loadedTimer = 0;
	int loadedLevel = 0;
	int loadedGoal = 0;
	int loadedScore = 0;
	int loadedAnimals = 0;
	int loadedWarehouseEgg = 0;
	int loadedWarehouseMilk = 0;
	int loadedWarehouseWool = 0;
	std::string loadedUsername;

	if (!readLabelIntLine(input, "LEVEL", loadedLevel) ||
		!readLabelIntLine(input, "BUDGET", loadedBudget))
	{
		printMessage("Save file is not valid");
		return;
	}

	std::string scoreOrTimerLine;
	if (!readDataLine(input, scoreOrTimerLine))
	{
		printMessage("Save file is not valid");
		return;
	}

	if (parseLabelIntLine(scoreOrTimerLine, "SCORE", loadedScore))
	{
		if (!readLabelIntLine(input, "TIMER", loadedTimer))
		{
			printMessage("Save file is not valid");
			return;
		}
	}
	else if (!parseLabelIntLine(scoreOrTimerLine, "TIMER", loadedTimer))
	{
		printMessage("Save file is not valid");
		return;
	}

	if (!readLabelIntLine(input, "GOAL", loadedGoal) ||
		!readLabelIntLine(input, "PAUSED", loadedPaused))
	{
		printMessage("Save file is not valid");
		return;
	}

	std::string line;
	if (!readDataLine(input, line))
	{
		printMessage("Save file is not valid");
		return;
	}

	std::stringstream usernameParser(line);
	std::string usernameLabel;
	usernameParser >> usernameLabel;
	if (!usernameParser || usernameLabel != "USERNAME")
	{
		printMessage("Save file is not valid");
		return;
	}

	std::getline(usernameParser, loadedUsername);
	loadedUsername = trimCopy(loadedUsername);

	if (loadedUsername != username)
	{
		printMessage("This save belongs to " + loadedUsername);
		return;
	}

	clearProducts();
	clearWolves();
	gameBudgetbar->resetAnimals();

	budget = loadedBudget;
	isPaused = (loadedPaused != 0);
	timer = loadedTimer;
	level = loadedLevel;
	goal = loadedGoal;
	score = loadedScore;
	helperActive = false;
	delete helperFarmer;
	helperFarmer = nullptr;
	lastTime = GetTickCount64();


	if (!readLabelIntLine(input, "ANIMALS", loadedAnimals) || loadedAnimals < 0)
	{
		printMessage("Save file is not valid");
		return;
	}

	ChickIcon* chickIcon = gameBudgetbar->getChickIcon();
	CowIcon* cowIcon = gameBudgetbar->getCowIcon();
	GoatIcon* goatIcon = gameBudgetbar->getGoatIcon();
	SheepIcon* sheepIcon = gameBudgetbar->getSheepIcon();
	DuckIcon* duckIcon = gameBudgetbar->getDuckIcon();
	WaterIcon* waterIcon = gameBudgetbar->getWaterIcon();

	for (int i = 0; i < loadedAnimals; i++)
	{
		if (!readDataLine(input, line))
		{
			printMessage("Save file is not valid");
			return;
		}

		std::stringstream typeParser(line);
		std::string animalType;
		typeParser >> animalType;
		bool loaded = false;
		if (animalType == "CHICK")
		{
			loaded = loadAnimalRow(line, "CHICK", chickIcon->chickList, chickIcon->count, max_budget_items, this, chickIcon->image_path);
		}
		else if (animalType == "COW")
		{
			loaded = loadAnimalRow(line, "COW", cowIcon->cowList, cowIcon->count, max_budget_items, this, cowIcon->image_path);
		}
		else if (animalType == "GOAT")
		{
			loaded = loadAnimalRow(line, "GOAT", goatIcon->goatList, goatIcon->count, max_budget_items, this, goatIcon->image_path);
		}
		else if (animalType == "SHEEP")
		{
			loaded = loadAnimalRow(line, "SHEEP", sheepIcon->sheepList, sheepIcon->count, max_budget_items, this, sheepIcon->image_path);
		}
		else if (animalType == "DUCK")
		{
			loaded = loadAnimalRow(line, "DUCK", duckIcon->duckList, duckIcon->count, max_budget_items, this, duckIcon->image_path);
		}

		if (!loaded)
		{
			printMessage("Save file is not valid");
			return;
		}
	}

	if (!readLabelIntLine(input, "WOLVES", wolfCount) || wolfCount < 0 || wolfCount > kMaxProducts)
	{
		printMessage("Save file is not valid");
		return;
	}

	for (int i = 0; i < wolfCount; i++)
	{
		if (!readDataLine(input, line))
		{
			printMessage("Save file is not valid");
			return;
		}

		int loadedWolfCount = i;
		if (!loadAnimalRow(line, "WOLF", wolfList, loadedWolfCount, kMaxProducts, this, "images\\wolf.jpg"))
		{
			printMessage("Save file is not valid");
			return;
		}
	}

	if (!readLabelIntLine(input, "FOODAREAS", waterIcon->count) || waterIcon->count < 0 || waterIcon->count > max_budget_items)
	{
		printMessage("Save file is not valid");
		return;
	}

	for (int i = 0; i < waterIcon->count; i++)
	{
		point p;
		std::string foodLabel;
		if (!readDataLine(input, line))
		{
			printMessage("Save file is not valid");
			return;
		}

		std::stringstream foodParser(line);
		foodParser >> foodLabel >> p.x >> p.y >> waterIcon->grassTileCounts[i];
		if (!foodParser || foodLabel != "FOOD" || waterIcon->grassTileCounts[i] < 0 || waterIcon->grassTileCounts[i] > grass_tiles_per_water)
		{
			printMessage("Save file is not valid");
			return;
		}

		waterIcon->waterList[i] = new Water(this, p, 50, 50, waterIcon->image_path);
		waterIcon->grassLastDecayTick[i] = GetTickCount64();

		for (int j = 0; j < waterIcon->grassTileCounts[i]; j++)
		{
			std::string grassLabel;
			if (!readDataLine(input, line))
			{
				printMessage("Save file is not valid");
				return;
			}

			std::stringstream grassParser(line);
			grassParser >> grassLabel >> waterIcon->grassTiles[i][j].x >> waterIcon->grassTiles[i][j].y;
			if (!grassParser || grassLabel != "GRASS")
			{
				printMessage("Save file is not valid");
				return;
			}
		}
	}

	if (!readSectionLine(input, "WAREHOUSE") ||
		!readLabelIntLine(input, "EGGS", loadedWarehouseEgg) ||
		!readLabelIntLine(input, "MILK", loadedWarehouseMilk) ||
		!readLabelIntLine(input, "WOOL", loadedWarehouseWool) ||
		!readSectionLine(input, "PRODUCTS") ||
		!readTypedProductList(input, "EGGS", "EGG", eggList, eggCount, kMaxProducts, this) ||
		!readTypedProductList(input, "MILK", "MILK", milkList, milkCount, kMaxProducts, this) ||
		!readTypedProductList(input, "WOOL", "WOOL", woolList, woolCount, kMaxProducts, this))
	{
		printMessage("Save file is not valid");
		return;
	}

	warehouseEgg = loadedWarehouseEgg;
	warehouseMilk = loadedWarehouseMilk;
	warehouseWool = loadedWarehouseWool;
	animals = gameBudgetbar->getAnimalCount();

	pWind->SetPen(config.bkGrndColor, 1);
	pWind->SetBrush(config.bkGrndColor);
	pWind->DrawRectangle(0, 0, config.windWidth, config.windHeight);
	gameToolbar->draw();
	gameBudgetbar->draw();
	drawFoodArea();
	DrawProducts();
	gameBudgetbar->updateAnimals();
	for (int i = 0; i < wolfCount; i++)
	{
		if (wolfList[i] != nullptr)
		{
			wolfList[i]->draw();
		}
	}
	updateStatusBar();
	printBudget("BUDGET = $" + to_string(budget));
	printMessage("Game loaded from " + saveFileName);
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


//for the warehouse 
void Game::showWarehouseWindow()
{

	window* pWarehouseWind = new window(400, 300, 500, 200);
	pWarehouseWind->ChangeTitle("Warehouse Inventory");


	pWarehouseWind->SetBrush(WHITE);
	pWarehouseWind->DrawRectangle(0, 0, 400, 300);
	pWarehouseWind->SetFont(20, BOLD, BY_NAME, "Arial");
	pWarehouseWind->SetPen(BLACK);
	pWarehouseWind->DrawString(50, 40, "Warehouse Contents:");
	pWarehouseWind->DrawString(70, 100, "Eggs: " + std::to_string(warehouseEgg));
	pWarehouseWind->DrawString(70, 140, "Milk: " + std::to_string(warehouseMilk));
	pWarehouseWind->DrawString(70, 180, "Wool: " + std::to_string(warehouseWool));
	pWarehouseWind->SetPen(RED);
	pWarehouseWind->DrawString(40, 240, "Click anywhere on the main map to close");


	int x, y;
	while (pWind->GetButtonState(LEFT_BUTTON, x, y)) {
		Sleep(10);
	}


	bool clickedOutside = false;
	//what keeps the new window open
	while (!clickedOutside)
	{

		if (pWind->GetButtonState(LEFT_BUTTON, x, y))
		{
			clickedOutside = true;
		}


		if (!pWind->IsOpen()) break;

		Sleep(20);
	}

	delete pWarehouseWind;
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

bool Game::getNearestWolfPoint(point fromPoint, point& wolfPoint) const
{
	bool foundWolf = false;
	int bestDistance = 0;

	for (int i = 0; i < wolfCount; i++)
	{
		if (wolfList[i] == nullptr)
		{
			continue;
		}

		point candidate = wolfList[i]->getRefPoint();
		const int dx = candidate.x - fromPoint.x;
		const int dy = candidate.y - fromPoint.y;
		const int distance = (dx * dx) + (dy * dy);

		if (!foundWolf || distance < bestDistance)
		{
			foundWolf = true;
			bestDistance = distance;
			wolfPoint = candidate;
		}
	}

	return foundWolf;
}

bool Game::removeWolfAt(point dogPoint, int dogWidth, int dogHeight)
{
	for (int i = 0; i < wolfCount; i++)
	{
		if (wolfList[i] != nullptr && rectanglesOverlap(dogPoint, dogWidth, dogHeight, wolfList[i]->getRefPoint(), 50, 50))
		{
			delete wolfList[i];
			wolfList[i] = wolfList[wolfCount - 1];
			wolfList[wolfCount - 1] = nullptr;
			wolfCount--;
			printMessage("Dog chased the wolf away!");
			return true;
		}
	}

	return false;
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
				// --

				// 
				PlaySound(TEXT("loser.wav"), NULL, SND_FILENAME | SND_ASYNC);

				pWind->SetPen(RED, 8);
				pWind->SetFont(80, BOLD, BY_NAME, "Arial");

				
				pWind->DrawString(config.windWidth / 2 - 200, config.windHeight / 2 - 50, "GAME OVER");

				// 4. 
				pWind->UpdateBuffer();

				Sleep(2000);

				

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
			handleFeedingLogic();
			drawFoodArea();
			DrawProducts();
			updateHelper();
			showRandomWolf();
			for (int i = 0; i < wolfCount; i++) {
				if (wolfList[i] != nullptr) {
					wolfList[i]->moveStep();
					DuckIcon* duckIcon = gameBudgetbar->getDuckIcon();
					for (int j = 0; j < duckIcon->count; j++) {
						if (duckIcon->duckList[j] == nullptr) {
							continue;
						}
						if (rectanglesOverlap(wolfList[i]->getRefPoint(), 50, 50, duckIcon->duckList[j]->getRefPoint(), 50, 50))
						{
							delete duckIcon->duckList[j];
							duckIcon->duckList[j] = nullptr;
							animals--;
							printMessage("A wolf ate your duck !");
						}
					}
					ChickIcon* chikIcon = gameBudgetbar->getChickIcon();
					for (int j = 0; j < chikIcon->count; j++) {
						if (chikIcon->chickList[j] == nullptr) {
							continue;
						}
						if (rectanglesOverlap(wolfList[i]->getRefPoint(), 50, 50, chikIcon->chickList[j]->getRefPoint(), 50, 50))
						{
							delete chikIcon->chickList[j];
							chikIcon->chickList[j] = nullptr;
							animals--;
							printMessage("A wolf ate your chicken !");

						}
					}

					SheepIcon* sheepIcon = gameBudgetbar->getSheepIcon();
					for (int j = 0; j < sheepIcon->count; j++) {
						if (sheepIcon->sheepList[j] == nullptr) {
							continue;
						}
						if (rectanglesOverlap(wolfList[i]->getRefPoint(), 50, 50, sheepIcon->sheepList[j]->getRefPoint(), 50, 50))
						{
							delete sheepIcon->sheepList[j];
							sheepIcon->sheepList[j] = nullptr;
							animals--;
							printMessage("A wolf ate your sheep !");
						}
					}
					CowIcon* cowIcon = gameBudgetbar->getCowIcon();
					for (int j = 0; j < cowIcon->count; j++) {
						if (cowIcon->cowList[j] == nullptr) {
							continue;
						}
						if (rectanglesOverlap(wolfList[i]->getRefPoint(), 50, 50, cowIcon->cowList[j]->getRefPoint(), 50, 50))
						{
							delete cowIcon->cowList[j];
							cowIcon->cowList[j] = nullptr;
							animals--;
							printMessage("A wolf ate your cow !");
						}
					}
					GoatIcon* goatIcon = gameBudgetbar->getGoatIcon();
					for (int j = 0; j < goatIcon->count; j++) {
						if (goatIcon->goatList[j] == nullptr) {
							continue;
						}
						if (rectanglesOverlap(wolfList[i]->getRefPoint(), 50, 50, goatIcon->goatList[j]->getRefPoint(), 50, 50))
						{
							delete goatIcon->goatList[j];
							goatIcon->goatList[j] = nullptr;
							animals--;
							printMessage("A wolf ate your goat !");
						}
					}
				}
			}
			
			gameBudgetbar->updateAnimals();
			animals = gameBudgetbar->getAnimalCount();
			updateStatusBar();
		}


		pWind->UpdateBuffer();

		Sleep(16);

		if (pWind->GetMouseClick(x, y) == LEFT_CLICK)
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

			pWind->UpdateBuffer();

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
	int showChance = 24 + ((level - 1) * 4);
	if (showChance > 55)
	{
		showChance = 55;
	}

	if (wolfCount < kMaxProducts && (rand() % 10000) < showChance) {
		const int maxWolfX = config.windWidth - config.warehouseWidth - 110;
		const int minWolfY = (2 * config.toolBarHeight) + config.fieldPadding;
		const int maxWolfY = config.windHeight - config.statusBarHeight - config.fieldPadding - 50;
		int x = config.fieldPadding + (rand() % (maxWolfX - config.fieldPadding));
		int y = minWolfY + (rand() % (maxWolfY - minWolfY));
		point p = { x, y };
		wolfList[wolfCount] = new Wolf(this, p, 50, 50, "images\\wolf.JPEG");
		wolfCount++;
	}
}
