#include <raylib.h>
#include <math.h>
#include <string.h>

// Defines
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define TARGET_FPS 60

#define SAFE_ZONE_SIZE 250

#define PLAYER_SPEED 2.5
#define PLAYER_X_SIZE 20
#define PLAYER_Y_SIZE 20
#define PLAYER_COLOR DARKGREEN

#define MAX_BULLETS 4
#define BULLET_SPEED 18
#define BULLET_LENGTH 20
#define BULLET_COLOR WHITE

#define BULLET_DELAY 5

#define MAX_ENEMIES 128

#define GRUNT_SIZE 20
#define GRUNT_SPEED 1.25
#define GRUNT_COLOR RED
#define GRUNT_REWARD 100

#define SPHEROID_SIZE 20
#define SPHEROID_SPEED 3.5
#define SPHEROID_COLOR VIOLET
#define SPHEROID_REWARD 1000
#define SPHEROID_MOVE_COOLDOWN 150
#define ENFORCER_SPAWN_DELAY 240
#define ENFORCER_SPAWN_COOLDOWN 60

#define ENFORCER_SIZE 20
#define ENFORCER_SPEED 1.25
#define ENFORCER_COLOR BLUE
#define ENFORCER_REWARD 150
#define ENFORCER_MOVE_COOLDOWN 60
#define ENFORCER_SHOT_COOLDOWN 90

#define SPARK_SIZE 10
#define SPARK_SPEED 3.5
#define SPARK_REWARD 25
#define SPARK_COLOR YELLOW
#define SPARK_LIFETIME 120

#define HULK_X_SIZE 20
#define HULK_Y_SIZE 30
#define HULK_SPEED 0.25
#define HULK_COLOR LIME

#define ELECTRODE_SIZE 20
#define ELECTRODE_COLOR ORANGE

#define GAME_OVER_TEXT "PRESS ENTER TO PLAY AGAIN"
#define GAME_OVER_TEXT_SIZE 30

#define PAUSE_TEXT "Game paused"
#define PAUSE_TEXT_SIZE 30

#define VICTORY_TEXT_1 "Congratulations! You have defeated the robot horde!"
#define VICTORY_TEXT_2 "Press enter to play a victory lap"
#define VICTORY_TEXT_SIZE 25

#define MAX_WAVES 20

// Enums and structs
enum EnemyKind {
	GRUNT,
	SPHEROID,
	ENFORCER,
	SPARK,
	HULK,
	ELECTRODE
};

struct Player {
	Vector2 position;
	float speed;
	Vector2 velocity;
	Color color;
	Vector2 size;
};

struct Bullet {
	Vector2 startPos;
	Vector2 endPos;
	Vector2 velocity;
};

struct Enemy {
	Vector2 position;
	float speed;
	Vector2 velocity;
	Color color;
	Vector2 size;
	int counter;
	int reward;
	EnemyKind kind;
};

struct WaveData {
	int gruntCount;
	int spheroidCount;
	int hulkCount;
	int electrodeCount;
};


// Global variables
const float diagonalFactor = sqrtf(2) / 2;

const float offsetY = SCREEN_HEIGHT / 15;
const Vector2 screenCenter = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
const Vector2 playFieldCenter = { SCREEN_WIDTH / 2, offsetY + (SCREEN_HEIGHT - offsetY) / 2 };

const float safeZoneLeft = playFieldCenter.x - (SAFE_ZONE_SIZE / 2);
const float safeZoneRight = playFieldCenter.x + (SAFE_ZONE_SIZE / 2);
const float safeZoneTop = playFieldCenter.y - (SAFE_ZONE_SIZE / 2);
const float safeZoneBottom = playFieldCenter.y + (SAFE_ZONE_SIZE / 2);

int bulletCount;
int enemyCount;

Vector2 bulletVelocity; // Velocity of next bullet

int bulletCooldown;

bool gameOver;
bool paused;
bool victory;

int wave;

int frameCount;

int score;
int hiScore;

Player player;

