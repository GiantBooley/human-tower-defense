#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <linmath.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;


GLFWwindow* window;

struct Vertex {
	float x, y, z;
	float u, v;
	float space;
};
vector<Vertex> vertices = {};
vector<unsigned int> indices = {};

float randFloat() {
	return static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
}

static const char* vertex_shader_text = 
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec2 vTexCoord;\n"
"attribute vec3 vPos;\n"
"attribute float vSpace;\n"
"varying vec2 texCoord;\n"
"varying vec3 pos;\n"
"void main() {\n"
"	vec4 position = vSpace == 1.0 ? MVP * vec4(vPos, 1.0) : vec4(vPos, 1.0);\n"
"	gl_Position = position;\n"
"	texCoord = vTexCoord / 32.0;\n"
"	pos = vPos;\n"
"}";
static const char* fragment_shader_text =
"#version 110\n"
"varying vec2 texCoord;\n"
"varying vec3 pos;\n"
"uniform sampler2D texture1;\n"
"void main() {\n"
"	vec4 color = texture2D(texture1, texCoord);\n"
"	gl_FragColor = color;\n"
"}";

class Vec2 {
public:
	float x = 0.f;
	float y = 0.f;
	Vec2(float asdx, float asdy) {
		x = asdx;
		y = asdy;
	}
};
class Controls {
	public:
	   bool w = false;
	   bool a = false;
	   bool s = false;
	   bool d = false;
	   bool space = false;
	   bool left = false;
	   bool right = false;
	   bool up = false;
	   bool down = false;
	   bool p = false;
	   bool mouseDown = false;
	   Vec2 mouse{0.f, 0.f};
	   Vec2 worldMouse{0.f, 0.f};
};
class Vec3 {
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	Vec3(float asdx, float asdy, float asdz) {
		x = asdx;
		y = asdy;
		z = asdz;
	};
	Vec3(float asd) {
		x = asd;
		y = asd;
		z = asd;
	};
	float getMagnitude() {
		return sqrt(x*x+y*y+z*z);
	}
	Vec3 normalise(float strength) {
		float magnitude = getMagnitude();
		return {x / magnitude * strength, y / magnitude * strength, z / magnitude * strength};
	}
};
Vec3 vec3Add(Vec3 a, Vec3 b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}
Vec3 vec3Add(Vec3 a, float b) {
	return {a.x + b, a.y + b, a.z + b};
}
Vec3 vec3Subtract(Vec3 a, Vec3 b) {
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}
Vec3 vec3Subtract(Vec3 a, float b) {
	return {a.x - b, a.y - b, a.z - b};
}
Vec3 vec3Mul(Vec3 a, Vec3 b) {
	return {a.x * b.x, a.y * b.y, a.z * b.z};
}
Vec3 vec3Mul(Vec3 a, float b) {
	return {a.x * b, a.y * b, a.z * b};
}
#define ENTITY_NORMAL 0
#define ENTITY_FAST 1
#define ENTITY_MONSTER 2
class Entity {
public:
	int type;
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 size{0.5f, 1.5f, 0.5f};
	int targetPoint = 0;
	float health = 1.f;
	float speed = 0.03f;
	float u;
	float reward;
	float damage;
	Entity(int tyipe, Vec3 position) {
		type = tyipe;
		pos = position;
		switch (type) {
		case ENTITY_NORMAL:
			speed = 0.03f;
			u = 0.f;
			reward = 1.f;
			damage = 1.f;
			break;
		case ENTITY_FAST:
			speed = 0.1f;
			u = 1.f;
			reward = 3.f;
			damage = 2.f;
			break;
		case ENTITY_MONSTER:
			speed = 1.f;
			u = 2.f;
			reward = 3123.f;
			damage = 2213.f;
		}
	}
};
class Camera {
public:
	Vec3 pos{5.f, 5.f, 6.f};
	float fov = 1.57f;
	Vec3 rotation{1.57f, 1.57f, 0.f};
};
float distance3D(Vec3 p1, Vec3 p2) {
	return sqrt(pow(p2.x - p1.x, 2.f) + pow(p2.y - p1.y, 2.f) + pow(p2.z - p1.z, 2.f));
}
class Level {
public:
	vector<Vec3> path = {{2.f, 1.f, 0.f}, {5.f, 1.f, 2.f}, {2.f, 1.f, 8.f}, {4.f, 1.f, 8.f}, {4.f, 1.f, 3.f}, {1.f, 1.f, 3.f}, {1.f, 1.f, 9.f}, {6.f, 1.f, 9.f}, {6.f, 1.f, 3.f}, {9.f, 1.f, 3.f}, {9.f, 1.f, 10.f}};
};
#define PERSON_ARCHER 0
class Person {
	public:
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 size{1.f, 1.f, 1.f};
	int shootDelay = 0;
	float price;
	Person(int type, Vec3 asdpos) {
		pos = asdpos;
		switch (type) {
		case 0:
			price = 100.f;
		}
	}
};
class Projectile {
	public:
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 velocity{0.f, 0.f, 0.f};
	Vec3 size{0.3f, 0.3f, 0.3f};
	float age = 0.f;
	Projectile(Vec3 asdpos, Vec3 asdvelocity) {
		pos = asdpos;
		velocity = asdvelocity;
	}
};
Controls controls;
float roundToPlace(float x, float place) {
	return round(x / place) * place;
}
class Wave {
	public:
		vector<int> entities;
		Wave(vector<int> them) {
			entities = them;
		}
};
class GameState {
public:
	vector<Entity> entities = {};
	vector<Person> people = {};
	vector<Projectile> projectiles = {};
	Camera camera;
	Level levels[1] = {{}};
	int activeLevel = 0;
	float health = 100.f;
	int entitySpawnDelay = 5;
	Person placingPerson{PERSON_ARCHER, {2.f, 1.f, 2.f}};
	bool isPlacingPerson = false;
	float money = 4010.f;
	Wave waveCurrentlySpawning{{}};
	Wave waves[6] = {
		{{ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL}},
		{{ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL}},
		{{ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_FAST, ENTITY_NORMAL, ENTITY_FAST, ENTITY_NORMAL, ENTITY_FAST}},
		{{ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_NORMAL, ENTITY_FAST, ENTITY_NORMAL, ENTITY_FAST, ENTITY_NORMAL, ENTITY_FAST, ENTITY_MONSTER}},
		{{ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_FAST, ENTITY_NORMAL, ENTITY_FAST, ENTITY_MONSTER}},
		{{ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER, ENTITY_MONSTER}}
	};
	int waveNumber = 0;
	
