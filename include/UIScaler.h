#pragma once
#include <SFML/Graphics.hpp>

/*
	Class scaler for the user interface for different screen resolutions.
	Wrapper class.
*/

class UIScaler
{
public:
	UIScaler(const sf::Vector2u& baseResolution, const sf::Vector2u& currentResolution);

	sf::Vector2f scalePosition(const sf::Vector2f& basePosition) const;
	sf::Vector2f scaleSize(const sf::Vector2f& baseSize) const;
	int scaleFontSize(int baseFontSize) const;
	float getScaleX() const;
	float getScaleY() const;
	float getAverageScale() const;
private:
	float scaleX;
	float scaleY;
};