Bullet bullets[MAX_BULLETS];

Enemy enemies[MAX_ENEMIES];

// Enemy counts for each wave
const WaveData waves[MAX_WAVES] = // {grunts, spheroids, hulks, electrodes}
{
	{15, 0, 0, 10}, // Wave 1
	{10, 1, 5, 10}, // 2
	{10, 2, 5, 10}, // 3
	{10, 3, 5, 10}, // 4
	{10, 5, 5, 10}, // 5
	{25, 0, 0, 10}, // 6
	{12, 4, 12, 12}, // 7
	{12, 5, 6, 12}, // 8
	{12, 5, 12, 12}, // 9
	{12, 8, 6, 12}, // 10
	{35, 0, 0, 10}, // 11
	{14, 6, 12, 14}, // 12
	{14, 6, 16, 14}, // 13
	{14, 7, 12, 14}, // 14
	{14, 10, 8, 14}, // 15
	{45, 0, 0, 10}, // 16
	{16, 7, 13, 16}, // 17
	{16, 5, 18, 16}, // 18
	{16, 8, 13, 16}, // 19
	{16, 12, 10, 16} // 20
};

// Function declarations
void init_game();
void init_wave();

void handle_input();
void update_game();
void draw_game();

void create_bullet();
void create_grunts(int count);
void create_spheroids(int count);
void create_enforcer(Vector2 pos);
void create_spark(Vector2 pos);
void create_hulks(int count);
void create_electrodes(int count);

void move_enemies();

void destroy_bullet(int i);
void destroy_enemy(int i);

Vector2 generate_enemy_pos(const Enemy &en);
Vector2 get_vector_to_player(Vector2 pos);

bool wave_is_clear();


int main()
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Robotron");
	
	init_game();
	
	SetTargetFPS(TARGET_FPS);
	
	while (!WindowShouldClose())
	{
		handle_input();
		
		update_game();
		
		draw_game();
	}
	
	CloseWindow();
	
	return 0;
}

void init_game()
{
	gameOver = false;
	paused = false;
	victory = false;
	wave = 0;
	frameCount = 0;
	score = 0;
	
	init_wave();
}

void init_wave()
{
	bulletCount = 0;
	enemyCount = 0;
	
	bulletCooldown = 0;
	
	// Initialize player components
	player.speed = PLAYER_SPEED;
	player.velocity = (Vector2){ 0, 0 };
	player.color = PLAYER_COLOR;
	player.size = (Vector2){ PLAYER_X_SIZE, PLAYER_Y_SIZE };
	player.position.x = playFieldCenter.x - player.size.x / 2;
	player.position.y = playFieldCenter.y - player.size.y / 2;
	
	bulletVelocity = (Vector2) {0, 0};
	
	// Initialize bullet components
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		bullets[i].startPos = (Vector2) {0, 0};
		bullets[i].endPos = (Vector2) {0, 0};
	}
	
	// Initialize enemy components
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		enemies[i].position = (Vector2) {0, 0};
		enemies[i].speed = 0;
		enemies[i].velocity = (Vector2) {0, 0};
		enemies[i].size = (Vector2) {0, 0};
		enemies[i].color = BLACK;
		enemies[i].counter = 0;
		enemies[i].reward = 0;
		enemies[i].kind = GRUNT;
	}
	
	if (wave == MAX_WAVES)
	{
		victory = true;
		return;
	}
	
	// Create enemies for this wave
	create_grunts(waves[wave].gruntCount);
	create_spheroids(waves[wave].spheroidCount);
	create_hulks(waves[wave].hulkCount);
	create_electrodes(waves[wave].electrodeCount);
}

