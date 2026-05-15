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
	void moveInsideField(int maxSpeed, int changeInterval);
	bool shouldSkipMovementFrame();

private:
	string image_path;
	image sprite;
	int dx, dy;
	int changeCounter;
	int movementCounter;
	unsigned long lastProductTick;
	int productIntervalMs;
	ProductType productType;
	int foodEatenCounter;
public:
	void increaseFoodCounter() {
		foodEatenCounter++;
	}

	int getFoodCounter() const {
		return foodEatenCounter;
	}
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
private:
	int disapclicks;
public:
	Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
	bool clicked() {
		disapclicks--;
		return (disapclicks <= 0);
	}
};

class Dog : public Animal
{
private:
	unsigned long birthTick;
	static const unsigned long lifetimeMs = 10000;
public:
	Dog(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
	void moveToward(point target);
	bool isExpired() const;
	int getRemainingLifetimeSeconds() const;
	void drawLifetimeCounter() const;
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
class Duck : public Animal
{
public:
	Duck(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
	

	

};
