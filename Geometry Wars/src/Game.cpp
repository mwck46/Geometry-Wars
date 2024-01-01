#include "Game.h"

#include "Common.h"

Game::Game(const std::string& configFile)
{
	init(configFile);
}

void Game::init(const std::string& config)
{
	//Read in config file here
	//	use the premade PlayerConfig,Enemy Config, BulletConfig variables 

	//Makes sure to change the rand seed so it doesnt spawn the same as the last gamess
	srand(time(nullptr));

	m_backgroundTexture.loadFromFile("art/ezgif-2-76dec4f055.jpg");
	m_backgroundSprite.setTexture(m_backgroundTexture);
	m_backgroundSprite.setPosition(sf::Vector2f(0, 0));

	hReadConfig(config);

	m_scoreText.setPosition(0, 0);
	m_scoreText.setString(std::to_string(m_score));

	spawnPlayer();
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

void Game::sMovement()
{
	Vec2 playerVelocity;
	if (m_player->cInput->left)
	{
		playerVelocity.x -= m_playerConfig.S;
	}
	if (m_player->cInput->right)
	{
		playerVelocity.x += m_playerConfig.S;
	}
	if (m_player->cInput->up)
	{
		playerVelocity.y -= m_playerConfig.S;
	}
	if (m_player->cInput->down)
	{
		playerVelocity.y += m_playerConfig.S;
	}

	if (m_player->cInput->shoot == true)
	{
		Vec2 mousePos{ static_cast<double>(sf::Mouse::getPosition(m_window).x), static_cast<double>(sf::Mouse::getPosition(m_window).y) };
		spawnBullet(m_player, mousePos);
		m_player->cInput->shoot = false;
	}

	for (auto e : m_entities.getEntities())
	{
		if (e->tag() == entityTags::player)
		{
			//Updates the position of the player
			m_player->cTransform->pos += playerVelocity;
			//Rotates the player
			e->cTransform->angle += 1.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
		}
		if (e->cTransform)
		{
			//Updates the position of the entities
			e->cTransform->pos += e->cTransform->velocity;
			//Rotates the player
			e->cTransform->angle += 1.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
		}
	}
}

void Game::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		switch (event.type)
		{
			case sf::Event::Closed:
			{
				m_running = false;
				break;
			}
			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					m_running = false;
				}
				else if (event.key.code == sf::Keyboard::P)
				{
					m_paused ? setPaused(false) : setPaused(true);
				}
				//Player input
				else if (event.key.code == sf::Keyboard::A) { m_player->cInput->left = true; }
				else if (event.key.code == sf::Keyboard::D) { m_player->cInput->right = true; }
				else if (event.key.code == sf::Keyboard::W) { m_player->cInput->up = true; }
				else if (event.key.code == sf::Keyboard::S) { m_player->cInput->down = true; }

				break;
			}
			case sf::Event::KeyReleased:
			{
				if (event.key.code == sf::Keyboard::A) { m_player->cInput->left = false; }
				else if (event.key.code == sf::Keyboard::D) { m_player->cInput->right = false; }
				else if (event.key.code == sf::Keyboard::W) { m_player->cInput->up = false; }
				else if (event.key.code == sf::Keyboard::S) { m_player->cInput->down = false; }

				break;
			}
			case sf::Event::MouseButtonPressed:
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (m_player->cInput->shoot == false)
						m_player->cInput->shoot = true;
				}
				//DEBUG
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					Vec2 mousePos = Vec2(sf::Mouse::getPosition(m_window).x,sf::Mouse::getPosition(m_window).y);
					
					for (auto e : m_entities.getEntities(entityTags::enemy))
					{
						//std::cout << "Position is: " << e->cTransform->pos.x << " " << e->cTransform->pos.y << '\n';

						Vec2 diff{ e->cTransform->pos.x - mousePos.x , e->cTransform->pos.y - mousePos.y };
						
						double collisionRadiusSQ{ (10 + e->cCollision->radius) * (10 + e->cCollision->radius) };
						double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

						if (distSQ < collisionRadiusSQ)
						{
							std::cout << "Position is: " << e->cTransform->pos.x << " " << e->cTransform->pos.y << '\n';
						}
					}
				}
				break;
			}
			case sf::Event::MouseButtonReleased:
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					m_player->cInput->shoot = false;

				}
				break;
			}

			default:
				break;
		}
	}
}

