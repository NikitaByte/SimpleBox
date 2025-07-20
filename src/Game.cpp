// Project headers
#include "Game.h"
#include "Materials.h"

// STL
#include <iostream>
#include <memory>
#include <unordered_set>
#include <sstream>

// WinAPI
#include <windows.h>


// === GLOBAL RESOLUTION PARAMETERS ===
sf::Vector2u windowSize = Screen::R16x9::HD;
sf::Vector2u gameSize = Screen::R16x9::HD;
int baseFontSize = 45;

float scaleX = static_cast<float>(gameSize.x) / BASE_RESOLUTION.x;
float scaleY = static_cast<float>(gameSize.y) / BASE_RESOLUTION.y;
float scale = (scaleX + scaleY) / 2.0f;
float gameScale = 1.0f;

int cellSize = std::max(static_cast<int>(4 * gameScale * scale), 1);
int windowWidth = windowSize.x;
int windowHeight = windowSize.y;
int gridWidth = gameSize.x / cellSize;
int gridHeight = gameSize.y / cellSize;

UIScaler uiScaler(BASE_RESOLUTION, gameSize);

// === PUBLIC METHODS ===
// === Constructors ===
Game::Game()
	: window(initWindow()),
	currentMaterial(MaterialType::Sand),
	isFullscreen(false),
	isPaused(false),
	hasBorders(true),
	showFps(false),
	brushSize(5),
	brushSolidity(0.1f),
	brushShape(BrushShape::CIRCLE),
	gen(rd())
{
	int centerX = gameSize.x / 2;
	int centerY = gameSize.y / 2;

	if (!this->icon.loadFromFile("resources/images/icon.png")) exit(EXIT_FAILURE);

	// Window settings
	this->window->setFramerateLimit(maxFps);
	this->window->setMouseCursorVisible(false);
	this->window->setIcon(32, 32, this->icon.getPixelsPtr());
	this->resizeViewFit();

	// Init text GUI
	if (!this->defaultFont.loadFromFile("resources/fonts/ByteBounce.ttf")) exit(EXIT_FAILURE);
	this->fontSize = uiScaler.scaleFontSize(baseFontSize);

	// Selected material text
	this->selectedMaterialText.setFont(this->defaultFont);
	this->selectedMaterialText.setFillColor(sf::Color::White);
	this->selectedMaterialText.setCharacterSize(fontSize);
	this->selectedMaterialText.setPosition(uiScaler.scalePosition(showFps ? sf::Vector2f(10, 35) : sf::Vector2f(10, 0)));
	this->selectedMaterialText.setString("Sand");

	// Pause text
	this->pauseText.setFont(this->defaultFont);
	this->pauseText.setFillColor(sf::Color::White);
	this->pauseText.setCharacterSize(fontSize);
	this->pauseText.setString("Pause");
	sf::FloatRect pauseTextBounds = this->pauseText.getLocalBounds();
	this->pauseText.setOrigin(pauseTextBounds.left + pauseTextBounds.width / 2.0f, 0.0f);
	this->pauseText.setPosition(uiScaler.scalePosition(sf::Vector2f(centerX, 0)));

	// FPS text
	this->fpsText.setFont(this->defaultFont);
	this->fpsText.setFillColor(sf::Color::White);
	this->fpsText.setCharacterSize(fontSize);
	this->fpsText.setPosition(uiScaler.scalePosition(sf::Vector2f(10, 0)));
	this->fpsText.setString("FPS: " + std::to_string(static_cast<int>(this->fps)));

	std::ostringstream ss;
	ss.precision(1);
	ss << ss.fixed << this->brushSolidity;

	// Brush data text
	this->brushInfoText.setFont(this->defaultFont);
	this->brushInfoText.setFillColor(sf::Color::White);
	this->brushInfoText.setCharacterSize(fontSize);
	this->brushInfoText.setString("Brush: Size: " + std::to_string(this->brushSize) + " Solidity: " + ss.str());
	sf::FloatRect brushInfoTextBounds = this->brushInfoText.getLocalBounds();
	this->brushInfoText.setOrigin(0, brushInfoTextBounds.top + brushInfoTextBounds.height);
	this->brushInfoText.setPosition(10.f, static_cast<float>(windowSize.y - 10));

	// Temporary message
	this->messageText.setFont(this->defaultFont);
	this->messageText.setFillColor(sf::Color::White);
	this->messageText.setCharacterSize(fontSize);
	this->messageText.setString("");
	sf::FloatRect messageTextBounds = this->messageText.getLocalBounds();
	this->messageText.setOrigin(messageTextBounds.left + messageTextBounds.width / 2.0f, 
		messageTextBounds.top + messageTextBounds.height / 2.0f);
	this->messageText.setPosition(uiScaler.scalePosition(sf::Vector2f(centerX, 150.0f)));

	// Init vertex grid
	this->vertexGrid.setPrimitiveType(sf::Quads);
	this->vertexGrid.resize(gridWidth * gridHeight * 4);
	this->initVertexGrid();

	this->printTips();
}

