#pragma once
#include "../Core/Drawable.h"

class Animal :public Drawable
{
protected:
	enum ProductType
	{
		PRODUCT_NONE,
		PRODUCT_EGG,
		PRODUCT_MILK,
		PRODUCT_WOOL
	};

private:
	string image_path;
	image sprite;
	int dx, dy;
	int changeCounter;
	unsigned long lastProductTick;
	int productIntervalMs;
	ProductType productType;
public:
	point curr_pos;
	point curr_vel;
	Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void draw() const override;
	virtual void moveStep() = 0;   //The action that should be taken each time step

	void setDx(int v) { dx = v; }
	void setDy(int v) { dy = v; }
	int getDx() const { return dx; }
	int getDy() const { return dy; }
	void setChangeCounter(int v) { changeCounter = v; }
	int getChangeCounter() const { return changeCounter; }
	void setProductIntervalMs(int v) { productIntervalMs = v; }
	int getProductIntervalMs() const { return productIntervalMs; }
	void setProductType(ProductType value) { productType = value; }
	bool isProductReady();
	int getRemainingProductSeconds() const;
	void drawCounter() const;
	void produceProduct();
};

class Chick : public Animal
{
public:
	Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};

class Cow : public Animal
{
public:
	Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};

class Wolf : public Animal
{
public:
	Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};

class Goat : public Animal
{
public:
	Goat(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};


class Sheep : public Animal
{
public:
	Sheep(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};


class Water : public Animal
{
public:
	Water(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};

