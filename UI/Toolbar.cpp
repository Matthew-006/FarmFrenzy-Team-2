#include "Toolbar.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"

ToolbarIcon::ToolbarIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path, string textLabel) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
	label = textLabel;
}

void ToolbarIcon::draw() const
{
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 2);
	pWind->SetBrush(color(240, 236, 214));
	pWind->DrawRectangle(RefPoint.x + 2, RefPoint.y + 2, RefPoint.x + width - 2, RefPoint.y + height - 2);

	pWind->SetFont(11, BOLD, BY_NAME, "Arial");
	pWind->DrawString(RefPoint.x + 8, RefPoint.y + height - 16, label);

	const int centerX = RefPoint.x + width / 2;
	const int centerY = RefPoint.y + 17;

	if (label == "Restart")
	{
		pWind->DrawCircle(centerX, centerY, 10, FRAME);
		pWind->DrawLine(centerX - 3, centerY - 12, centerX - 11, centerY - 4);
		pWind->DrawLine(centerX - 3, centerY - 12, centerX + 4, centerY - 10);
	}
	else if (label == "Pause")
	{
		pWind->SetBrush(DARKBLUE);
		pWind->DrawRectangle(centerX - 9, centerY - 10, centerX - 3, centerY + 10);
		pWind->DrawRectangle(centerX + 3, centerY - 10, centerX + 9, centerY + 10);
	}
	else if (label == "Resume")
	{
		int xPoints[3] = { centerX - 7, centerX - 7, centerX + 9 };
		int yPoints[3] = { centerY - 11, centerY + 11, centerY };
		pWind->SetBrush(DARKGREEN);
		pWind->DrawPolygon(xPoints, yPoints, 3);
	}
	else if (label == "Save")
	{
		pWind->SetBrush(color(90, 149, 214));
		pWind->DrawRectangle(centerX - 11, centerY - 11, centerX + 11, centerY + 11);
		pWind->SetBrush(WHITE);
		pWind->DrawRectangle(centerX - 7, centerY - 8, centerX + 7, centerY - 1);
		pWind->SetBrush(BLACK);
		pWind->DrawRectangle(centerX - 5, centerY + 3, centerX + 5, centerY + 10);
	}
	else if (label == "Load")
	{
		pWind->SetBrush(color(232, 197, 93));
		pWind->DrawRectangle(centerX - 12, centerY - 3, centerX + 12, centerY + 10);
		pWind->DrawLine(centerX, centerY - 12, centerX, centerY + 2);
		pWind->DrawLine(centerX, centerY + 2, centerX - 6, centerY - 4);
		pWind->DrawLine(centerX, centerY + 2, centerX + 6, centerY - 4);
	}
	else if (label == "Sound")
	{
		pWind->SetPen(BLACK, 2);
		pWind->SetBrush(color(90, 149, 214));
		int speakerX[4] = { centerX - 13, centerX - 6, centerX - 6, centerX - 13 };
		int speakerY[4] = { centerY - 5, centerY - 10, centerY + 10, centerY + 5 };
		pWind->DrawPolygon(speakerX, speakerY, 4);
		pWind->DrawRectangle(centerX - 17, centerY - 5, centerX - 13, centerY + 5);
		pWind->DrawArc(centerX - 7, centerY - 9, centerX + 11, centerY + 9, -45, 45);
		pWind->DrawArc(centerX - 5, centerY - 14, centerX + 18, centerY + 14, -45, 45);
		if (pGame->isSoundMuted())
		{
			pWind->SetPen(RED, 3);
			pWind->DrawLine(centerX - 17, centerY - 13, centerX + 17, centerY + 13);
		}
	}
	else if (label == "Exit")
	{
		pWind->SetPen(RED, 3);
		pWind->DrawLine(centerX - 9, centerY - 9, centerX + 9, centerY + 9);
		pWind->DrawLine(centerX + 9, centerY - 9, centerX - 9, centerY + 9);
	}
}

RestartIcon::RestartIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Restart")
{}

void RestartIcon::onClick()
{
	pGame->restartGame();
}

ExitIcon::ExitIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Exit")
{}

void ExitIcon::onClick()
{
	pGame->printMessage("Exiting game...");
}

PauseIcon::PauseIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Pause")
{}

void PauseIcon::onClick()
{
	pGame->pauseGame();
}

ResumeIcon::ResumeIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Resume")
{}

void ResumeIcon::onClick()
{
	pGame->resumeGame();
}

SaveIcon::SaveIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Save")
{}

void SaveIcon::onClick()
{
	pGame->saveGame();
}

LoadGameIcon::LoadGameIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Load")
{}

void LoadGameIcon::onClick()
{
	pGame->loadGame();
}

SoundIcon::SoundIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path, "Sound")
{}

void SoundIcon::onClick()
{
	pGame->toggleSound();
}

Toolbar::Toolbar(Game* r_pGame, point r_point, int r_width, int r_height) : Drawable(r_pGame, r_point, r_width, r_height)
{
	//First prepare List of images for each icon
	//To control the order of these images in the menu, reoder them in enum ICONS above	
	iconsImages[ICON_RESTART] = "images\\RESTART.JPEG";
	iconsImages[ICON_PAUSE] = "images\\PAUSE.JPEG";
	iconsImages[ICON_RESUME] = "images\\RESUME.JPEG";
	iconsImages[ICON_SAVE] = "images\\SAVE.JPEG";
	iconsImages[ICON_LOAD] = "images\\LOAD.JPEG";
	iconsImages[ICON_SOUND] = "";
	iconsImages[ICON_EXIT] = "images\\EXIT.JPEG";
	point p;
	p.x = 0;
	p.y = 0;

	iconsList = new ToolbarIcon * [ICON_COUNT];

	//For each icon in the tool bar create an object 
	iconsList[ICON_RESTART] = new RestartIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_RESTART]);
	p.x += config.iconWidth;
	iconsList[ICON_PAUSE] = new PauseIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_PAUSE]);
	p.x += config.iconWidth;
	iconsList[ICON_RESUME] = new ResumeIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_RESUME]);
	p.x += config.iconWidth;
	iconsList[ICON_SAVE] = new SaveIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_SAVE]);
	p.x += config.iconWidth;
	iconsList[ICON_LOAD] = new LoadGameIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_LOAD]);
	p.x += config.iconWidth;
	iconsList[ICON_SOUND] = new SoundIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_SOUND]);
	p.x += config.iconWidth;
	iconsList[ICON_EXIT] = new ExitIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_EXIT]);
	//p.x += config.iconWidth;
}

Toolbar::~Toolbar()
{
	for (int i = 0; i < ICON_COUNT; i++)
		delete iconsList[i];
	delete[] iconsList;
}

void Toolbar::draw() const
{
	for (int i = 0; i < ICON_COUNT; i++)
		iconsList[i]->draw();
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 3);
	pWind->DrawLine(0, config.toolBarHeight, pWind->GetWidth(), config.toolBarHeight);
}

bool Toolbar::handleClick(int x, int y)
{
	if (x >= ICON_COUNT * config.iconWidth)	//click outside toolbar boundaries
		return false;


	//Check whick icon was clicked
	//==> This assumes that menu icons are lined up horizontally <==
	//Divide x co-ord of the point clicked by the icon width (int division)
	//if division result is 0 ==> first icon is clicked, if 1 ==> 2nd icon and so on

	int clickedIconIndex = (x / config.iconWidth);
	iconsList[clickedIconIndex]->onClick();	//execute onClick action of clicled icon

	if (clickedIconIndex == ICON_EXIT) return true;

	return false;

}

