#include "Platform/Platform.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Window.hpp>
#include <list>
#include <sstream>
#include <time.h>

using namespace std;
using namespace sf;

const int W = 1490;
const int H = 682;

float DEGTORAD = 0.017453f;

class Animation
{
public:
	float Frame, speed;
	Sprite sprite;
	std::vector<IntRect> frames;

	Animation()
	{}

	Animation(Texture& t, int x, int y, int w, int h, int count, float Speed)
	{
		Frame = 0;
		speed = Speed;

		for (int i = 0; i < count; i++)
			frames.push_back(IntRect(x + i * w, y, w, h));

		sprite.setTexture(t);
		sprite.setOrigin(w / 2, h / 2);
		sprite.setTextureRect(frames[0]);
	}

	void update()
	{
		Frame += speed;
		int n = frames.size();
		if (Frame >= n)
			Frame -= n;
		if (n > 0)
			sprite.setTextureRect(frames[int(Frame)]);
	}

	bool isEnd()
	{
		return Frame + speed >= frames.size();
	}
};

class Entity
{
public:
	float x, y, dx, dy, R, angle;
	bool life;
	std::string name;
	Animation anim;

	Entity()
	{
		life = 1;
	}

	void settings(Animation& a, int X, int Y, float Angle = 0, int radius = 1)
	{
		anim = a;
		x = X;
		y = Y;
		angle = Angle;
		R = radius;
	}

	virtual void update() {};

	void draw(RenderWindow& app)
	{
		anim.sprite.setPosition(x, y);
		anim.sprite.setRotation(angle + 90);
		app.draw(anim.sprite);

		CircleShape circle(R);
		circle.setFillColor(Color(255, 0, 0, 170));
		circle.setPosition(x, y);
		circle.setOrigin(R, R);
		//app.draw(circle);
	}

	virtual ~Entity() {};
};

class asteroid : public Entity
{
public:
	asteroid()
	{
		dx = rand() % 8 - 4;
		dy = rand() % 8 - 4;
		name = "asteroid";
	}

	void update()
	{
		x += dx;
		y += dy;

		if (x > W)
			x = 0;
		if (x < 0)
			x = W;
		if (y > H)
			y = 0;
		if (y < 0)
			y = H;
	}
};

class bullet : public Entity
{
public:
	bullet()
	{
		name = "bullet";
	}

	void update()
	{
		dx = cos(angle * DEGTORAD) * 6;
		dy = sin(angle * DEGTORAD) * 6;
		// angle+=rand()%6-3;
		x += dx;
		y += dy;

		if (x > W || x < 0 || y > H || y < 0)
			life = 0;
	}
};

class player : public Entity
{
public:
	bool thrust;
	bool thrust2;

	player()
	{
		name = "player";
	}

	void update()
	{
		if (thrust)
		{
			dx += cos(angle * DEGTORAD) * 0.2;
			dy += sin(angle * DEGTORAD) * 0.2;
		}
		else if (thrust2)
		{
			dx -= cos(angle * DEGTORAD) * 0.2;
			dy -= sin(angle * DEGTORAD) * 0.2;
		}
		else
		{
			dx *= 0.99;
			dy *= 0.99;
		}

		int maxSpeed = 15;
		float speed = sqrt(dx * dx + dy * dy);
		if (speed > maxSpeed)
		{
			dx *= maxSpeed / speed;
			dy *= maxSpeed / speed;
		}

		x += dx;
		y += dy;

		if (x > W)
			x = 0;
		if (x < 0)
			x = W;
		if (y > H)
			y = 0;
		if (y < 0)
			y = H;
	}
};

bool isCollide(Entity* a, Entity* b)
{
	return (b->x - a->x) * (b->x - a->x) + (b->y - a->y) * (b->y - a->y) < (a->R + b->R) * (a->R + b->R);
}