Game::~Game() { }

// === Accessors ===
const bool Game::running() const
{
	return this->window->isOpen();
}

sf::Vector2u Game::getWindowSize() const
{
	return this->window->getSize();
}

bool Game::hasGameBorders() const
{
	return this->hasBorders;
}

std::mt19937& Game::getRandom()
{
	return this->gen;
}

// === Grid helpers ===
bool Game::isValidPosition(int x, int y) const
{
	return x >= 0 && x < gridWidth && y >= 0 && y < gridHeight;
}

bool Game::isEmpty(int x, int y) const
{
	return isValidPosition(x, y) &&
		this->grid[y][x]->getType() == MaterialType::Empty;
}

MaterialType Game::getMaterialType(int x, int y) const
{
	if (!this->isValidPosition(x, y))
		return MaterialType::Empty;
	return this->grid[y][x]->getType();
}

Material* Game::getRawMaterial(int x, int y)
{
	if (!isValidPosition(x, y)) return nullptr;
	return grid[y][x].get();
}

void Game::setMaterialAt(MaterialType material, int x, int y)
{
	if (this->isValidPosition(x, y))
		this->grid[y][x] = createMaterial(material, x, y);
}

void Game::swapMaterials(int x1, int y1, int x2, int y2)
{
	if (this->isValidPosition(x1, y1) &&
		this->isValidPosition(x2, y2))
		std::swap(this->grid[y1][x1], this->grid[y2][x2]);
}

// === Main logic ===
void Game::update()
{
	/*
		@return void

		- event processing
		- update grid
		- update vertex colors
		- update selected material text
		- update FPS

		Updates game objects per frame.
	*/

	this->handleEvents();

	if (!isPaused) {
		// Reset the update checkboxes
		for (int y = 0; y < gridHeight; y++)
			for (int x = 0; x < gridWidth; x++)
				grid[y][x]->setUpdated(false);

		// Change the update direction left/right every frame
		static bool leftToRight = true;
		leftToRight = !leftToRight;

		// Updating the grid from the bottom up
		for (int y = gridHeight - 1; y >= 0; y--) {
			if (leftToRight) {
				for (int x = 0; x < gridWidth; x++) {
					if (grid[y][x]->isUpdated() ||
						grid[y][x]->getType() == MaterialType::Empty)
						continue;

					grid[y][x]->setUpdated(true);
					grid[y][x]->update(x, y, *this);
				}
			}
			else {
				for (int x = gridWidth - 1; x >= 0; x--) {
					if (grid[y][x]->isUpdated() ||
						grid[y][x]->getType() == MaterialType::Empty)
						continue;

					grid[y][x]->setUpdated(true);
					grid[y][x]->update(x, y, *this);
				}
			}
		}
	}

	this->updateVertexColors();

	// Update UI
	std::ostringstream ss;
	ss.precision(1);
	ss << std::fixed << this->brushSolidity;

	this->brushInfoText.setString("Brush: Size: " + std::to_string(this->brushSize) + " Solidity: " + ss.str());

	if (showFps)
		this->updateFPS();

	if (this->messageClock.getElapsedTime().asSeconds() > this->messageDuration)
		this->showMessage = false;
}

