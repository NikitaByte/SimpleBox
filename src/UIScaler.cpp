#include "UIScaler.h"

UIScaler::UIScaler(const sf::Vector2u& baseResolution, const sf::Vector2u& currentResolution)
{
	this->scaleX = static_cast<float>(currentResolution.x) / baseResolution.x;
	this->scaleY = static_cast<float>(currentResolution.y) / baseResolution.y;
}

sf::Vector2f UIScaler::scalePosition(const sf::Vector2f& basePosition) const
{
	return sf::Vector2f(basePosition.x * this->scaleX, basePosition.y * this->scaleY);
}

sf::Vector2f UIScaler::scaleSize(const sf::Vector2f& baseSize) const
{
	return sf::Vector2f(baseSize.x * this->scaleX, baseSize.y * this->scaleY);
}

int UIScaler::scaleFontSize(int baseFontSize) const
{ 
	float averageScale = this->getAverageScale();
	return std::max(static_cast<int>(baseFontSize * averageScale), 15);
}

float UIScaler::getScaleX() const
{
	return this->scaleX;
}

float UIScaler::getScaleY() const
{
	return this->scaleY;
}

float UIScaler::getAverageScale() const
{
	return (this->scaleX + this->scaleY) / 2.0f;
}