int main()
{
	// play music
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile("content/music.ogg"))
		return -1;

	sf::Sound sound;
	sound.setBuffer(buffer);

	sf::SoundBuffer buffer2;
	if (!buffer2.loadFromFile("content/endmusic.ogg"))
		return -1;

	sf::Sound sound2;
	sound2.setBuffer(buffer2);

	sf::SoundBuffer buffer3;
	if (!buffer3.loadFromFile("content/daddymad.ogg"))
		return -1;

	sf::Sound sound3;
	sound3.setBuffer(buffer3);

	sf::SoundBuffer buffer4;
	if (!buffer4.loadFromFile("content/gotcha.ogg"))
		return -1;

	sf::Sound sound4;
	sound4.setBuffer(buffer4);

	// count score and life
	int score = 0;
	int heart = 15;

	// Declare and load a font
	sf::Font myfont;
	myfont.loadFromFile("content/The_Machinatos.ttf");
	// Create a text scorelabel
	sf::Text scoretext;
	scoretext.setFont(myfont);
	scoretext.setFillColor(sf::Color::Red);
	scoretext.setStyle(sf::Text::Bold);
	scoretext.setCharacterSize(60);
	scoretext.setString("Score:");
	// Create a text score
	sf::Text currentscore;
	currentscore.setFont(myfont);
	currentscore.setFillColor(sf::Color::Red);
	currentscore.setStyle(sf::Text::Bold);
	currentscore.setCharacterSize(60);
	currentscore.setPosition(140, 0);
	std::stringstream s;
	s << score;
	currentscore.setString(s.str());

	// Create a text lifelabel
	sf::Text lifetext;
	lifetext.setFont(myfont);
	lifetext.setFillColor(sf::Color::Red);
	lifetext.setStyle(sf::Text::Bold);
	lifetext.setCharacterSize(60);
	lifetext.setString("Life:");
	lifetext.setPosition(0, 100);
	// Create a text life
	sf::Text currentlife;
	currentlife.setFont(myfont);
	currentlife.setFillColor(sf::Color::Red);
	currentlife.setStyle(sf::Text::Bold);
	currentlife.setCharacterSize(60);
	currentlife.setPosition(140, 100);
	std::stringstream l;
	l << heart;
	currentlife.setString(l.str());

	//create window
	RenderWindow app(VideoMode(W, H), "Asteroids!");
	app.setFramerateLimit(60);

	//input images
	Texture t1, t2, t3, t4, t5, t6, t7, tadd, tadd2;
	t1.loadFromFile("content/images/spaceship.png");
	t2.loadFromFile("content/images/background.png");
	t3.loadFromFile("content/images/explosions/type_C.png");
	t4.loadFromFile("content/images/test.png");
	t5.loadFromFile("content/images/fire_blue.png");
	t6.loadFromFile("content/images/pic.png");
	t7.loadFromFile("content/images/explosions/type_B.png");
	tadd.loadFromFile("content/images/endgame1.png");
	tadd2.loadFromFile("content/images/endgame2.png");

	t1.setSmooth(true);
	t2.setSmooth(true);
	tadd.setSmooth(true);
	tadd2.setSmooth(true);

	Sprite background(t2);
	Sprite endgame(tadd);
	Sprite endgame2(tadd2);

	//set animations
	Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
	Animation sRock(t4, 0, 0, 118, 115, 16, 0.0000002);
	Animation sRock_small(t6, 0, 0, 63, 70, 16, 0.0000002);
	Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
	Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
	Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
	Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

	//asteroid setup
	std::list<Entity*> entities;

	for (int i = 0; i < 15; i++)
	{
		asteroid* a = new asteroid();
		a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
		entities.push_back(a);
	}
	//player setup
	player* p = new player();
	p->settings(sPlayer, 200, 200, 0, 20);
	entities.push_back(p);

	/////main loop/////
	bool run = true;
	bool run2 = false;
	bool playMusic = false;
	while (app.isOpen())
	{
		//keyboard control
		Event event;
		while (app.pollEvent(event))
		{
			if (event.type == Event::Closed)
				app.close();

			if (event.type == Event::KeyPressed)
			{
				if (event.key.code == Keyboard::Space)
				{
					bullet* b = new bullet();
					b->settings(sBullet, p->x, p->y, p->angle, 10);
					entities.push_back(b);
				}
			}
		}

		if (run == true)
		{
			if (Keyboard::isKeyPressed(Keyboard::Right))
				p->angle += 3;
			if (Keyboard::isKeyPressed(Keyboard::Left))
				p->angle -= 3;
			if (Keyboard::isKeyPressed(Keyboard::Up))
				p->thrust = true;
			else
				p->thrust = false;
			if (Keyboard::isKeyPressed(Keyboard::Down))
				p->thrust2 = true;
			else
				p->thrust2 = false;
		}

		//animation process
		for (auto a : entities)
		{
			for (auto b : entities)
			{
				//asteroid shot
				if (a->name == "asteroid" && b->name == "bullet")
				{
					if (isCollide(a, b))
					{
						score++;
						std::stringstream s;
						s << score;
						currentscore.setString(s.str());

						a->life = false;
						b->life = false;

						Entity* e = new Entity();
						e->settings(sExplosion, a->x, a->y);
						e->name = "explosion";
						entities.push_back(e);

						for (int i = 0; i < 2; i++)
						{
							if (a->R == 15)
							{
								sound3.play();
								continue;
							}
							sound4.play();
							Entity* e = new asteroid();
							e->settings(sRock_small, a->x, a->y, rand() % 360, 15);
							entities.push_back(e);
						}
					}
				}

				//player hit
				if (a->name == "player" && b->name == "asteroid")
				{
					if (isCollide(a, b))
					{
						if (heart > 1)
						{
							sound.play();
							heart--;
							std::stringstream l;
							l << heart;
							currentlife.setString(l.str());
						}
						else
						{
							run = false;
						}

						b->life = false;

						Entity* e = new Entity();
						e->settings(sExplosion_ship, a->x, a->y);
						e->name = "explosion";
						entities.push_back(e);

						p->settings(sPlayer, W / 2, H / 2, 0, 20);
						p->dx = 0;
						p->dy = 0;
					}
				}
			}
		}
		if (p->thrust)
			p->anim = sPlayer_go;
		else
			p->anim = sPlayer;

		for (auto e : entities)
			if (e->name == "explosion")
				if (e->anim.isEnd())
					e->life = 0;

		if (rand() % 150 == 0)
		{
			asteroid* a = new asteroid();
			a->settings(sRock, 0, rand() % H, rand() % 360, 25);
			entities.push_back(a);
		}

		for (auto i = entities.begin(); i != entities.end();)
		{
			Entity* e = *i;

			e->update();
			e->anim.update();

			if (e->life == false)
			{
				i = entities.erase(i);
				delete e;
			}
			else
				i++;
		}

		//////draw//////
		if (run == true)
		{
			app.draw(background);

			app.draw(scoretext);
			app.draw(currentscore);
			app.draw(lifetext);
			app.draw(currentlife);

			for (auto i : entities)
			{
				i->draw(app);
			}

			app.display();
		}

		if (run == false)
		{
			if (run2 == false)
			{
				playMusic = true;
				if (playMusic == true)
				{
					sound.stop();
					sound2.play();

					playMusic = false;
				}
				app.draw(endgame);
				app.display();
				clock_t start_time;
				start_time = clock();
				while ((clock() - start_time) < 1.5 * CLOCKS_PER_SEC)
				{
					;
				}
				run2 = true;
			}
			else
			{
				app.draw(endgame2);
				app.display();
			}
		}
	}

	return 0;
}