void Game::render()
{
	/*
		@return void

		- clear old frame
		- render game objects
		- display frame in window

		Renders the game objects.
	*/

	this->window->clear(sf::Color::Black);

	// Draw game objects
	// Draw vertex grid
	this->window->draw(this->vertexGrid);
	
	// Draw a pen
	drawPen();

	// Draw UI
	this->window->draw(this->selectedMaterialText);

	this->window->draw(this->brushInfoText);

	if (showFps)
		this->window->draw(this->fpsText);

	if (isPaused)
		this->window->draw(this->pauseText);

	if (showMessage)
		this->window->draw(this->messageText);

	this->window->display();
}

// === PRIVATE METHODS ===
// === Init Methods ===
std::unique_ptr<sf::RenderWindow> Game::initWindow()
{
	return std::make_unique<sf::RenderWindow>(
		sf::VideoMode(windowWidth, windowHeight),
		WINDOW_TITLE,
		sf::Style::Default
	);
}

void Game::initVertexGrid() {
	// Fill the grid cells with Empty material
	this->grid.resize(gridHeight);
	for (int y = 0; y < gridHeight; y++) {
		this->grid[y].resize(gridWidth);
		for (int x = 0; x < gridWidth; x++)
			this->setMaterialAt(MaterialType::Empty, x, y);
	}

	// Create the vertex grid
	for (int y = 0; y < gridHeight; y++) {
		for (int x = 0; x < gridWidth; x++) {
			int i = (x + y * gridWidth) * 4;
			sf::Vertex* quad = &vertexGrid[i];

			float px = x * cellSize;
			float py = y * cellSize;

			quad[0].position = { px, py };
			quad[1].position = { px + cellSize, py };
			quad[2].position = { px + cellSize, py + cellSize };
			quad[3].position = { px, py + cellSize };
		}
	}
}