	void tick(int width, int height) {
		if (controls.w) camera.pos.z -= 0.1f;
		if (controls.a) camera.pos.x -= 0.1f;
		if (controls.s) camera.pos.z += 0.1f;
		if (controls.d) camera.pos.x += 0.1f;
		if (controls.up) camera.rotation.x += 0.01f;
		if (controls.down) camera.rotation.x -= 0.01f;
		if (controls.left) camera.rotation.y -= 0.01f;
		if (controls.right) camera.rotation.y += 0.01f;
		controls.worldMouse.x = controls.mouse.x * 10.f / (float)width;
		controls.worldMouse.y = 10.f - controls.mouse.y * 10.f / (float)height;

		if (entitySpawnDelay <= 0 && waveCurrentlySpawning.entities.size() > 0) {
			entities.push_back({waveCurrentlySpawning.entities[0], levels[activeLevel].path[0]});
			waveCurrentlySpawning.entities.erase(waveCurrentlySpawning.entities.begin());
			entitySpawnDelay = 20;
		} else {
			entitySpawnDelay--;
		};
		if (controls.space) {
			isPlacingPerson = true;
		}
		if (isPlacingPerson) {
			placingPerson.pos.x = controls.worldMouse.y;
			placingPerson.pos.z = controls.worldMouse.x;
		}

		for (int i = 0; i < people.size(); i++) {
			if (people[i].shootDelay <= 0) {
				people[i].shootDelay = 50;
				if (entities.size() > 0) {
					spawnProjectile(people[i].pos.x, people[i].pos.z, vec3Subtract(entities[0].pos, people[i].pos).normalise(0.4f));
				}
			} else {
				people[i].shootDelay--;
			}
		}
		for (int i = projectiles.size() - 1; i >= 0; i--) {
			projectiles[i].pos = vec3Add(projectiles[i].pos, projectiles[i].velocity);
			projectiles[i].age++;
			if (projectiles[i].age > 300) {
				projectiles.erase(projectiles.begin() + i);
				continue;
			}
			for (int j = 0; j < entities.size(); j++) {
				if (
					projectiles[i].pos.x - projectiles[i].size.x / 2.f < entities[j].pos.x + entities[j].size.x / 2.f &&
					projectiles[i].pos.x + projectiles[i].size.x / 2.f > entities[j].pos.x - entities[j].size.x / 2.f &&
					projectiles[i].pos.y                               < entities[j].pos.y + entities[j].size.y       &&
					projectiles[i].pos.y + projectiles[i].size.y       > entities[j].pos.y                            &&
					projectiles[i].pos.z - projectiles[i].size.z / 2.f < entities[j].pos.z + entities[j].size.z / 2.f &&
					projectiles[i].pos.z + projectiles[i].size.z / 2.f > entities[j].pos.z - entities[j].size.z / 2.f
				) {
					projectiles.erase(projectiles.begin() + i);
					entities[j].health -= 1.f;
					if (entities[j].health <= 0.f) {
						money += entities[j].reward;
					};
					break;
				};
			}
		}
		for (int i = entities.size() - 1; i >= 0; i--) {
			bool die = entities[i].health <= 0.f;
			Vec3 targetPoint = levels[activeLevel].path[entities[i].targetPoint];
			float distanceToTargetPoint = distance3D(entities[i].pos, targetPoint);
			if (distanceToTargetPoint < entities[i].speed) {
				entities[i].targetPoint++;
				if (entities[i].targetPoint >= levels[activeLevel].path.size()) {
					health -= entities[i].damage;
					die = true;
				}
				targetPoint = levels[activeLevel].path[entities[i].targetPoint];
				distanceToTargetPoint = distance3D(entities[i].pos, targetPoint);
			}
			entities[i].pos.x += (targetPoint.x - entities[i].pos.x) / distanceToTargetPoint * entities[i].speed;
			entities[i].pos.y += (targetPoint.y - entities[i].pos.y) / distanceToTargetPoint * entities[i].speed;
			entities[i].pos.z += (targetPoint.z - entities[i].pos.z) / distanceToTargetPoint * entities[i].speed;
			if (die) {
				entities.erase(entities.begin() + i);
			}
		}
		if (health <= 0.f) {
			camera.pos.y = 123987.f;
		}
	}
	void spawnEntity() {
		if (randFloat() < 0.5) {
			entities.push_back({ENTITY_NORMAL, levels[activeLevel].path[0]});
		} else {
			entities.push_back({ENTITY_FAST, levels[activeLevel].path[0]});
		}
	}
	void spawnProjectile(float x, float z, Vec3 vel) {
		projectiles.push_back({{x, 2.f, z}, vec3Add(vel, (Vec3){randFloat() * 0.1f - 0.05f, 0.f, randFloat() * 0.1f - 0.05f})});
	}
	void spawnWave(Wave wave) {
		waveCurrentlySpawning = wave;
	}
};
Vec2 getCharacterCoords(char c) {
	Vec2 coords{31.f, 4.f};
	switch (c) {
		case 'a':coords = {0.f, 3.f};break;
		case 'b':coords = {1.f, 3.f};break;
		case 'c':coords = {2.f, 3.f};break;
		case 'd':coords = {3.f, 3.f};break;
		case 'e':coords = {4.f, 3.f};break;
		case 'f':coords = {5.f, 3.f};break;
		case 'g':coords = {6.f, 3.f};break;
		case 'h':coords = {7.f, 3.f};break;
		case 'i':coords = {8.f, 3.f};break;
		case 'j':coords = {9.f, 3.f};break;
		case 'k':coords = {10.f, 3.f};break;
		case 'l':coords = {11.f, 3.f};break;
		case 'm':coords = {12.f, 3.f};break;
		case 'n':coords = {13.f, 3.f};break;
		case 'o':coords = {14.f, 3.f};break;
		case 'p':coords = {15.f, 3.f};break;
		case 'q':coords = {16.f, 3.f};break;
		case 'r':coords = {17.f, 3.f};break;
		case 's':coords = {18.f, 3.f};break;
		case 't':coords = {19.f, 3.f};break;
		case 'u':coords = {20.f, 3.f};break;
		case 'v':coords = {21.f, 3.f};break;
		case 'w':coords = {22.f, 3.f};break;
		case 'x':coords = {23.f, 3.f};break;
		case 'y':coords = {24.f, 3.f};break;
		case 'z':coords = {25.f, 3.f};break;

		case '1':coords = {0.f, 5.f};break;
		case '2':coords = {1.f, 5.f};break;
		case '3':coords = {2.f, 5.f};break;
		case '4':coords = {3.f, 5.f};break;
		case '5':coords = {4.f, 5.f};break;
		case '6':coords = {5.f, 5.f};break;
		case '7':coords = {6.f, 5.f};break;
		case '8':coords = {7.f, 5.f};break;
		case '9':coords = {8.f, 5.f};break;
		case '0':coords = {9.f, 5.f};break;
		
		case 'A':coords = {0.f, 4.f};break;
		case 'B':coords = {1.f, 4.f};break;
		case 'C':coords = {2.f, 4.f};break;
		case 'D':coords = {3.f, 4.f};break;
		case 'E':coords = {4.f, 4.f};break;
		case 'F':coords = {5.f, 4.f};break;
		case 'G':coords = {6.f, 4.f};break;
		case 'H':coords = {7.f, 4.f};break;
		case 'I':coords = {8.f, 4.f};break;
		case 'J':coords = {9.f, 4.f};break;
		case 'K':coords = {10.f, 4.f};break;
		case 'L':coords = {11.f, 4.f};break;
		case 'M':coords = {12.f, 4.f};break;
		case 'N':coords = {13.f, 4.f};break;
		case 'O':coords = {14.f, 4.f};break;
		case 'P':coords = {15.f, 4.f};break;
		case 'Q':coords = {16.f, 4.f};break;
		case 'R':coords = {17.f, 4.f};break;
		case 'S':coords = {18.f, 4.f};break;
		case 'T':coords = {19.f, 4.f};break;
		case 'U':coords = {20.f, 4.f};break;
		case 'V':coords = {21.f, 4.f};break;
		case 'W':coords = {22.f, 4.f};break;
		case 'X':coords = {23.f, 4.f};break;
		case 'Y':coords = {24.f, 4.f};break;
		case 'Z':coords = {25.f, 4.f};break;
		
		case '!':coords = {26.f, 3.f};break;
		case '@':coords = {27.f, 3.f};break;
		case '#':coords = {28.f, 3.f};break;
		case '$':coords = {29.f, 3.f};break;
		case '%':coords = {30.f, 3.f};break;
		case '^':coords = {31.f, 3.f};break;
		case '&':coords = {26.f, 4.f};break;
		case '*':coords = {27.f, 4.f};break;
		case '(':coords = {28.f, 4.f};break;
		case ')':coords = {29.f, 4.f};break;
		case '-':coords = {30.f, 4.f};break;
		case '=':coords = {31.f, 4.f};break;
		
		case '`':coords = {10.f, 5.f};break;
		case '~':coords = {11.f, 5.f};break;
		case '_':coords = {12.f, 5.f};break;
		case '+':coords = {13.f, 5.f};break;
		case '[':coords = {14.f, 5.f};break;
		case ']':coords = {15.f, 5.f};break;
		case '{':coords = {16.f, 5.f};break;
		case '}':coords = {17.f, 5.f};break;
		case '\\':coords = {18.f, 5.f};break;
		case '|':coords = {19.f, 5.f};break;
		case ';':coords = {20.f, 5.f};break;
		case '\'':coords = {21.f, 5.f};break;
		case ':':coords = {22.f, 5.f};break;
		case '"':coords = {23.f, 5.f};break;
		case ',':coords = {24.f, 5.f};break;
		case '.':coords = {25.f, 5.f};break;
		case '<':coords = {26.f, 5.f};break;
		case '>':coords = {27.f, 5.f};break;
		case '/':coords = {28.f, 5.f};break;
		case '?':coords = {29.f, 5.f};break;
		case ' ':coords = {30.f, 5.f};break;
	};
	return coords;
};
class GameStateVertexBuilder {
public:
	void buildThem(GameState* game, int width, int height) {
		clearVertices();
		// ground
		for (int x = 0; x < 10; x++) {
			for (int z = 0; z < 10; z++) {
				addPlane((float)x, 1.f, (float)z, 1.f, 1.f, 0.f, 0.f);
			}
		}
		// point path
		for (int i = 0; i < game->levels[game->activeLevel].path.size() - 1; i++) {
			addPath(game->levels[game->activeLevel].path[i], game->levels[game->activeLevel].path[i + 1], 1.f, 0.f, 0.4f);
		}
		// entities
		for (Entity entity : game->entities) {
			addCube(entity.pos.x - entity.size.x / 2.f, entity.pos.y, entity.pos.z - entity.size.z / 2.f, entity.size.x, entity.size.y, entity.size.z, entity.u, 1.f);
		}
		//people
		for (Person person : game->people) {
			addCube(person.pos.x - person.size.x / 2.f, person.pos.y, person.pos.z - person.size.z / 2.f, person.size.x, person.size.y, person.size.z, 0.f, 2.f);
		}
		if (game->isPlacingPerson) {
			addCube(game->placingPerson.pos.x - game->placingPerson.size.x / 2.f, game->placingPerson.pos.y, game->placingPerson.pos.z - game->placingPerson.size.z / 2.f, game->placingPerson.size.x, game->placingPerson.size.y, game->placingPerson.size.z, 0.f, 2.f);
		}
		//projectiles
		for (Projectile projectile : game->projectiles) {
			addPath(projectile.pos, vec3Add(projectile.pos, projectile.velocity), 0.f, 6.f, 0.1f);
		}
		addText("HEALTH: " + to_string((int)game->health), -0.9f, 0.8f, 0.1f, 0.8f);
		addText("WAVE " + to_string(game->waveNumber), 0.3f, 0.8f, 0.1f, 0.8f);
		addText("$" + to_string((int)game->money), -0.9f, 0.6f, 0.1f, 0.8f);
		addText("(space) archer: $100", -0.9f, -0.9f, -0.9f, 0.8f);
	}
private:
	void clearVertices() {
		vertices.clear();
		indices.clear();
	}
	void addCube(float x, float y, float z, float w, float h, float d, float u, float v) {
		unsigned int end = vertices.size();
		indices.insert(indices.end(), {
			2U+end, 6U+end, 7U+end,
			2U+end, 3U+end, 7U+end,

			0U+end, 4U+end, 5U+end,
			0U+end, 1U+end, 5U+end,

			0U+end, 2U+end, 6U+end,
			0U+end, 4U+end, 6U+end,

			1U+end, 3U+end, 7U+end,
			1U+end, 5U+end, 7U+end,

			0U+end, 2U+end, 3U+end,
			0U+end, 1U+end, 3U+end,

			4U+end, 6U+end, 7U+end,
			4U+end, 5U+end, 7U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y  , z+d, u    , v    , 1.f},
			{x+w, y  , z+d, u+1.f, v    , 1.f},
			{x  , y+h, z+d, u    , v+1.f, 1.f},
			{x+w, y+h, z+d, u+1.f, v+1.f, 1.f},
			{x  , y  , z  , u    , v+1.f, 1.f},
			{x+w, y  , z  , u+1.f, v+1.f, 1.f},
			{x  , y+h, z  , u    , v    , 1.f},
			{x+w, y+h, z  , u+1.f, v    , 1.f}
		});
	}
	void addPlane(float x, float y, float z, float w, float d, float u, float v) {
		unsigned int end = vertices.size();
		indices.insert(indices.end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y  , z+d, u    , v    , 1.f},
			{x+w, y  , z+d, u+1.f, v    , 1.f},
			{x  , y  , z  , u    , v+1.f, 1.f},
			{x+w, y  , z  , u+1.f, v+1.f, 1.f}
		});
	}
	
	void addCharacter(char character, float x, float y, float w) {
		unsigned int end = vertices.size();
		indices.insert(indices.end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
		float u = getCharacterCoords(character).x;
		float v = getCharacterCoords(character).y;
		vertices.insert(vertices.end(), {
			{x  , y+w, x*-0.1f, u    , v    , 0.f},
			{x+w, y+w, x*-0.1f, u+1.f, v    , 0.f},
			{x  , y  , x*-0.1f, u    , v+1.f, 0.f},
			{x+w, y  , x*-0.1f, u+1.f, v+1.f, 0.f}
		});
	}
	void addText(string text, float x, float y, float w, float spacing) {
		for (int i = 0; i < text.length(); i++) {
			addCharacter(text.at(i), x + (float)i * w * spacing, y, w*1.5);
		}
	}
	void addPath(Vec3 p1, Vec3 p2, float u, float v, float width) {
		unsigned int end = vertices.size();
		float dist = distance3D(p1, p2);
		float xDiff = (p2.x - p1.x) / dist * width;
		float zDiff = (p2.z - p1.z) / dist * width;
		vertices.insert(vertices.end(), {
			{p1.x - zDiff - xDiff, p1.y + 0.01f, p1.z + xDiff - zDiff, u      , v + 1.f, 1.f},
			{p1.x + zDiff - xDiff, p1.y + 0.01f, p1.z - xDiff - zDiff, u + 1.f, v + 1.f, 1.f},
			{p2.x - zDiff + xDiff, p2.y + 0.01f, p2.z + xDiff + zDiff, u      , v      , 1.f},
			{p2.x + zDiff + xDiff, p2.y + 0.01f, p2.z - xDiff + zDiff, u + 1.f, v      , 1.f}
		});
		indices.insert(indices.end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
	}
};
GameState game;
GameStateVertexBuilder vBuilder;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		else if (key == GLFW_KEY_W) controls.w = true;
		else if (key == GLFW_KEY_A) controls.a = true;
		else if (key == GLFW_KEY_S) controls.s = true;
		else if (key == GLFW_KEY_D) controls.d = true;
		else if (key == GLFW_KEY_SPACE) controls.space = true;
		else if (key == GLFW_KEY_LEFT) controls.left = true;
		else if (key == GLFW_KEY_RIGHT) controls.right = true;
		else if (key == GLFW_KEY_UP) controls.up = true;
		else if (key == GLFW_KEY_DOWN) controls.down = true;
		else if (key == GLFW_KEY_P) controls.p = true;
		else if (key == GLFW_KEY_Z && game.entities.size() == 0 && game.waveNumber < sizeof(game.waves) / sizeof(game.waves[0])) {
			game.spawnWave(game.waves[game.waveNumber]);
			game.waveNumber++;
		};
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) controls.w = false;
		else if (key == GLFW_KEY_A) controls.a = false;
		else if (key == GLFW_KEY_S) controls.s = false;
		else if (key == GLFW_KEY_D) controls.d = false;
		else if (key == GLFW_KEY_SPACE) controls.space = false;
		else if (key == GLFW_KEY_LEFT) controls.left = false;
		else if (key == GLFW_KEY_RIGHT) controls.right = false;
		else if (key == GLFW_KEY_UP) controls.up = false;
		else if (key == GLFW_KEY_DOWN) controls.down = false;
		else if (key == GLFW_KEY_P) controls.p = false;
	};
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	double xpos, ypos;
    	//getting cursor position
    	glfwGetCursorPos(window, &xpos, &ypos);

		if (game.isPlacingPerson && game.money >= game.placingPerson.price) {
			game.people.push_back(game.placingPerson);
			game.isPlacingPerson = false;
			game.money -= game.placingPerson.price;
		};

		controls.mouseDown = true;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
       double xpos, ypos;
       //getting cursor position
       glfwGetCursorPos(window, &xpos, &ypos);
	   controls.mouseDown = false;
    }
}
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	controls.mouse = {(float)xpos, (float)ypos};
}

