#include "Materials.h"

//////////////////////////    Material class     /////////////////////////

Material::Material(MaterialType type, sf::Color color, float density)
	: type(type), color(color), density(density), updated(false) { }

MaterialType Material::getType() const {
	return this->type;
}

MaterialState Material::getState() const
{
	return static_cast<MaterialState>(static_cast<uint16_t>(this->type) & STATE_MASK);
}

sf::Color Material::getColor() const
{
	return this->color;
}

float Material::getDensity() const
{
	return this->density;
}

bool Material::isUpdated() const {
	return this->updated;
}

void Material::setUpdated(bool value) {
	this->updated = value;
}

//========================================================================


//////////////////////////  SolidMaterial class  /////////////////////////

SolidMaterial::SolidMaterial(MaterialType type, sf::Color color, float density)
	: Material(type, color, density) { }

//========================================================================

////////////////////  SolidUnmovableMaterial class  //////////////////////

SolidUnmovableMaterial::SolidUnmovableMaterial(MaterialType type, sf::Color color, float density)
	: SolidMaterial(type, color, density) { }

void SolidUnmovableMaterial::update(int x, int y, Game& game) { }

//========================================================================


/////////////////////  SolidMovableMaterial class  ///////////////////////

SolidMovableMaterial::SolidMovableMaterial(MaterialType type, sf::Color color, float density)
	: SolidMaterial(type, color, density) { }

void SolidMovableMaterial::update(int x, int y, Game& game)
{
	auto tryMove = [&](int dx, int dy) {
		int nx = x + dx;
		int ny = y + dy;

		if (!game.hasGameBorders() &&
			!game.isValidPosition(nx, ny)) {
			game.setMaterialAt(MaterialType::Empty, x, y);
			return true;
		}

		Material* target = game.getRawMaterial(nx, ny);
		if (!target) return false;

		bool isLiquid = target->getState() == MaterialState::Liquid;
		bool isGas = target->getState() == MaterialState::Gaseous;

		// Down
		if (ny > y) {
			if (game.isEmpty(nx, ny) || isGas ||
				(isLiquid && this->density > target->getDensity())) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}
		// Sideway
		else if (ny == y) {
			if (isLiquid && this->density < target->getDensity()) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}
		// Up
		else {
			if (isLiquid && this->density < target->getDensity()) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}

		return false;
		};

	int dir = game.getRandom()() % 2 ? 1 : -1;

	if (tryMove(0, 1)) return;
	if (tryMove(dir, 1)) return;
	else if (tryMove(-dir, 1)) return;
	if (tryMove(0, -1)) return;
	for (int i = 1; i <= 3; i++)
		if (tryMove(i * dir, 0)) return;
}

//========================================================================


/////////////////////////  LiquidMaterial class  /////////////////////////

LiquidMaterial::LiquidMaterial(MaterialType type, sf::Color color, float density)
	: Material(type, color, density) { }

void LiquidMaterial::update(int x, int y, Game& game) {
	auto tryMove = [&](int dx, int dy) -> bool {
		int nx = x + dx;
		int ny = y + dy;

		if (!game.hasGameBorders() &&
			!game.isValidPosition(nx, ny)) {
			game.setMaterialAt(MaterialType::Empty, x, y);
			return true;
		}

		Material* target = game.getRawMaterial(nx, ny);
		if (!target) return false;

		bool isLiquid = target->getState() == MaterialState::Liquid;
		bool isGas = target->getState() == MaterialState::Gaseous;

		// Down
		if (ny > y) {
			if (game.isEmpty(nx, ny) || isGas ||
				(isLiquid && this->density > target->getDensity())) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}
		// Sideway
		else if (ny == y) {
			if (isLiquid || isGas || game.isEmpty(nx, ny)) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}
		// Up
		else {
			if (isLiquid && this->density < target->getDensity()) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}

		return false;
		};

	int dir = game.getRandom()() % 2 ? 1 : -1;

	if (tryMove(0, 1)) return;
	if (tryMove(0, -1)) return;
	for (int i = 1; i <= 10; i++)
		if (tryMove(i * dir, 0)) continue;
}

//========================================================================


//////////////////////////  GaseousMaterial class  ///////////////////////////

GaseousMaterial::GaseousMaterial(MaterialType type, sf::Color color, float density)
	: Material(type, color, density) { }

void GaseousMaterial::update(int x, int y, Game& game)
{
	auto tryMove = [&](int dx, int dy) -> bool {
		int nx = x + dx;
		int ny = y + dy;

		if (!game.hasGameBorders() &&
			!game.isValidPosition(nx, ny)) {
			game.setMaterialAt(MaterialType::Empty, x, y);
			return true;
		}

		Material* target = game.getRawMaterial(nx, ny);
		if (!target) return false;

		bool isLiquid = target->getState() == MaterialState::Liquid;
		bool isGas = target->getState() == MaterialState::Gaseous;

		// Up
		if (dy < 0) {
			if ((isGas && this->density < target->getDensity()) || game.isEmpty(nx, ny)) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}
		// Side or down
		else {
			if (((isGas || isLiquid) && this->density < target->getDensity()) || game.isEmpty(nx, ny)) {
				game.swapMaterials(x, y, nx, ny);
				return true;
			}
		}

		return false;
		};

	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> offsetX(-1, 1);
	std::uniform_int_distribution<int> offsetY(-1, 0);

	int dx = offsetX(rng);
	int dy = offsetY(rng);

	bool upFirst = rng() % 2;

	// Main directions
	if (upFirst) {
		if (tryMove(dx, -1)) return;
		if (tryMove(dx, 0)) return; 
		if (tryMove(0, -1)) return; 
	}
	else {
		if (tryMove(dx, 0)) return; 
		if (tryMove(dx, -1)) return;
		if (tryMove(0, -1)) return; 
	}

	// Attempted random displacement
	if (tryMove(offsetX(rng), offsetY(rng))) return;
}