// === Update Methods ===
void Game::handleEvents()
{
	/*
		@return void

		- handle window input
		- handle keyboard input
		- handle mouse input

		Handles events
	*/

	// Event polling
	while (this->window->pollEvent(this->event))
	{
		// Handle window input
		if (event.type == sf::Event::Closed) {
			this->clearConsoleRow();
			std::cout << "Game OVER";
			this->window->close();
		}

		if (event.type == sf::Event::Resized)
			updateView(this->windowMode);

		// Handle keyboard input
		if (event.type == sf::Event::KeyPressed) {
			switch (this->event.key.code) {
			case sf::Keyboard::Num1:
				this->currentMaterial = MaterialType::Sand;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Sand SELECTED";
				break;
			case sf::Keyboard::Num2:
				this->currentMaterial = MaterialType::Dirt;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Dirt SELECTED";
				break;
			case sf::Keyboard::Num3:
				this->currentMaterial = MaterialType::Stone;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Stone SELECTED";
				break;
			case sf::Keyboard::Num4:
				this->currentMaterial = MaterialType::Water;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Water SELECTED";
				break;
			case sf::Keyboard::Num5:
				this->currentMaterial = MaterialType::Brick;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Brick SELECTED";
				break;
			case sf::Keyboard::Num6:
				this->currentMaterial = MaterialType::Oil;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Oil SELECTED";
				break;
			case sf::Keyboard::Num7:
				this->currentMaterial = MaterialType::Smoke;
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << "Smoke SELECTED";
				break;
			case sf::Keyboard::Equal:
				this->brushSize = std::min(this->brushSize + 1, 25);
				this->clearConsoleRow();
				std::cout << "Brush Size: " << this->brushSize;
				break;
			case sf::Keyboard::Hyphen:
				this->brushSize = std::max(this->brushSize - 1, 0);
				this->clearConsoleRow();
				std::cout << "Brush Size: " << this->brushSize;
				break;
			case sf::Keyboard::Up:
				this->brushSolidity = std::min(this->brushSolidity + 0.1f, 1.0f);
				this->clearConsoleRow();
				std::cout << "Brush Solidity: " << this->brushSolidity;
				break;
			case sf::Keyboard::Down:
				this->brushSolidity = std::max(this->brushSolidity - 0.1f, 0.1f);
				this->clearConsoleRow();
				std::cout << "Brush Solidity: " << this->brushSolidity;
				break;
			case sf::Keyboard::C:
				for (int y = 0; y < gridHeight; y++)
					for (int x = 0; x < gridWidth; x++)
						this->grid[y][x] = std::make_unique<EmptyMaterial>();
				this->showTemporaryMessage("Area cleared");
				this->clearConsoleRow();
				std::cout << "Area CLEARED";
				break;
			case sf::Keyboard::B:
				this->hasBorders = !this->hasBorders;
				this->showTemporaryMessage(hasBorders ? "Borders are enabled" : "Borders are disabled");
				this->clearConsoleRow();
				std::cout << (hasBorders ? "Borders are ENABLED" : "Borders are DISABLED");
				break;
			case sf::Keyboard::P: {
					int shapeNum = static_cast<int>(this->brushShape);
					shapeNum = (shapeNum + 1) % 3;
					this->brushShape = static_cast<BrushShape>(shapeNum);
					std::string shape = shapeNum == 0 ? "Circle" : shapeNum == 1 ? "Square" : "Triangle";
					this->showTemporaryMessage(shape + " brush shape selected");
					this->clearConsoleRow();
					std::cout << shape << " brush shape SELECTED";
					break;
				}
			case sf::Keyboard::V: {
				int windowModeNum = static_cast<int>(this->windowMode);
				windowModeNum = (windowModeNum + 1) % 2;
				this->windowMode = static_cast<WindowMode>(windowModeNum);
				std::string windowModeSelected = windowModeNum == 0 ? "Strech" : windowModeNum == 1 ? "Fit" : "PixelPerfect";
				this->showTemporaryMessage(windowModeSelected + " window mode selected");
				this->clearConsoleRow();
				std::cout << windowModeSelected << " window mode SELECTED";
				updateView(this->windowMode);
				break;
			}
			case sf::Keyboard::F:
				this->showFps = !this->showFps;
				if (!showFps) {
					this->fps = 0;
					this->frameCount = 0;
				}
				this->updateSelectedMaterialText();
				this->clearConsoleRow();
				std::cout << (showFps ? "FPS display ENABLED" : "FPS display DISABLED");
				break;
			case sf::Keyboard::Pause:
				this->isPaused = !this->isPaused;
				this->clearConsoleRow();
				std::cout << (isPaused ? "Game PAUSED" : "Game RESUMED");
				break;
			case sf::Keyboard::F11:
				isFullscreen = !isFullscreen;
				if (isFullscreen) {
					this->window->create(sf::VideoMode::getDesktopMode(), WINDOW_TITLE, sf::Style::Fullscreen);
					updateView(this->windowMode);
				}	
				else {
					this->window->create(sf::VideoMode(windowWidth, windowHeight), WINDOW_TITLE, sf::Style::Default);
					updateView(this->windowMode);
				}
				this->window->setFramerateLimit(maxFps);
				this->window->setMouseCursorVisible(false);
				this->window->setIcon(32, 32, this->icon.getPixelsPtr());
				break;
			case sf::Keyboard::Escape:
				this->clearConsoleRow();
				std::cout << "Game OVER";
				this->window->close();
				break;
			case sf::Keyboard::Left:
				gameScale = std::max(gameScale - 0.1f, 0.4f);
				cellSize = std::max(static_cast<int>(4 * gameScale * scale), 1);
				gridWidth = gameSize.x / cellSize;
				gridHeight = gameSize.y / cellSize;
				this->vertexGrid.resize(gridWidth * gridHeight * 4);
				this->initVertexGrid();
				break;
			case sf::Keyboard::Right:
				gameScale = std::min(gameScale + 0.1f, 2.0f);
				cellSize = std::max(static_cast<int>(4 * gameScale * scale), 1);
				gridWidth = gameSize.x / cellSize;
				gridHeight = gameSize.y / cellSize;
				this->vertexGrid.resize(gridWidth * gridHeight * 4);
				this->initVertexGrid();
				break;
			}
		}
	}

	// Handle mouse input
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		spawnMaterial();

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		clearArea();
}