void handle_input()
{
	if (gameOver)
	{
		if (IsKeyPressed(KEY_ENTER)) init_game();
		else return;
	}
	
	if (victory)
	{
		if (IsKeyPressed(KEY_ENTER))
		{
			victory = false;
			wave = 0;
			init_wave();
		}
		else return;
	}
	
	if (IsKeyPressed(KEY_P)) paused = !paused;
	
	if (paused) return;
	
	player.velocity = (Vector2){ 0, 0 };
	
	if (IsKeyDown(KEY_D)) player.velocity.x = player.speed;
	if (IsKeyDown(KEY_A)) player.velocity.x = -player.speed;
	if (IsKeyDown(KEY_W)) player.velocity.y = -player.speed;
	if (IsKeyDown(KEY_S)) player.velocity.y = player.speed;
	
	// Scale diagonal movement of player down
	if (player.velocity.x && player.velocity.y)
	{
		player.velocity.x *= diagonalFactor;
		player.velocity.y *= diagonalFactor;
	}
	
	bulletVelocity = { 0, 0 };
	
	if (IsKeyDown(KEY_RIGHT)) bulletVelocity.x = 1;
	if (IsKeyDown(KEY_LEFT)) bulletVelocity.x = -1;
	if (IsKeyDown(KEY_UP)) bulletVelocity.y = -1;
	if (IsKeyDown(KEY_DOWN)) bulletVelocity.y = 1;
}

void update_game()
{
	if (gameOver || paused || victory) return;
	
	// If wave is clear, move to next wave
	if (wave_is_clear())
	{
		wave++;
		init_wave();
		return;
	}
	
	// Update high score
	if (score > hiScore) hiScore = score;
	
	// Update player position
	player.position.x += player.velocity.x;
	player.position.y += player.velocity.y;
	
	// Update enemy positions
	move_enemies();
	
	// Update bullet positions
	for (int i = 0; i < bulletCount; i++)
	{
		bullets[i].startPos.x += bullets[i].velocity.x;
		bullets[i].startPos.y += bullets[i].velocity.y;
		bullets[i].endPos.x += bullets[i].velocity.x;
		bullets[i].endPos.y += bullets[i].velocity.y;
	}
	
	// Decrement bullet cooldown by one each frame
	if (bulletCooldown > 0) bulletCooldown--;
	
	// Create bullet if a fire button is pressed and cooldown has elapsed
	if (bulletCooldown == 0 && (bulletVelocity.x || bulletVelocity.y)) create_bullet();
	
	// Player/Wall collision
	if (player.position.x < 0) player.position.x = 0;
	if (player.position.x > SCREEN_WIDTH - player.size.x) player.position.x = SCREEN_WIDTH - player.size.x;
	if (player.position.y < offsetY + 1) player.position.y = offsetY + 1;
	if (player.position.y > SCREEN_HEIGHT - player.size.y) player.position.y = SCREEN_HEIGHT - player.size.y;
	
	
	// Enemy/Wall Collision
	for (int i = 0; i < enemyCount; i++)
	{
		if (enemies[i].position.x < 0) enemies[i].position.x = 0;
		if (enemies[i].position.x > SCREEN_WIDTH - enemies[i].size.x) enemies[i].position.x = SCREEN_WIDTH - enemies[i].size.x;
		if (enemies[i].position.y < offsetY + 1) enemies[i].position.y = offsetY + 1;
		if (enemies[i].position.y > SCREEN_HEIGHT - enemies[i].size.y) enemies[i].position.y = SCREEN_HEIGHT - enemies[i].size.y;
	}
	
	// Player/Enemy Collision
	for (int i = 0; i < enemyCount; i++)
	{
		if (CheckCollisionRecs((Rectangle){ player.position.x, player.position.y, player.size.x, player.size.y},
			(Rectangle){ enemies[i].position.x, enemies[i].position.y, enemies[i].size.x, enemies[i].size.y }))
			{
				  gameOver = true; 
			}
	}
	
	// Bullet/Wall collision
	for (int i = 0; i < bulletCount; i++)
	{
		bool hitsRightWall = bullets[i].endPos.x > SCREEN_WIDTH;
		bool hitsLeftWall = bullets[i].endPos.x < 0;
		bool hitsTopWall = bullets[i].endPos.y < offsetY;
		bool hitsBottomWall = bullets[i].endPos.y > SCREEN_HEIGHT;
		
		
		if (hitsRightWall || hitsLeftWall || hitsTopWall || hitsBottomWall)
		{
			destroy_bullet(i);
		}
	}
	
	// Bullet/Enemy collision
	for (int i = 0; i < enemyCount; i++)
	{
		for (int j = 0; j < bulletCount; j++)
		{
			Vector2 p = {bullets[j].endPos.x, bullets[j].endPos.y};
			Rectangle rec = {enemies[i].position.x, enemies[i].position.y, enemies[i].size.x, enemies[i].size.y};
			if (CheckCollisionPointRec(p, rec))
			{
				if (enemies[i].kind != HULK) 
				{
					score += enemies[i].reward;
					destroy_enemy(i);
				}
				destroy_bullet(j);
				
				// Because another entity moves into place of the destroyed one, we check this index again
				i--;
				break;
			}
		}
	}
	
	frameCount++;
}

