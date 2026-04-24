#pragma once
#include <string>
#include "Core/Drawable.h" 

class Game;

class Product : public Drawable
{
protected:
	std::string image_path;
public:
	Product(Game* r_pGame, point r_point, int r_width, int r_height, std::string img_path);
	virtual void draw() const override;
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