void Game::updateVertexColors() {
	/*
		@return void

		Updates colors of the vertex grid
	*/

	for (int y = 0; y < gridHeight; y++) {
		for (int x = 0; x < gridWidth; x++) {
			int i = (x + y * gridWidth) * 4;

			sf::Color color = DEFAULT_COLOR;
			const std::unique_ptr<Material>& material = grid[y][x];
			if (material && material->getType() != MaterialType::Empty)
				color = material->getColor();

			for (int j = 0; j < 4; j++)
				vertexGrid[i + j].color = color;
		}
	}
}

void Game::updateFPS() {
	frameCount++;
	if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
		fps = frameCount / fpsClock.getElapsedTime().asSeconds();
		frameCount = 0;
		fpsClock.restart();

		this->fpsText.setString("FPS: " + std::to_string(static_cast<int>(this->fps)));
	}
}

void Game::updateSelectedMaterialText()
{
	switch (this->currentMaterial) {
	case MaterialType::Sand:
		this->selectedMaterialText.setString("Sand");
		break;
	case MaterialType::Dirt:
		this->selectedMaterialText.setString("Dirt");
		break;
	case MaterialType::Stone:
		this->selectedMaterialText.setString("Stone");
		break;
	case MaterialType::Water:
		this->selectedMaterialText.setString("Water");
		break;
	case MaterialType::Brick:
		this->selectedMaterialText.setString("Brick");
		break;
	case MaterialType::Oil:
		this->selectedMaterialText.setString("Oil");
		break;
	case MaterialType::Smoke:
		this->selectedMaterialText.setString("Smoke");
		break;
	}

	this->selectedMaterialText.setPosition(uiScaler.scalePosition(showFps ? sf::Vector2f(10, 35) : sf::Vector2f(10, 0)));
}


// === View Management ===
void Game::updateView(WindowMode mode)
{
	switch (mode) {
	case WindowMode::Stretch:
		resizeViewStrech();
		break;
	case WindowMode::Fit:
		resizeViewFit();
		break;
	}
}

void Game::resizeViewFit()
{
	sf::Vector2u winSize = window->getSize();

	float windowRatio = static_cast<float>(winSize.x) / winSize.y;
	float viewRatio = static_cast<float>(gameSize.x) / gameSize.y;

	sf::View view;
	if (windowRatio > viewRatio) {
		float scale = static_cast<float>(winSize.y) / gameSize.y;
		float width = gameSize.x * scale;
		float viewportWidth = static_cast<float>(winSize.y) * viewRatio / winSize.x;

		view.setViewport(sf::FloatRect((1.f - viewportWidth) / 2.f, 0.f, viewportWidth, 1.f));
	}
	else {
		float viewportHeight = static_cast<float>(winSize.x) / viewRatio / winSize.y;

		view.setViewport(sf::FloatRect(0.f, (1.f - viewportHeight) / 2.f, 1.f, viewportHeight));
	}

	view.setSize(static_cast<float>(gameSize.x), static_cast<float>(gameSize.y));
	view.setCenter(gameSize.x / 2.f, gameSize.y / 2.f);

	window->setView(view);
}

