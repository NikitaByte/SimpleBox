#pragma once

// Constants
constexpr uint16_t STATE_MASK     = 0b111000000000000;
constexpr uint16_t SOLID_TYPE_BIT = 0b000100000000000;
constexpr uint16_t INDEX_MASK     = static_cast<uint16_t>(~(STATE_MASK | SOLID_TYPE_BIT));


enum class MaterialState : uint16_t {
	None           = 0b000000000000000,

	// Solid Base:     001xxxxxxxxxxxx
	SolidUnmovable = 0b001000000000000,
	SolidMovable   = 0b001100000000000,

	// Liquid Base:    010xxxxxxxxxxxx
	Liquid         = 0b010000000000000,

	// Gaseous Base:   100xxxxxxxxxxxx
	Gaseous        = 0b100000000000000,
};

enum class MaterialType : uint16_t {
	// SOLID (start from 1)
	Empty = static_cast<uint16_t>(MaterialState::SolidUnmovable) | 0b000000000000001,
	Stone = static_cast<uint16_t>(MaterialState::SolidUnmovable) | 0b000000000000010,
	Brick = static_cast<uint16_t>(MaterialState::SolidUnmovable) | 0b000000000000011,
	Sand = static_cast<uint16_t>(MaterialState::SolidMovable)    | 0b000000000000100,
	Dirt = static_cast<uint16_t>(MaterialState::SolidMovable)    | 0b000000000000101,

	// LIQUID
	Water = static_cast<uint16_t>(MaterialState::Liquid) | 0b000000000000001,
	Oil = static_cast<uint16_t>(MaterialState::Liquid)   | 0b000000000000010,

	// GAS
	Smoke = static_cast<uint16_t>(MaterialState::Gaseous) | 0b000000000000001
};