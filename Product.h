#pragma once
#include <string>
#include "Core/Drawable.h"

class Product : public Drawable
{
protected:
	std::string image_path;
	image sprite;
	bool imageLoaded;
	unsigned long long spawnTick;
	static const unsigned long long expiryMs = 30000; 
public:
	bool isExpired() const;
	int getRemainingSeconds() const;
	Product(Game* r_pGame, point r_point, int r_width, int r_height, std::string img_path);
	virtual void draw() const override;
	bool isClicked(int x, int y) const;
};

class Egg : public Product
{
private:
	bool isGoldenEgg;
public:
	Egg(Game* r_pGame, point r_point, bool is_gold = false);
	bool isGolden() const { return isGoldenEgg; }
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