void draw_game()
{
	BeginDrawing();
	
		ClearBackground(BLACK);
		
		//DrawFPS(0, 0);
	
		if (gameOver)
		{
			// Draw game over text in center of screen
			DrawText(GAME_OVER_TEXT, screenCenter.x - MeasureText(GAME_OVER_TEXT, GAME_OVER_TEXT_SIZE) / 2, screenCenter.y - GAME_OVER_TEXT_SIZE / 2,
				GAME_OVER_TEXT_SIZE, RED);
			EndDrawing();
			return;
		}
		
		if (victory)
		{
			DrawText(VICTORY_TEXT_1, screenCenter.x - MeasureText(VICTORY_TEXT_1, VICTORY_TEXT_SIZE) / 2, screenCenter.y - VICTORY_TEXT_SIZE / 2,
				VICTORY_TEXT_SIZE, GOLD);
			DrawText(VICTORY_TEXT_2, screenCenter.x - MeasureText(VICTORY_TEXT_2, VICTORY_TEXT_SIZE) / 2, screenCenter.y + VICTORY_TEXT_SIZE / 2,
				VICTORY_TEXT_SIZE, GOLD);
			EndDrawing();
			return;
		}
	
		// Draw top wall of play area
		DrawLine(0, offsetY, SCREEN_WIDTH, offsetY, ORANGE);
		
		// Draw score
		DrawText(TextFormat("Score: %d", score), 50, 0, 30, WHITE);
		
		// Draw high score
		DrawText(TextFormat("Best: %d", hiScore), 600, 0, 30, WHITE);
		
		// Draw wave count
		DrawText(TextFormat("Wave %d", wave + 1), 350, 0, 30, WHITE);
	
		// Draw Player
		DrawRectangleV(player.position, player.size, player.color);
		
		//Draw bullets
		for (int i = 0; i < bulletCount; i++)
		{
			DrawLineV(bullets[i].startPos, bullets[i].endPos, (Color){(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255),
				(unsigned char)GetRandomValue(0, 255), 255});
		}
	
		// Draw Enemies
		for (int i = 0; i < enemyCount; i++)
		{
			DrawRectangleV(enemies[i].position, enemies[i].size, enemies[i].color);
		}
	
		// TEMPORARY!!
		// Draw safe zone
		//DrawRectangleLines(playFieldCenter.x - SAFE_ZONE_SIZE / 2, playFieldCenter.y - SAFE_ZONE_SIZE / 2, SAFE_ZONE_SIZE, SAFE_ZONE_SIZE, ORANGE);
	
		if (paused)
		{
			// Draw pause text in center of screen
			DrawText(PAUSE_TEXT, screenCenter.x - MeasureText(PAUSE_TEXT, PAUSE_TEXT_SIZE) / 2, screenCenter.y - PAUSE_TEXT_SIZE, PAUSE_TEXT_SIZE, WHITE);
		}
	
	EndDrawing();
}

