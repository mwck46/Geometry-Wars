#pragma once

#include "Common.h"
#include "EntityManager.h"
#include "Entity.h"
#include <cstdio>

// SR: shape radius
// CR: collision radius
// S: speed
// FR, FG, FB: fill color
// OR, OG, OB: outline color
// OT: outline thickness
// V: vertices
struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
// SMIN: min speed
// SMAX: max speed
// VMIN: min vertice
// VMAX: max vertice
// L: lift span
// SI: spawn internal
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
private:
	sf::RenderWindow m_window;
	EntityManager	m_entities;  
	sf::Font m_font; 
	sf::Text m_scoreText;
	PlayerConfig	m_playerConfig; // the player's properties
	EnemyConfig m_enemyConfig; // the enemy's properties
	BulletConfig m_bulletConfig; // the bullet's properties
	int m_score{ 0 };
	int m_currentFrame{ 0 };
	int m_lastEnemySpawnTime{ 0 };
	bool m_paused{ false };  // If game logic is updated
	bool m_running{ true };  // If the game is running

	sf::Texture m_backgroundTexture;
	sf::Sprite m_backgroundSprite;

	std::shared_ptr<Entity> m_player;

	void init(const std::string& config); //initialize the gamestate with a config file path
	void setPaused(bool paused);  //Pause the game

	void sMovement(); //Systems: entity movement
	void sUserInput();    //Systems: User Input
	void sLifeSpan();    //Systems: lifespan of the entities
	void sRender();     //Systems: Render
	void sEnemySpawner();   //Systems: Spawns enemies
	void sCollision();    //Systems: Collision detection

	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);

	void hReadConfig(std::string strConfigPath);  // Helper: read config file
public:

	Game(const std::string& configFile);
	void run();
};

