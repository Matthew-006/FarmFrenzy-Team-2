#include "Product.h"
#include "Core/Game.h"

Product::Product(Game* r_pGame, point r_point, int r_width, int r_height, std::string img_path)
	: Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
}

void Product::draw() const
{
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
}

Egg::Egg(Game* r_pGame, point r_point)
	: Product(r_pGame, r_point, 30, 30, "images\\egg.jpg")
{
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
}