void create_bullet()
{
	// If bullets array is full, don't create more
	if (bulletCount >= MAX_BULLETS) return;
	
	Vector2 playerCenter = { player.position.x + (player.size.x / 2), player.position.y + (player.size.y / 2) };
	
	bullets[bulletCount].startPos.x = playerCenter.x + (player.size.x / 2) * bulletVelocity.x;
	bullets[bulletCount].startPos.y = playerCenter.y + (player.size.y / 2) * bulletVelocity.y;
	
	// Scale diagonal movement down
	if (bulletVelocity.x && bulletVelocity.y)
	{
		bulletVelocity.x *= diagonalFactor;
		bulletVelocity.y *= diagonalFactor;
	}
	
	bullets[bulletCount].endPos.x = bullets[bulletCount].startPos.x + bulletVelocity.x * BULLET_LENGTH;
	bullets[bulletCount].endPos.y = bullets[bulletCount].startPos.y + bulletVelocity.y * BULLET_LENGTH;
	
	bullets[bulletCount].velocity.x = bulletVelocity.x * BULLET_SPEED;
	bullets[bulletCount].velocity.y = bulletVelocity.y * BULLET_SPEED;
	
	bulletCount++;
	
	bulletCooldown = BULLET_DELAY;
}

void create_grunts(int count)
{
	for (int i = 0; i < count; i++)
	{
		if (enemyCount >= MAX_ENEMIES) return;
		
		enemies[enemyCount].speed = GRUNT_SPEED;
		enemies[enemyCount].color = GRUNT_COLOR;
		enemies[enemyCount].size = (Vector2){ GRUNT_SIZE, GRUNT_SIZE };
		enemies[enemyCount].reward = GRUNT_REWARD;
		enemies[enemyCount].kind = GRUNT;
		
		enemies[enemyCount].position = generate_enemy_pos(enemies[enemyCount]);
		
		enemyCount++;
	}
}

void create_spheroids(int count)
{
	for (int i = 0; i < count; i++)
	{
		if (enemyCount >= MAX_ENEMIES) return;
		
		enemies[enemyCount].speed = SPHEROID_SPEED;
		enemies[enemyCount].color = SPHEROID_COLOR;
		enemies[enemyCount].size = (Vector2) { SPHEROID_SIZE, SPHEROID_SIZE };
		enemies[enemyCount].counter = GetRandomValue(0, 30);
		enemies[enemyCount].reward = SPHEROID_REWARD;
		enemies[enemyCount].kind = SPHEROID;
		
		// It spawns on a random edge of the screen
		int side = GetRandomValue(0, 3);
		int direction = 1 - GetRandomValue(0, 1) * 2;
		
		switch (side)
		{
			case 0: // Top
				enemies[enemyCount].position.y = offsetY + 1;
				enemies[enemyCount].position.x = (float)GetRandomValue(0, SCREEN_WIDTH - SPHEROID_SIZE);
				enemies[enemyCount].velocity.x = SPHEROID_SPEED * direction;
				break;
			case 1: // Right
				enemies[enemyCount].position.x = SCREEN_WIDTH - SPHEROID_SIZE;
				enemies[enemyCount].position.y = (float)GetRandomValue(offsetY + 1, SCREEN_HEIGHT - SPHEROID_SIZE);
				enemies[enemyCount].velocity.y = SPHEROID_SPEED * direction;
				break;
			case 2: // Bottom
				enemies[enemyCount].position.y = SCREEN_HEIGHT - SPHEROID_SIZE;
				enemies[enemyCount].position.x = (float)GetRandomValue(0, SCREEN_WIDTH - SPHEROID_SIZE);
				enemies[enemyCount].velocity.x = SPHEROID_SPEED * direction;
				break;
			case 3: // Left
				enemies[enemyCount].position.x = 0;
				enemies[enemyCount].position.y = (float)GetRandomValue(offsetY + 1, SCREEN_HEIGHT - SPHEROID_SIZE);
				enemies[enemyCount].velocity.y = SPHEROID_SPEED * direction;
				break;
		}
		
		enemyCount++;
	}
}