static void error_callback(int error, const char* description) {
	fprintf(stderr, "ERROR: %s\n", description);
}

int main(void) {
	//getShaderText();
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vtexcoord_location, vspace_location, texture1_location;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Human Tower Defence", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	GLFWimage images[1]{};
	images[0].pixels = stbi_load("icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(window, 1, images);
	glfwWindowHint(GLFW_SAMPLES, 4);
	stbi_image_free(images[0].pixels);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	// NOTE: OpenGL error checks have been omitted for brevity

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);
	
	GLint vertexSuccess = GL_FALSE;//error handeling    
	char vertexInfoLog[512];

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertexSuccess);
	if (!vertexSuccess){
		glGetShaderInfoLog(vertex_shader, 512, NULL, vertexInfoLog);
		cerr << "ERROR: vertex Shader compilation error: " << vertexInfoLog << endl;

		glDeleteShader(vertex_shader);
		return -1;
	} else {
		cout << "INFO: successfuly loaded vert exshader" << endl;
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
	
	GLint fragmentSuccess = GL_FALSE;//error handeling    
	char fragmentInfoLog[512];

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragmentSuccess);
	if (!fragmentSuccess){
		glGetShaderInfoLog(fragment_shader, 512, NULL, fragmentInfoLog);
		cerr << "ERROR: fragment Shader compilation error: " << fragmentInfoLog << endl;

		glDeleteShader(vertex_shader);
		return -1;
	} else {
		cout << "INFO: successfuly loaded  fragmentshader" << endl;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	mvp_location = glGetUniformLocation(program, "MVP");
	texture1_location = glGetUniformLocation(program, "texture1");
	vpos_location = glGetAttribLocation(program, "vPos");
	vtexcoord_location = glGetAttribLocation(program, "vTexCoord");
	vspace_location = glGetAttribLocation(program, "vSpace");

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)0);
	glEnableVertexAttribArray(vtexcoord_location);
	glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(vspace_location);
	glVertexAttribPointer(vspace_location, 1, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)(sizeof(float) * 5));

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int widthImg, heightImg, numColCh;
	unsigned char *bytes = stbi_load("resources/atlas.png", &widthImg, &heightImg, &numColCh, 0);

	if (bytes) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "INFO: successfully loaded texture atlas" << endl;
	} else {
		cerr << "ERROR: failed to load texture atlas. make sure resources/atlas.png exists" << endl;
	};


	stbi_image_free(bytes);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	while (!glfwWindowShouldClose(window)) {
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		game.tick(width, height);
		vBuilder.buildThem(&game, width, height);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		
		glViewport(0, 0, width, height);
		glClearColor(0.5f, 0.5f, 0.7f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, texture);

		mat4x4_identity(m);
		mat4x4_translate(m, -game.camera.pos.x, -game.camera.pos.y, -game.camera.pos.z);
		mat4x4_rotate_X(m, m, game.camera.rotation.x);
		mat4x4_rotate_Y(m, m, game.camera.rotation.y);
		mat4x4_rotate_Z(m, m, game.camera.rotation.z);
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_perspective(p, 1.57f, ratio, 0.1f, 200.f);
		mat4x4_mul(mvp, p, m);

		glUniform1i(texture1_location, 0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteTextures(1, &texture);
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
