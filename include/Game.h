#pragma once

/*
	Class that represents the game engine.
	Wrapper class.
*/

// STL
#include <random>
#include <vector>

// SFML
#include <SFML/Graphics.hpp>

// Project headers
#include "MaterialEnums.h"
#include "UIScaler.h"


// === ENUMS & STRUCTS ===
enum class WindowMode { Stretch, Fit };
enum class BrushActionType { SPAWN, CLEAR, DRAW };
enum class BrushShape { CIRCLE, SQUARE, TRIANGLE };

struct Screen {
	struct R11x9 { // 1.22
		static inline const sf::Vector2u CIF        = { 352  , 288  }; // 11:9  -   Video conferences, monitoring
	};

	struct R5x4 { // 1.25
		static inline const sf::Vector2u SXGA       = { 1280 , 1024 }; // 5:4   -   Office monitors
	};

	struct R4x3 { // 1.33
		static inline const sf::Vector2u QVGA       = { 320  , 240  }; // 4:3   -   Older mobile devices
		static inline const sf::Vector2u VGA        = { 640  , 480  }; // 4:3   -   Older monitors
		static inline const sf::Vector2u SVGA       = { 800  , 600  }; // 4:3   -   Older monitors
		static inline const sf::Vector2u XGA        = { 1024 , 768  }; // 4:3   -   Office monitors
		static inline const sf::Vector2u XVGA       = { 1280 , 960  }; // 4:3   -   Office monitors
		static inline const sf::Vector2u SXGA_PLUS  = { 1400 , 1050 }; // 4:3   -   Office monitors
		static inline const sf::Vector2u UXGA       = { 1600 , 1200 }; // 4:3   -   Professional monitors
		static inline const sf::Vector2u QXGA       = { 2048 , 1536 }; // 4:3   -   Professional monitors
	};

	struct R8x5 { // 1.6
		static inline const sf::Vector2u CGA        = { 320  , 200  }; // 8:5   -   Retro computers
	};

	struct R16x10 { // 1.6
		static inline const sf::Vector2u WXGA       = { 1280 , 800  }; // 16:10
		static inline const sf::Vector2u WXGA_PLUS  = { 1440 , 900  }; // 16:10
		static inline const sf::Vector2u WSXGA_PLUS = { 1680 , 1050 }; // 16:10
		static inline const sf::Vector2u WUXGA      = { 1920 , 1200 }; // 16:10 -   Professional monitors
		static inline const sf::Vector2u WQXGA      = { 2560 , 1600 }; // 16:10 -   Professional monitors
	};

	struct R5x3 { // 1.67
		static inline const sf::Vector2u WQVGA      = { 400  , 240  }; // 5:3   -   Mobile devices
		static inline const sf::Vector2u WVGA       = { 800  , 480  }; // 5:3   -   Mobile devices, netbooks
	};

	struct R16x9 {  // 1.7
		static inline const sf::Vector2u HD         = { 1280 , 720  }; // 16:9  -   Televisions, monitors
		static inline const sf::Vector2u WXGA       = { 1366 , 768  }; // 16:9  -   Laptops, TVs
		static inline const sf::Vector2u FHD        = { 1920 , 1080 }; // 16:9  -   Televisions, monitors, laptops
		static inline const sf::Vector2u QWXGA      = { 2048 , 1152 }; // 16:9
		static inline const sf::Vector2u QHD        = { 2560 , 1440 }; // 16:9  -   Gaming, professional monitors
		static inline const sf::Vector2u UHD4K      = { 3840 , 2160 }; // 16:9  -   Televisions, professional monitors
		static inline const sf::Vector2u UHD5K      = { 5120, 2880  }; // 16:9  -   Televisions, professional monitors
		static inline const sf::Vector2u UHD6K      = { 6144 , 3456 }; // 16:9  -   Televisions, professional monitors
		static inline const sf::Vector2u UHD8K      = { 7680 , 4320 }; // 16:9  -   Televisions, professional monitors
	};