void create_enforcer(Vector2 pos)
{
		if (enemyCount >= MAX_ENEMIES) return;
		
		enemies[enemyCount].speed = ENFORCER_SPEED;
		enemies[enemyCount].color = ENFORCER_COLOR;
		enemies[enemyCount].size = (Vector2){ ENFORCER_SIZE, ENFORCER_SIZE };
		enemies[enemyCount].kind = ENFORCER;
		enemies[enemyCount].counter = GetRandomValue(0, 30);
		enemies[enemyCount].reward = ENFORCER_REWARD;
		
		enemies[enemyCount].position = pos;
		
		Vector2 vec = get_vector_to_player(enemies[enemyCount].position);
		enemies[enemyCount].velocity.x = vec.x * ENFORCER_SPEED;
		enemies[enemyCount].velocity.y = vec.y * ENFORCER_SPEED;
		
		enemyCount++;
}

void create_spark(Vector2 pos)
{
	if (enemyCount >= MAX_ENEMIES) return;
	
	enemies[enemyCount].speed = SPARK_SPEED;
	enemies[enemyCount].color = SPARK_COLOR;
	enemies[enemyCount].size = (Vector2){ SPARK_SIZE, SPARK_SIZE };
	enemies[enemyCount].reward = SPARK_REWARD;
	enemies[enemyCount].kind = SPARK;

	enemies[enemyCount].position = pos;
	
	Vector2 vec = get_vector_to_player(pos);
	enemies[enemyCount].velocity.x = vec.x * SPARK_SPEED;
	enemies[enemyCount].velocity.y = vec.y * SPARK_SPEED;
	
	enemyCount++;
}

void create_hulks(int count)
{
	for (int i = 0; i < count; i++)
	{
		if (enemyCount >= MAX_ENEMIES) return;
		
		enemies[enemyCount].speed = HULK_SPEED;
		enemies[enemyCount].color = HULK_COLOR;
		enemies[enemyCount].size = (Vector2){ HULK_X_SIZE, HULK_Y_SIZE };
		enemies[enemyCount].kind = HULK;
		
		enemies[enemyCount].position = generate_enemy_pos(enemies[enemyCount]);
		
		enemyCount++;
	}
}

void create_electrodes(int count)
{
	for (int i = 0; i < count; i++)
	{
		if (enemyCount >= MAX_ENEMIES) return;
		
		enemies[enemyCount].color = ELECTRODE_COLOR;
		enemies[enemyCount].size = (Vector2){ ELECTRODE_SIZE, ELECTRODE_SIZE };
		enemies[enemyCount].kind = ELECTRODE;
		enemies[enemyCount].reward = 0;
		
		enemies[enemyCount].position = generate_enemy_pos(enemies[enemyCount]);
		
		enemyCount++;
	}
}

