#include "Game.h"
#include "../Config/GameConfig.h"

Game::Game()
{
	wolfCount = 0;
	for (int i = 0; i < kMaxProducts; i++) {
		wolfList[i] = nullptr;
	}
	eggCount = 0;
	milkCount = 0;
	woolCount = 0;
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
	lastTime = GetTickCount();

	//1 - Create the main window
	pWind = CreateWind(config.windWidth, config.windHeight, config.wx, config.wy);
	pWind->SetBuffering(true);

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

	delete gameToolbar;
	delete gameBudgetbar;
	delete pWind;
}

clicktype Game::getMouseClick(int& x, int& y) const
{
	return pWind->WaitMouseClick(x, y);	//Wait for mouse click

}

string Game::getSrting() const
{
	string Label;
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
	const int warehouseLeft = config.windWidth - config.warehouseWidth - 35;
	const int warehouseRight = warehouseLeft + config.warehouseWidth;
	const int warehouseBottom = fieldBottom - 35;
	const int warehouseTop = warehouseBottom - config.warehouseHeight;

	pWind->SetPen(BLACK, 3);
	pWind->SetBrush(color(196, 129, 79));
	pWind->DrawRectangle(warehouseLeft, warehouseTop, warehouseRight, warehouseBottom);

	int roofX[3] = { warehouseLeft - 10, warehouseLeft + (config.warehouseWidth / 2), warehouseRight + 10 };
	int roofY[3] = { warehouseTop, warehouseTop - 70, warehouseTop };
	pWind->SetBrush(color(145, 53, 40));
	pWind->DrawPolygon(roofX, roofY, 3);

	pWind->SetBrush(color(104, 67, 40));
	pWind->DrawRectangle(warehouseLeft + 60, warehouseBottom - 75, warehouseLeft + 130, warehouseBottom);

	pWind->SetBrush(color(188, 225, 255));
	pWind->DrawRectangle(warehouseLeft + 20, warehouseTop + 35, warehouseLeft + 55, warehouseTop + 70);
	pWind->DrawRectangle(warehouseRight - 55, warehouseTop + 35, warehouseRight - 20, warehouseTop + 70);

	pWind->SetPen(BLACK, 2);
	pWind->SetFont(20, BOLD, BY_NAME, "Arial");
	pWind->DrawString(warehouseLeft + 35, warehouseTop + 100, "Warehouse");
}

void Game::drawFoodArea() const
{
	drawFieldBackground();
	drawWarehouse();

	const int fieldTop = (2 * config.toolBarHeight) + config.fieldPadding;
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

void Game::printBudget(string msg) const
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

void Game::printMessage(string msg) const
{
	clearStatusBar();	//First clear the status bar

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(10, config.windHeight - (int)(0.85 * config.statusBarHeight), msg);

}

void Game::updateStatusBar() const
{
	clearStatusBar();

	string msg = "Timer: " + to_string(timer) +
		" | Goal: " + to_string(goal) +
		" | Level: " + to_string(level) +
		" | Animals: " + to_string(animals);

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(10, config.windHeight - (int)(0.85 * config.statusBarHeight), msg);
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

void Game::updateTimer()
{
	unsigned long currentTime = GetTickCount();

	if (currentTime - lastTime >= 1000 && timer > 0)
	{
		timer--;
		lastTime = currentTime;
	}
}

void Game::restartGame()
{
	clearProducts();
	budget = kStartingBudget;
	isPaused = false;
	timer = 60;
	level = 1;
	goal = 5;
	animals = 0;
	lastTime = GetTickCount();
	gameBudgetbar->resetAnimals();
	pWind->SetPen(config.bkGrndColor, 1);
	pWind->SetBrush(config.bkGrndColor);
	pWind->DrawRectangle(0, 0, config.windWidth, config.windHeight);
	gameToolbar->draw();
	gameBudgetbar->draw();
	drawFieldBackground();
	drawWarehouse();
	clearStatusBar();
	printBudget("BUDGET = $" + to_string(budget));
	printMessage("Game restarted");
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

void Game::go()
{
	int x = 0, y = 0;
	bool isExit = false;

	pWind->ChangeTitle("- - - - - - - - - - Farm Frenzy (CIE101-project) - - - - - - - - - -");

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
				printMessage("Game Over!");
				pWind->UpdateBuffer();
				isExit = true;
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

			while (pWind->GetButtonState(LEFT_BUTTON, x, y))
			{
				Sleep(10);
			}
		}
	} while (!isExit && pWind->IsOpen());
}
void Game::showRandomWolf() {
	int showChance = 30 + (level * 5);

	if (wolfCount < kMaxProducts && (rand() % 10000) < showChance) {
		int x = 100 + (rand() % 600);
		int y = 200 + (rand() % 300);
		point p = { x, y };
		wolfList[wolfCount] = new Wolf(this, p, 50, 50, "images\\wolf.JPEG");
		wolfCount++;
	}
}