	struct R64x35 { // 1.83
		static inline const sf::Vector2u EGA        = { 640  , 350  }; // 64:35 -   Retro computers
	};

	struct R21x9 { // 2.33
		static inline const sf::Vector2u UWFHD      = { 2560 , 1080 }; // 21:9
		static inline const sf::Vector2u UWQHD      = { 3440 , 1440 }; // 21:9
	};

	struct R32x9 { // 3.55
		static inline const sf::Vector2u DUHD       = { 7680 , 2160 }; // 32:9
	};
};


// === GLOBAL RESOLUTION PARAMETERS ===
inline const std::string WINDOW_TITLE = "SimpleBox v0.1";
inline const sf::Vector2u BASE_RESOLUTION = Screen::R16x9::HD;

extern sf::Vector2u windowSize;
extern sf::Vector2u gameSize;
extern int baseFontSize;

extern float scaleX, scaleY, scale, gameScale;
extern int cellSize;
extern int windowWidth, windowHeight;
extern int gridWidth, gridHeight;

extern UIScaler uiScaler;


// === Forward declarations ===
class Material;


// === Game class ===
class Game
{
public:
	// === Constructors ===
	Game();
	virtual ~Game();

	// === Accessors ===
	const bool running() const;
	sf::Vector2u getWindowSize() const;
	bool hasGameBorders() const;
	std::mt19937& getRandom();

	// === Grid helpers ===
	bool isValidPosition(int x, int y) const;
	bool isEmpty(int x, int y) const;
	MaterialType getMaterialType(int x, int y) const;
	Material* getRawMaterial(int x, int y);
	void setMaterialAt(MaterialType material, int x, int y);
	void swapMaterials(int x1, int y1, int x2, int y2);

	// === Main logic ===
	void update();
	void render();

private:
	// === Init Methods ===
	std::unique_ptr<sf::RenderWindow> initWindow();
	void initVertexGrid();

	// === Update Methods ===
	void handleEvents();
	void updateVertexColors();
	void updateFPS();
	void updateSelectedMaterialText();

	// === View Management ===
	void updateView(WindowMode mode);
	void resizeViewFit();
	void resizeViewStrech();

	// === Drawing ===
	void drawCell(int x, int y, sf::Color color);
	void drawPen();

	// === Interaction ===
	void spawnMaterial();
	std::unique_ptr<Material> createMaterial(MaterialType type, int x, int y);
	void clearArea();
	template <typename Func>
	void forEachInBrush(const sf::Vector2i& mousePos, Func func, BrushActionType action);
	sf::Vector2i getMousePosition();

	// === GUI ===
	void showTemporaryMessage(const std::string& text, float duration = 2.0f);

	// === Console output ===
	void printTips();
	void clearConsoleRow();

private:
	// === Window ===
	std::unique_ptr<sf::RenderWindow> window;
	sf::View view;
	sf::Event event;
	bool isFullscreen;
	WindowMode windowMode;

	// === Game State ===
	bool isPaused;
	bool hasBorders;
	bool showFps;
	sf::Image icon;

	// === Grid ===
	std::vector<std::vector<std::unique_ptr<Material>>> grid;
	sf::VertexArray vertexGrid;

	// === Brush ===
	MaterialType currentMaterial;
	int brushSize;
	float brushSolidity;
	BrushShape brushShape;

	// === Random ===
	std::random_device rd;
	std::mt19937 gen;

	// === FPS ===
	sf::Clock fpsClock;
	int frameCount = 0;
	int maxFps = 120;
	float fps = 0.f;

	// === Text UI ===
	sf::Font defaultFont;
	int fontSize;
	sf::Text selectedMaterialText;
	sf::Text pauseText;
	sf::Text fpsText;
	sf::Text brushInfoText;
	sf::Clock messageClock;
	sf::Text messageText;
	bool showMessage = false;
	float messageDuration = 2.0f;
};