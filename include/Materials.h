#pragma once

/*
	Class that represents materials for the simulation.
	Wrapper class.
*/

// STL
#include <random>

// Project headers
#include "Game.h"
#include "MaterialEnums.h"


// Constants
inline const sf::Color DEFAULT_COLOR = sf::Color(3, 9, 28);

//////////////////////////    Material class     /////////////////////////

class Material
{
public:
	// Constructor / Destructor
	Material(MaterialType type, sf::Color color, float density = 1000.0f);
	virtual ~Material() = default;

	// Accessors
	MaterialType getType() const;
	virtual MaterialState getState() const;
	virtual sf::Color getColor() const;
	float getDensity() const;
	bool isUpdated() const;
	void setUpdated(bool value);

	// Methods
	virtual void update(int x, int y, Game& game) = 0;

protected:
	// Protected variables
	MaterialType type;
	sf::Color color;
	float density;         // kg / m^3
	bool updated = false;

	// Protected methods
	virtual sf::Color generateColor() const {
		return sf::Color::Transparent;
	}

	virtual sf::Color generateColor(int x, int y) const {
		return generateColor();
	}
};

//========================================================================


//////////////////////////  SolidMaterial class   /////////////////////////

class SolidMaterial
	: public Material {
public:
	SolidMaterial(MaterialType type, sf::Color color, float density);

	virtual void update(int x, int y, Game& game) = 0;
};

//========================================================================


////////////////////  SolidUnmovableMaterial class  //////////////////////

class SolidUnmovableMaterial
	: public SolidMaterial {
public:
	SolidUnmovableMaterial(MaterialType type, sf::Color color, float density = 2200.0f);

	void update(int x, int y, Game& game) override;
};

//========================================================================


/////////////////////  SolidMovableMaterial class  ///////////////////////

class SolidMovableMaterial
	: public SolidMaterial
{
public:
	SolidMovableMaterial(MaterialType type, sf::Color color, float density = 1300.0f);

	void update(int x, int y, Game& game) override;
};

//========================================================================


/////////////////////////  LiquidMaterial class  /////////////////////////

class LiquidMaterial
	: public Material {
public:
	LiquidMaterial(MaterialType type, sf::Color color, float density = 1000.0f);

	void update(int x, int y, Game& game) override;
protected:
	virtual sf::Color generateColor() const override = 0;
};

//========================================================================


//////////////////////////  GaseousMaterial class  ///////////////////////////

class GaseousMaterial
	: public Material {
public:
	GaseousMaterial(MaterialType type, sf::Color color, float density = 1.26f);

	void update(int x, int y, Game& game) override;
protected:
	virtual sf::Color generateColor() const override = 0;
};

//========================================================================


//////////////////////////  EmptyMaterial class   /////////////////////////

class EmptyMaterial :
	public SolidUnmovableMaterial
{
public:
	EmptyMaterial();

};

//========================================================================


//////////////////////////  SandMaterial class  //////////////////////////

class SandMaterial :
	public SolidMovableMaterial
{
public:
	SandMaterial();

protected:
	sf::Color generateColor() const override;
};

//========================================================================


//////////////////////////  StoneMaterial class  /////////////////////////

class StoneMaterial :
	public SolidUnmovableMaterial
{
public:
	StoneMaterial();

protected:
	sf::Color generateColor() const override;
};

//========================================================================


//////////////////////////  DirtMaterial class  //////////////////////////

class DirtMaterial :
	public SolidMovableMaterial
{
public:
	DirtMaterial();

protected:
	sf::Color generateColor() const override;
};

//========================================================================


//////////////////////////  WaterMaterial class  /////////////////////////

class WaterMaterial :
	public LiquidMaterial
{
public:
	WaterMaterial();

private:
	sf::Color generateColor() const override;
};

//========================================================================


//////////////////////////  BrickMaterial class  /////////////////////////

class BrickMaterial :
	public SolidUnmovableMaterial
{
public:
	BrickMaterial(int x, int y);

protected:
	sf::Color generateColor(int x, int y) const override;
};

//========================================================================


////////////////////////////  OilMaterial class  //////////////////////////

class OilMaterial :
	public LiquidMaterial
{
public:
	OilMaterial();

private:
	sf::Color generateColor() const override;
};

//========================================================================


//////////////////////////  SmokeMaterial class  /////////////////////////

class SmokeMaterial :
	public GaseousMaterial
{
public:
	SmokeMaterial();

private:
	sf::Color generateColor() const override;
};

//========================================================================