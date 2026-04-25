#pragma once
#include <string>
#include "Core/Drawable.h"

class Product : public Drawable
{
protected:
	std::string image_path;
	image sprite;

public:
	Product(Game* r_pGame, point r_point, int r_width, int r_height, std::string img_path);
	virtual void draw() const override;
	bool isClicked(int x, int y) const;
};

class Egg : public Product
{
public:
	Egg(Game* r_pGame, point r_point);
};

class Milk : public Product
{
public:
	Milk(Game* r_pGame, point r_point);
};

class Wool : public Product
{
public:
	Wool(Game* r_pGame, point r_point);
	virtual void draw() const override;
};
