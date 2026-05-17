#include "Product.h"
#include "Core/Game.h"
#include <fstream>

namespace
{
	std::string resolveAssetPath(const std::string& path)
	{
		std::ifstream localFile(path.c_str());
		if (localFile)
		{
			return path;
		}

		const std::string parentPath = "..\\" + path;
		std::ifstream parentFile(parentPath.c_str());
		if (parentFile)
		{
			return parentPath;
		}

		return path;
	}
}

Product::Product(Game* r_pGame, point r_point, int r_width, int r_height, std::string img_path)
	: Drawable(r_pGame, r_point, r_width, r_height)
{
	spawnTick = GetTickCount64();
	image_path = img_path;
	imageLoaded = false;
	if (!image_path.empty())
	{
		image_path = resolveAssetPath(image_path);
		try
		{
			sprite.Open(image_path);
			imageLoaded = true;
		}
		catch (...)
		{
			imageLoaded = false;
		}
	}
}

void Product::draw() const
{
	window* pWind = pGame->getWind();
	if (imageLoaded)
	{
		pWind->DrawImage(sprite, RefPoint.x, RefPoint.y, width, height);
	}
	else
	{
		pWind->SetPen(BLACK, 1);
		pWind->SetBrush(WHITE);
		pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height, FILLED, 5, 5);
	}
	pWind->SetPen(RED, 1);
	pWind->SetFont(12, BOLD, BY_NAME, "Arial");
	pWind->DrawString(RefPoint.x, RefPoint.y - 12, to_string(getRemainingSeconds()) + "s");
}

bool Product::isClicked(int x, int y) const
{
	return x >= RefPoint.x && x <= RefPoint.x + width &&
		y >= RefPoint.y && y <= RefPoint.y + height;
}

Egg::Egg(Game* r_pGame, point r_point, bool is_gold)
	: Product(r_pGame, r_point, 30, 30, is_gold ? "images\\golden_egg.jpg" : "images\\egg.jpg")
{
	isGoldenEgg = is_gold;
}

Milk::Milk(Game* r_pGame, point r_point)
	: Product(r_pGame, r_point, 40, 40, "images\\milk.jpg")
{
}

Wool::Wool(Game* r_pGame, point r_point)
	: Product(r_pGame, r_point, 36, 28, "")
{
}

void Wool::draw() const
{
	window* pWind = pGame->getWind();

	pWind->SetPen(color(180, 180, 180), 1);
	pWind->SetBrush(WHITE);
	pWind->DrawCircle(RefPoint.x + 10, RefPoint.y + 12, 8);
	pWind->DrawCircle(RefPoint.x + 18, RefPoint.y + 9, 9);
	pWind->DrawCircle(RefPoint.x + 26, RefPoint.y + 12, 8);
	pWind->DrawCircle(RefPoint.x + 15, RefPoint.y + 18, 8);
	pWind->DrawCircle(RefPoint.x + 23, RefPoint.y + 18, 8);

	pWind->SetPen(RED, 1);
	pWind->SetFont(12, BOLD, BY_NAME, "Arial");
	pWind->DrawString(RefPoint.x, RefPoint.y - 12, to_string(getRemainingSeconds()) + "s");
}
bool Product::isExpired() const
{
	return GetTickCount64() - spawnTick >= expiryMs;
}

int Product::getRemainingSeconds() const
{
	unsigned long long elapsedMs = GetTickCount64() - spawnTick;
	if (elapsedMs >= expiryMs)
		return 0;
	return static_cast<int>((expiryMs - elapsedMs + 999) / 1000);
}