void Game::resizeViewStrech()
{
	sf::View view;

	view.setSize(static_cast<float>(gameSize.x), static_cast<float>(gameSize.y));
	view.setCenter(gameSize.x / 2.f, gameSize.y / 2.f);
	view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

	this->window->setView(view);
}


// === Drawing ===
void Game::drawPen() {
	/*
		@return void

		Draws a selection of the drawing area
	*/

	sf::Vector2i worldMousePos = getMousePosition();

	forEachInBrush(worldMousePos, [&](int x, int y) {
		drawCell(x, y, sf::Color(255, 255, 255, 130));
		}, BrushActionType::DRAW);
}

void Game::drawCell(int x, int y, sf::Color color)
{
	/*
		@return void

		Draws a cell
	*/

	const float px = x * cellSize;
	const float py = y * cellSize;

	sf::VertexArray cell(sf::Quads, 4);

	cell[0].position = { px,            py };
	cell[1].position = { px + cellSize, py };
	cell[2].position = { px + cellSize, py + cellSize };
	cell[3].position = { px,            py + cellSize };

	for (int i = 0; i < 4; i++)
		cell[i].color = color;

	this->window->draw(cell);
}


// === Interaction ===
void Game::spawnMaterial()
{
	/*
		@return void

		Ñreates a material area in the grid with the mouse
	*/

	sf::Vector2i worldMousePos = getMousePosition();

	forEachInBrush(worldMousePos, 
		[&](int x, int y) {
			auto isSolidUnmovable = [&]() -> bool {
				return static_cast<MaterialState>(static_cast<uint16_t>(this->currentMaterial) & (STATE_MASK | SOLID_TYPE_BIT)) == MaterialState::SolidUnmovable;
				};
		if (isSolidUnmovable() || getRandom()() / static_cast<float>(this->gen.max()) <= this->brushSolidity)
			setMaterialAt(this->currentMaterial, x, y);
		},
		BrushActionType::SPAWN);
}

std::unique_ptr<Material> Game::createMaterial(MaterialType type, int x = 0, int y = 0)
{
	/*
		@return std::unique_ptr<Material>

		Ñreates material at certain grid coordinates
	*/

	switch (type) {
		case MaterialType::Sand: return std::make_unique<SandMaterial>();
		case MaterialType::Dirt: return std::make_unique<DirtMaterial>();
		case MaterialType::Water: return std::make_unique<WaterMaterial>();
		case MaterialType::Stone: return std::make_unique<StoneMaterial>();
		case MaterialType::Brick: return std::make_unique<BrickMaterial>(x, y);
		case MaterialType::Oil: return std::make_unique<OilMaterial>();
		case MaterialType::Smoke: return std::make_unique<SmokeMaterial>();
		default: return std::make_unique<EmptyMaterial>();
	}
}

void Game::clearArea()
{
	/*
		@return void

		Erases the material area in the grid with the mouse
	*/

	sf::Vector2i worldMousePos = getMousePosition();

	forEachInBrush(worldMousePos, [&](int x, int y) {
		setMaterialAt(MaterialType::Empty, x, y);
		}, BrushActionType::CLEAR);
}

