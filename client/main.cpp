#include "raylib.h"
#include <vector>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

struct Vector2i {
	int x;
	int y;

	Vector2i operator + (const Vector2i& other) {
		return { x + other.x, y + other.y };
	}

	Vector2i operator += (const Vector2i& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
};

enum class InputCommand {
	None = 0,
	Up = 1,
	Down = 2,
	Left = 4,
	Right = 8,
};

struct Entity {
	Vector2i position;
	Vector2i velocity;
};

struct SimulationState {
	Entity entity;
	unsigned int commands;
};

struct Field {
	std::vector<SimulationState> states;
};

int Pings2Tick(int pings) {
	return pings / 16.667f;
}

void DrawEntity(const Entity& entity, const Color& color) {
	DrawCircle(entity.position.x + SCREEN_WIDTH / 2, entity.position.y + SCREEN_HEIGHT / 2, 100, color);
}

void UpdateCommands(SimulationState& state) {
	state.commands = static_cast<unsigned int>(InputCommand::None);
	if (IsKeyDown(KEY_UP)) {
		state.commands |= static_cast<unsigned int>(InputCommand::Up);
	}
	else if (IsKeyDown(KEY_DOWN)) {
		state.commands |= static_cast<unsigned int>(InputCommand::Down);
	}
	if (IsKeyDown(KEY_LEFT)) {
		state.commands |= static_cast<unsigned int>(InputCommand::Left);
	}
	else if (IsKeyDown(KEY_RIGHT)) {
		state.commands |= static_cast<unsigned int>(InputCommand::Right);
	}
}

void EntityUpdateByInput(Entity& entity, unsigned int command) {
	if (command & static_cast<unsigned int>(InputCommand::Up)) {
		entity.velocity.y = -2;
	}
	else if (command & static_cast<unsigned int>(InputCommand::Down)) {
		entity.velocity.y = 2;
	}

	if (command & static_cast<unsigned int>(InputCommand::Left)) {
		entity.velocity.x = -2;
	}
	else if (command & static_cast<unsigned int>(InputCommand::Right)) {
		entity.velocity.x = 2;
	}
	entity.position += entity.velocity;
}

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Input Test");

	SetTargetFPS(120);

	Field field;
	field.states.push_back({ {-300, 0} });
	field.states.push_back({ {300, 0} });

	// game loop
	while (!WindowShouldClose())
	{
		// clear section
		field.states[0].entity.velocity.x = 0;
		field.states[0].entity.velocity.y = 0;

		// Input Section
		UpdateCommands(field.states[0]);
		EntityUpdateByInput(field.states[0].entity, field.states[0].commands);

		// drawing
		BeginDrawing();

		ClearBackground(BLACK);

		DrawEntity(field.states[0].entity, RED);
		DrawEntity(field.states[1].entity, BLUE);

		EndDrawing();
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