void move_enemies()
{
	for (int i = 0; i < enemyCount; i++)
	{
		Vector2 vec = { 0 };
		
		switch (enemies[i].kind) {
			case GRUNT:
			    vec = get_vector_to_player(enemies[i].position);
				enemies[i].velocity.x = vec.x * enemies[i].speed;
				enemies[i].velocity.y = vec.y * enemies[i].speed;
				break;
				
			case SPHEROID:
				enemies[i].counter++;
				if (enemies[i].counter % SPHEROID_MOVE_COOLDOWN == 0)
				{
					enemies[i].velocity = {0, 0};
					if (GetRandomValue(0, 1))
					{
						enemies[i].velocity.x = enemies[i].speed * (1 - GetRandomValue(0, 1) * 2);
					}
					else
					{
						enemies[i].velocity.y = enemies[i].speed * (1 - GetRandomValue(0, 1) * 2);
					}
				}
				if (enemies[i].counter >= ENFORCER_SPAWN_DELAY && enemies[i].counter % ENFORCER_SPAWN_COOLDOWN == 0)
				{
					create_enforcer(enemies[i].position);
				}
				if (enemies[i].counter == (ENFORCER_SPAWN_DELAY + ENFORCER_SPAWN_COOLDOWN * 2))
				{
					destroy_enemy(i);
					i--;
					continue;
				}
				break;
				
			case ENFORCER:
				if (enemies[i].counter % ENFORCER_MOVE_COOLDOWN == 0)
				{
					vec = get_vector_to_player(enemies[i].position);
					enemies[i].velocity.x = vec.x * enemies[i].speed;
					enemies[i].velocity.y = vec.y * enemies[i].speed;
				}
				if (enemies[i].counter % ENFORCER_SHOT_COOLDOWN == 0) create_spark(enemies[i].position);
				enemies[i].counter++;
				break;
				
			case SPARK:
				enemies[i].counter++;
				if (enemies[i].counter == SPARK_LIFETIME)
				{
					destroy_enemy(i);
					i--;
					continue;
				}
				break;
				
			case HULK:
				if (frameCount % 60 == 0)
				{
					enemies[i].velocity.x = enemies[i].speed * (1 - GetRandomValue(0, 1) * 2);
					enemies[i].velocity.y = enemies[i].speed * (1 - GetRandomValue(0, 1) * 2);
				}
				break;
		}
		
		// Update enemy position
		enemies[i].position.x += enemies[i].velocity.x;
		enemies[i].position.y += enemies[i].velocity.y;
	}
}

void destroy_bullet(int i)
{
	bulletCount--;
	
	if (i != bulletCount)
	{
		// Move last bullet to position of bullet to be destroyed
		memcpy(&bullets[i], &bullets[bulletCount], sizeof(Bullet));
	}
	
	// Initialize last bullet to 0
	memset(&bullets[bulletCount], 0, sizeof(Bullet));
}

void destroy_enemy(int i)
{
	enemyCount--;
	
	if (i != enemyCount)
	{
		// Move last enemy to position of enemy to be destroyed
		memcpy(&enemies[i], &enemies[enemyCount], sizeof(Enemy));
	}
	
	// Initialize last enemy to 0
	memset(&enemies[enemyCount], 0, sizeof(Enemy));
}

Vector2 generate_enemy_pos(const Enemy &en)
{
	float enemyXPos;
	float enemyYPos;
		
	bool rightOfSafeZoneLeft;
	bool leftOfSafeZoneRight;
	bool belowSafeZoneTop;
	bool aboveSafeZoneBottom;
		
	// Generate enemy position while ensuring it's not in the safe zone
	do {
		enemyXPos = (float)GetRandomValue(0, SCREEN_WIDTH - en.size.x);
		enemyYPos = (float)GetRandomValue(offsetY, SCREEN_HEIGHT - en.size.y);
			
		rightOfSafeZoneLeft = safeZoneLeft < enemyXPos + en.size.x;
		leftOfSafeZoneRight = enemyXPos < safeZoneRight;
		belowSafeZoneTop = safeZoneTop < enemyYPos + en.size.y;
		aboveSafeZoneBottom = enemyYPos < safeZoneBottom;
	} while (rightOfSafeZoneLeft && leftOfSafeZoneRight && belowSafeZoneTop && aboveSafeZoneBottom);
	
	return (Vector2) { enemyXPos, enemyYPos };
}

Vector2 get_vector_to_player(Vector2 pos)
{
	// Calculate distance to player
	Vector2 playerDist = { player.position.x - pos.x, player.position.y - pos.y};
		
	// Get magnitude of vector
	float magnitude = sqrtf(playerDist.x * playerDist.x + playerDist.y * playerDist.y);
		
	// Normalize vector
	Vector2 vel = { 0 };
	
	if (magnitude > 0)
	{
		vel = { playerDist.x / magnitude, playerDist.y / magnitude };
	}
	
	return vel;
}

bool wave_is_clear()
{
	for (int i = 0; i < enemyCount; i++)
	{
		if (enemies[i].kind != SPARK && enemies[i].kind != HULK && enemies[i].kind != ELECTRODE)
			return false;
	}
	
	return true;
}