//========================================================================


//////////////////////////  EmptyMaterial class  /////////////////////////

EmptyMaterial::EmptyMaterial()
	: SolidUnmovableMaterial(MaterialType::Empty, DEFAULT_COLOR, 1.293f) {
}

//========================================================================


//////////////////////////  SandMaterial class  //////////////////////////

SandMaterial::SandMaterial()
	: SolidMovableMaterial(MaterialType::Sand, generateColor()) { }

sf::Color SandMaterial::generateColor() const
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> offset(-10, 10);

	int r = std::clamp(194 + offset(rng), 0, 255);
	int g = std::clamp(178 + offset(rng), 0, 255);
	int b = std::clamp(128 + offset(rng), 0, 255);

	return sf::Color(r, g, b);
}

//========================================================================


//////////////////////////  StoneMaterial class  /////////////////////////

StoneMaterial::StoneMaterial()
	: SolidUnmovableMaterial(MaterialType::Stone, generateColor()) { }

sf::Color StoneMaterial::generateColor() const {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	int shade = 90 + gen() % 30;
	return sf::Color(shade, shade, shade);
}

//========================================================================


//////////////////////////  DirtMaterial class  //////////////////////////

DirtMaterial::DirtMaterial()
	: SolidMovableMaterial(MaterialType::Dirt, generateColor(), 1500.0f) { }

sf::Color DirtMaterial::generateColor() const {
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> offset(-10, 10);

	int r = std::clamp(96 + offset(rng), 0, 255);
	int g = std::clamp(54 + offset(rng), 0, 255);
	int b = std::clamp(27 + offset(rng), 0, 255);

	return sf::Color(r, g, b);
}

//========================================================================


//////////////////////////  WaterMaterial class  /////////////////////////

WaterMaterial::WaterMaterial()
	: LiquidMaterial(MaterialType::Water, generateColor()) { }

// 58 144 220
// 27, 123, 154, 1
// 18, 79, 98, 1
sf::Color WaterMaterial::generateColor() const {
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> offset(-2, 2);

	int r = std::clamp(18 + offset(rng), 0, 255);
	int g = std::clamp(79 + offset(rng), 0, 255);
	int b = std::clamp(98 + offset(rng), 0, 255);

	return sf::Color(r, g, b);
}

//========================================================================


//////////////////////////  BrickMaterial class  /////////////////////////

BrickMaterial::BrickMaterial(int x, int y)
	: SolidUnmovableMaterial(MaterialType::Brick, generateColor(x, y)) { }

sf::Color BrickMaterial::generateColor(int x, int y) const {
	const int BRICK_WIDTH = 10;
	const int BRICK_HEIGHT = 3;
	const int MORTAR_WIDTH = 1;

	int brickRow = y / (BRICK_HEIGHT + MORTAR_WIDTH);
	int brickCol = x / (BRICK_WIDTH + MORTAR_WIDTH);

	if (brickRow % 2 == 1)
		brickCol = (x + BRICK_WIDTH / 2) / (BRICK_WIDTH + MORTAR_WIDTH);

	int localX = x - brickCol * (BRICK_WIDTH + MORTAR_WIDTH);
	int localY = y - brickRow * (BRICK_HEIGHT + MORTAR_WIDTH);

	if (brickRow % 2 == 1)
		localX = (x + BRICK_WIDTH / 2) - brickCol * (BRICK_WIDTH + MORTAR_WIDTH);

	bool isVerticalMortar = (localX >= BRICK_WIDTH);
	bool isHorizontalMortar = (localY >= BRICK_HEIGHT);

	static std::random_device rd;
	static std::mt19937 gen(rd());

	if (isVerticalMortar || isHorizontalMortar) {
		//int gray = 180;
		int gray = 180 + (gen() % 20) - 10;
		return sf::Color(gray, gray, gray);
	}
	else {
		//std::mt19937 brickGen(brickRow * 1000 + brickCol);

		//int r = 160 + (brickGen() % 60) - 30;
		//int g = 80 + (brickGen() % 40) - 20;
		//int b = 60 + (brickGen() % 30) - 15;

		int r = 160;
		int g = 80;
		int b = 60;
		//int textureVariation = (brickGen() % 20) - 10;
		//r = std::max(0, std::min(255, r + textureVariation));
		//g = std::max(0, std::min(255, g + textureVariation));
		//b = std::max(0, std::min(255, b + textureVariation));

		return sf::Color(r, b, g);
	}
}

//========================================================================


////////////////////////////  OilMaterial class  //////////////////////////
//154, 158, 43, 1
OilMaterial::OilMaterial()
	: LiquidMaterial(MaterialType::Oil, generateColor(), 900.0f) { }

sf::Color OilMaterial::generateColor() const {
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> offset(-2, 2);

	int r = std::clamp(154 + offset(rng), 0, 255);
	int g = std::clamp(158 + offset(rng), 0, 255);
	int b = std::clamp(43 + offset(rng), 0, 255);

	return sf::Color(r, g, b);
}

//========================================================================


//////////////////////////  SmokeMaterial class  /////////////////////////

SmokeMaterial::SmokeMaterial()
	: GaseousMaterial(MaterialType::Smoke, generateColor()) { }

sf::Color SmokeMaterial::generateColor() const {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	int shade = 50 + gen() % 20;
	return sf::Color(shade, shade, shade);
}

//========================================================================