void Game::sLifeSpan()
{
	for (auto e : m_entities.getEntities())
	{
		if (!e->cLifespan) { continue; }

		if (e->cLifespan->remaining > 0)
		{
			--e->cLifespan->remaining;
		}
		if (e->isActive() && e->cLifespan->remaining > 0)
		{
			float alphaMultiplier{ static_cast<float>(e->cLifespan->remaining) / static_cast<float>(e->cLifespan->total) };

			auto fillColor{ e->cShape->circle.getFillColor() };
			sf::Color newFillColor{ fillColor.r,fillColor.g,fillColor.b, static_cast<sf::Uint8>( 255 * alphaMultiplier) };
			e->cShape->circle.setFillColor(newFillColor);

			auto outlineColor{ e->cShape->circle.getOutlineColor() };
			sf::Color newOutlineColor{ outlineColor.r,outlineColor.g,outlineColor.b, static_cast<sf::Uint8>(255 * alphaMultiplier) };
			e->cShape->circle.setOutlineColor(newOutlineColor);
		}
		else if (e->cLifespan->remaining <= 0)
		{
			e->destroy();
		}
	}
}

void Game::sRender()
{
	m_window.clear();

	m_window.draw(m_backgroundSprite);

	for (auto e : m_entities.getEntities())
	{
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		m_window.draw(e->cShape->circle);
	}
	m_window.draw(m_scoreText);
	
	m_window.display();
}

void Game::sEnemySpawner()
{
	if ((m_currentFrame - m_lastEnemySpawnTime) >= m_enemyConfig.SI)
	{
		spawnEnemy();
		//std::cout << "Enemy was spawned\n";
		//std::cout << "Current frame: " << m_currentFrame << '\n';
	}
}