template<typename Func>
void Game::forEachInBrush(const sf::Vector2i& mousePos, Func func, BrushActionType action)
{
	/*
		@return void

		Shapes a region and draws a selection, inserts material, and erases a region
	*/

	int centerX = mousePos.x / cellSize;
	int centerY = mousePos.y / cellSize;

	// TRIANGLE
	if (this->brushShape == BrushShape::TRIANGLE) {
		centerY -= this->brushSize;
		int fullSize = brushSize * 2;

		for (int dy = -1; dy <= fullSize; dy++) {
			float ratio = static_cast<float>(dy + 1) / fullSize;
			int halfWidth = static_cast<int>(std::round((ratio * fullSize) / 2.0f));

			for (int dx = -halfWidth; dx <= halfWidth; dx++) {
				int x = centerX + dx;
				int y = centerY + dy;

				if (!isValidPosition(x, y)) continue;

				if (action == BrushActionType::DRAW) {
					bool onEdge = dx == -halfWidth || dx == halfWidth || dy == fullSize;
					if (!onEdge) continue;
				}

				func(x, y);
			}
		}
		return;
	}

	// CIRCLE / SQUARE
	for (int dx = -brushSize; dx <= brushSize; dx++) {
		for (int dy = -brushSize; dy <= brushSize; dy++) {
			int x = centerX + dx;
			int y = centerY + dy;

			if (!isValidPosition(x, y)) continue;

			if (this->brushShape == BrushShape::CIRCLE) {
				int distSq = dx * dx + dy * dy;

				if (action == BrushActionType::DRAW) {
					if (std::round(std::sqrt(distSq)) != this->brushSize) continue;
				}
				else if (distSq > this->brushSize * this->brushSize) continue;
			}
			else if (this->brushShape == BrushShape::SQUARE &&
				action == BrushActionType::DRAW)
				if (abs(dx) != this->brushSize && abs(dy) != this->brushSize) continue;

			func(x, y);
		}
	}
}

sf::Vector2i Game::getMousePosition()
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
	sf::Vector2f worldPos = window->mapPixelToCoords(pixelPos);
	return { static_cast<int>(worldPos.x), static_cast<int>(worldPos.y) };
}


// === GUI ===
void Game::showTemporaryMessage(const std::string& text, float duration)
{
	int centerX = gameSize.x / 2;
	int centerY = gameSize.y / 2;
	
	this->messageText.setString(text);
	sf::FloatRect bounds = this->messageText.getLocalBounds();

	this->messageText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
	this->messageText.setPosition(uiScaler.scalePosition(sf::Vector2f(centerX, 150.0f)));
	
	this->messageClock.restart();
	this->messageDuration = duration;
	this->showMessage = true;
}


// === Console output ===
void Game::printTips()
{
	std::cout << "SFML SandBox Game - SimpleBox v1.0" << std::endl << std::endl;
	std::cout << "Controls:" << std::endl;
	std::cout << "1 - Sand" << std::endl;
	std::cout << "2 - Dirt" << std::endl;
	std::cout << "3 - Stone" << std::endl;
	std::cout << "4 - Water" << std::endl;
	std::cout << "5 - Brick" << std::endl;
	std::cout << "6 - Oil" << std::endl;
	std::cout << "7 - Smoke" << std::endl;
	std::cout << "Left Mouse Click - Spawn material" << std::endl;
	std::cout << "Right Mouse Click - Erase" << std::endl;
	std::cout << "+ - Increase brush size" << std::endl;
	std::cout << "- - Decrease brush size" << std::endl;
	std::cout << "P - Change brush shape" << std::endl;
	std::cout << "Arrow Up - Increase brush solidity" << std::endl;
	std::cout << "Arrow Down - Decrease brush solidity" << std::endl;
	std::cout << "Arrow Right - Scaling up" << std::endl;
	std::cout << "Arrow Left - Scaling down" << std::endl;
	std::cout << "C - Clear screen" << std::endl;
	std::cout << "B - Enable/Disable borders" << std::endl;
	std::cout << "V - Resize view/Ñhange window mode (Fit/Stretch)" << std::endl;
	std::cout << "F11 - Displaying the game (Window/Fullscreen)" << std::endl;
	std::cout << "Pause - Pause" << std::endl;
	std::cout << "ESC - End the game" << std::endl << std::endl;
	std::cout << "Game STARTED";
}

void Game::clearConsoleRow()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int width = 80;

	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		width = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	std::cout << "\r" << std::string(width - 1, ' ') << "\r";
}