void Game::sCollision()
{
	for (auto player : m_entities.getEntities(entityTags::player))
	{
		for (auto enemy : m_entities.getEntities(entityTags::enemy))
		{
			//if there's a collision between player and enemy
			//Destroy the player,//Destroy the enemy,//Respawn the player

			Vec2 diff{ enemy->cTransform->pos.x - player->cTransform->pos.x , enemy->cTransform->pos.y - player->cTransform->pos.y };

			double collisionRadiusSQ{ (player->cCollision->radius + enemy->cCollision->radius) * (player->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				//makes sure the player is alive and doesnt spawn 2 players
				if (player->isActive())
				{
					enemy->destroy();
					player->destroy();
					spawnPlayer();
				}
			}
		}

		for (auto enemy : m_entities.getEntities(entityTags::smallEnemy))
		{
			//if there's a collision between player and enemy
			//Destroy the player,//Destroy the enemy,//Respawn the player
		
			Vec2 diff{ enemy->cTransform->pos.x - player->cTransform->pos.x , enemy->cTransform->pos.y - player->cTransform->pos.y };
		
			double collisionRadiusSQ{ (player->cCollision->radius + enemy->cCollision->radius) * (player->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };
		
			if (distSQ < collisionRadiusSQ)
			{
				//makes sure the player is alive so it doesnt spawn 2 players
				if (player->isActive())
				{
					player->destroy();
					enemy->destroy();
					spawnPlayer();
				}
			}
		}
	}

	for (auto bullet : m_entities.getEntities(entityTags::bullet))
	{
		for (auto enemy : m_entities.getEntities(entityTags::enemy))
		{
			//if there's a collision between player and enemy
			//Destroy the player,//Destroy the enemy,//Respawn the player

			Vec2 diff{ enemy->cTransform->pos.x - bullet->cTransform->pos.x , enemy->cTransform->pos.y - bullet->cTransform->pos.y };

			double collisionRadiusSQ{ (bullet->cCollision->radius + enemy->cCollision->radius) * (bullet->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				//Updates the score
				m_score += enemy->cScore->score;
				m_scoreText.setString(std::to_string(m_score));

				spawnSmallEnemies(enemy);
				bullet->destroy();
				enemy->destroy();

				break;
			}
		}

		for (auto enemy : m_entities.getEntities(entityTags::smallEnemy))
		{
			//Updates the score

			Vec2 diff{ enemy->cTransform->pos.x - bullet->cTransform->pos.x , enemy->cTransform->pos.y - bullet->cTransform->pos.y };

			double collisionRadiusSQ{ (bullet->cCollision->radius + enemy->cCollision->radius) * (bullet->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				m_score += enemy->cScore->score;
				m_scoreText.setString(std::to_string(m_score));

				bullet->destroy();
				enemy->destroy();

				break;
			}
		}
	}

	//General Collision ie walls && ground && ceiling for player

	for (auto e : m_entities.getEntities(entityTags::player))
	{
		//Checks to see if player collided with walls
		if (e->cTransform->pos.x + m_playerConfig.CR > m_window.getSize().x)
		{
			e->cTransform->pos.x -= m_playerConfig.S;
		}
		else if (e->cTransform->pos.x - m_playerConfig.CR < 0)
		{
			e->cTransform->pos.x += m_playerConfig.S;
		}

		if (e->cTransform->pos.y + m_playerConfig.CR > m_window.getSize().y)
		{
			e->cTransform->pos.y -= m_playerConfig.S;
		}
		else if (e->cTransform->pos.y - m_playerConfig.CR < 0)
		{
			e->cTransform->pos.y += m_playerConfig.S;
		}
	}

	//General Collision ie walls && ground && ceiling for entities
	for (auto e : m_entities.getEntities())
	{
		if (e->tag() == entityTags::bullet || e->tag() == entityTags::enemy || e->tag() == entityTags::smallEnemy)
		{
			if (e->cTransform->pos.x + e->cCollision->radius > m_window.getSize().x)
			{
				e->cTransform->velocity.x *= -1;
			}
			else if (e->cTransform->pos.x - e->cCollision->radius < 0)
			{
				e->cTransform->velocity.x *= -1;
			}
			if (e->cTransform->pos.y + e->cCollision->radius > m_window.getSize().y)
			{
				e->cTransform->velocity.y *= -1;
			}
			else if (e->cTransform->pos.y - e->cCollision->radius < 0)
			{
				e->cTransform->velocity.y *= -1;
			}
		}
	}
}

void Game::spawnPlayer()
{
	//Create every entity by calling EntityManager.addEntity(tag) because Entity constructor is private
	//This returns a std::shared_ptr<Entity>
	auto playerEntity = m_entities.addEntity(entityTags::player);

	Vec2 middleWindow{ m_window.getSize().x * 0.5, m_window.getSize().y * 0.5 };
	
	playerEntity->cTransform = std::make_shared<CTransform>(middleWindow, Vec2(0, 0), 0.0f);

	//Creates the entities shape
	playerEntity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V,sf::Color(m_playerConfig.FR, m_playerConfig.FG,
			m_playerConfig.FB), sf::Color(m_playerConfig.OR,m_playerConfig.OG, m_playerConfig.OB),m_playerConfig.OT);

	//Give the entity an collision component
	playerEntity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	//Add an input component to take the player's input
	playerEntity->cInput = std::make_shared<CInput>();

	//This goes slightly against the EntityManagers paradigm, but we use the player so much, it's worth it
	m_player = playerEntity;
}

void Game::spawnEnemy()
{
	auto e = m_entities.addEntity(entityTags::enemy);
	
	int minPosX{ m_enemyConfig.CR };
	int maxPosX{ static_cast<int>(m_window.getSize().x - m_enemyConfig.CR) };
	
	int minPosY{ m_enemyConfig.CR };
	int maxPosY{ static_cast<int>(m_window.getSize().y - m_enemyConfig.CR) };
	
	Vec2 randPos
	{ 
		static_cast<double>(minPosX + (rand() % (maxPosX - minPosX + 1))),
		static_cast<double>(minPosY + (rand() % (maxPosY - minPosY + 1)))
	};
	
	//Randomize the speed of the enemy and checks if (0) true->redo
	int randSpeed
	{ 
		static_cast<int>(m_enemyConfig.SMIN + rand() % static_cast<int>((m_enemyConfig.SMAX - m_enemyConfig.SMIN + 1)))
	};
	
	//Makes sure the velocity is not 0
	while (randSpeed == 0)
	{
		randSpeed = m_enemyConfig.SMIN + rand() % static_cast<int>((m_enemyConfig.SMAX - m_enemyConfig.SMIN + 1));
	}
	
	//Gives the entity the random position on the screen 
	e->cTransform = std::make_shared<CTransform>(randPos, Vec2(randSpeed, randSpeed), 0.0f);
	
	//Give the enemy a score of 100
	e->cScore = std::make_shared<CScore>(100);
	
	int vertices{ m_enemyConfig.VMIN + (rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN + 1)) };
	
	sf::Color fillColor
	{
		static_cast<sf::Uint8>(rand() % (1 + 255)),
		static_cast<sf::Uint8>(rand() % (1 + 255)),
		static_cast<sf::Uint8>(rand() % (1 + 255)) 
	};
	
	sf::Color outlineColor
	{ 
		static_cast<sf::Uint8>(m_enemyConfig.OR),
		static_cast<sf::Uint8>(m_enemyConfig.OG),
		static_cast<sf::Uint8>(m_enemyConfig.OB)
	};
	
	int outlineThickness{ m_enemyConfig.OT };
	
	e->cShape = std::make_shared<CShape>(m_enemyConfig.SR, vertices, fillColor,outlineColor, outlineThickness);
	
	e->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
	
	//Record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> parent)
{
	//	- spawn a number of small enemies equal to the number of vertices of the original
	//	- set each enemy to the same color as the original, half the size
	//	- small enemies are worth double points of the original enemy

	size_t vertices{ parent->cShape->circle.getPointCount() };

	Vec2 parentPos{ parent->cTransform->pos.x ,parent->cTransform->pos.y };

	sf::Color parentFill{ parent->cShape->circle.getFillColor() };
	sf::Color parentOutline;
	float parentTickness{ parent->cShape->circle.getOutlineThickness() };

	float smallEnemyRadius{ parent->cShape->circle.getRadius() * 0.5f };
	float smallEnemyCollisionRadius{ parent->cCollision->radius * 0.5f };

	for (size_t i{ 0 }; i < vertices; ++i)
	{
		auto e = m_entities.addEntity(entityTags::smallEnemy);
		e->cScore = std::make_shared<CScore>(parent->cScore->score * 2);
		e->cShape = std::make_shared<CShape>(smallEnemyRadius, vertices, parentFill, parentOutline, parentTickness);
		e->cCollision = std::make_shared<CCollision>(smallEnemyCollisionRadius);
		e->cLifespan = std::make_shared<CLifeSpan>(m_enemyConfig.L);

		//TODO: add the correct velocity for each small enemies
		e->cTransform = std::make_shared<CTransform>(parentPos, Vec2(0, 0), 0);

		float lenght{ sqrtf(parent->cShape->circle.getRadius() * parent->cShape->circle.getRadius() + parent->cShape->circle.getRadius() * parent->cShape->circle.getRadius()) };

		float cos{ parent->cShape->circle.getRadius() / lenght};

		float x{ lenght * cos };
		float y{ lenght * cos };


	}
}

//Spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> shooter, const Vec2& mousePos)
{
	auto bullet = m_entities.addEntity(entityTags::bullet);

	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

	bullet->cLifespan = std::make_shared<CLifeSpan>(m_bulletConfig.L);

	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG,
		m_bulletConfig.OB), m_bulletConfig.OT);


	Vec2 difference{ mousePos.x - shooter->cTransform->pos.x, mousePos.y - shooter->cTransform->pos.y };

	difference.normalize();

	Vec2 velocity{m_bulletConfig.S * difference.x, m_bulletConfig.S * difference.y};

	bullet->cTransform = std::make_shared<CTransform>(shooter->cTransform->pos, velocity, 0);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
}

void Game::hReadConfig(std::string config)
{
	//Read in the config file 
	std::fstream input{ config };

	if (!input.is_open())
	{
		std::cout << "Failed to open: " << config << '\n';
		exit(-1);
	}

	std::string identifier{};
	while (input >> identifier)
	{
		if (identifier == "Window")
		{
			sf::VideoMode vm;
			int frameLimit{}, fullScreen{};

			input >> vm.width;
			input >> vm.height;
			input >> frameLimit;
			input >> fullScreen;

			if (fullScreen == 0)
			{
				m_window.create(vm, "Geometry Wars", sf::Style::Close);
				m_window.setFramerateLimit(frameLimit);
			}
			else if (fullScreen == 1)
			{
				auto fullscreenMode{ sf::VideoMode::getFullscreenModes() };
				//fullscreenMode[0] is the most compatible mode for fullscreen on this device
				m_window.create(fullscreenMode[0], "Geometry Wars", sf::Style::Fullscreen);
				m_window.setFramerateLimit(frameLimit);
			}
		}
		else if (identifier == "Font")
		{
			std::string filepath{};
			input >> filepath;
			if (!m_font.loadFromFile(filepath))
			{
				std::cerr << "Failed to load font. Filepath: " << filepath;
			}
			m_scoreText.setFont(m_font);

			int fontSize{};
			input >> fontSize;
			m_scoreText.setCharacterSize(fontSize);

			sf::Color clr;
			input >> clr.r;
			input >> clr.g;
			input >> clr.b;
			m_scoreText.setFillColor(clr);
		}
		else if (identifier == "Player")
		{
			input >> m_playerConfig.SR;

			input >> m_playerConfig.CR;
			input >> m_playerConfig.S;

			input >> m_playerConfig.FR;
			input >> m_playerConfig.FG;
			input >> m_playerConfig.FB;

			input >> m_playerConfig.OR;
			input >> m_playerConfig.OG;
			input >> m_playerConfig.OB;
			input >> m_playerConfig.OT;

			input >> m_playerConfig.V;
		}
		else if (identifier == "Enemy")
		{
			input >> m_enemyConfig.SR;
			input >> m_enemyConfig.CR;

			input >> m_enemyConfig.SMIN;
			input >> m_enemyConfig.SMAX;

			input >> m_enemyConfig.OR;
			input >> m_enemyConfig.OG;
			input >> m_enemyConfig.OB;
			input >> m_enemyConfig.OT;

			input >> m_enemyConfig.VMIN;
			input >> m_enemyConfig.VMAX;

			input >> m_enemyConfig.L;

			input >> m_enemyConfig.SI;
		}
		else if (identifier == "Bullet")
		{
			input >> m_bulletConfig.SR;

			input >> m_bulletConfig.CR;
			input >> m_bulletConfig.S;

			input >> m_bulletConfig.FR;
			input >> m_bulletConfig.FG;
			input >> m_bulletConfig.FB;

			input >> m_bulletConfig.OR;
			input >> m_bulletConfig.OG;
			input >> m_bulletConfig.OB;
			input >> m_bulletConfig.OT;

			input >> m_bulletConfig.V;

			input >> m_bulletConfig.L;
		}
	}
}

void Game::run()
{
	while (m_running)
	{
		m_entities.update();

		if (!m_paused)
		{
			sLifeSpan();
			sEnemySpawner();
			sMovement();
			sCollision();
		}
		sUserInput();
		sRender();

		//Increment the current frame
		++m_currentFrame;
	}
}