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
#include <string>
#include <chrono>

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
float lerp(float a, float b, float t) {
	return (b - a) * t + a;
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
bool lineCircleIntersects(float ax, float ay, float bx, float by, float cx, float cy, float r) {
	ax -= cx;
	ay -= cy;
	bx -= cx;
	by -= cy;
	float a = pow(bx - ax, 2.f) + pow(by - ay, 2.f);
	float b = 2.f*(ax*(bx - ax) + ay*(by - ay));
	float c = pow(ax, 2.f) + pow(ay, 2.f) - pow(r, 2.f);
	float disc = pow(b, 2.f) - 4.f*a*c;
	if(disc <= 0.f) return false;
	float sqrtdisc = sqrt(disc);
	float t1 = (-b + sqrtdisc)/(2.f*a);
	float t2 = (-b - sqrtdisc)/(2.f*a);
	if((0.f < t1 && t1 < 1.f) || (0.f < t2 && t2 < 1.f)) return true;
	return false;
}
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
	   Vec2 clipMouse{0.f, 0.f};
	   Vec2 previousClipMouse{0.f, 0.f};
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
		float magnitude = max(getMagnitude(), 0.001f);
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
#define ENTITY_ADMIN 3
#define ENTITY_IRON_MAIDEN 4
#define ENTITY_TUNGSTEN_MAIDEN 5
#define ENTITY_TWIN 6
#define ENTITY_GENERAL 7
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
	float yRotation = 0.f;
	Entity(int tyipe, Vec3 position) {
		type = tyipe;
		pos = position;
		switch (type) {
		case ENTITY_NORMAL:
			speed = 0.03f;
			u = 0.f;
			health = 1.f;
			reward = 1.f;
			damage = 1.f;
			break;
		case ENTITY_FAST:
			speed = 0.1f;
			u = 1.f;
			health = 1.f;
			reward = 5.f;
			damage = 2.f;
			break;
		case ENTITY_MONSTER:
			speed = 0.2f;
			u = 2.f;
			health = 4.f;
			reward = 30.f;
			damage = 15.f;
			break;
		case ENTITY_ADMIN:
			speed = 1.5f;
			u = 3.f;
			health = 5.f;
			reward = 500.f;
			damage = 2213.f;
			break;
		case ENTITY_IRON_MAIDEN:
			speed = 0.01f;
			u = 4.f;
			health = 100.f;
			reward = 150.f;
			damage = 25.f;
			break;
		case ENTITY_TUNGSTEN_MAIDEN:
			speed = 0.01f;
			u = 5.f;
			health = 1500.f;
			reward = 800.f;
			damage = 99.f;
			break;
		case ENTITY_TWIN:
			speed = 0.15f;
			u = 7.f;
			health = 3.f;
			reward = 5.f;
			damage = 5.f;
			break;
		case ENTITY_GENERAL:
			speed = 0.025f;
			u = 8.f;
			health = 500.f;
			reward = 2000.f;
			damage = 509231589.f;
			break;
		}
	}
};
class Camera {
public:
	Vec3 pos{12.5f, 12.5f, 12.5f};
	float fov = 1.57f;
	Vec3 rotation{1.57f, 1.57f, 0.f};
};
float distance3D(Vec3 p1, Vec3 p2) {
	return sqrt(pow(p2.x - p1.x, 2.f) + pow(p2.y - p1.y, 2.f) + pow(p2.z - p1.z, 2.f));
}
class Level {
public:
	float pathWidth = 0.6f;
	vector<Vec3> path = {{2.000000f, 0.000000f, 0.000000f}, {5.000000f, 0.000000f, 2.000000f}, {2.000000f, 0.000000f, 8.000000f}, {4.000000f, 0.000000f, 8.000000f}, {4.000000f, 0.000000f, 3.000000f}, {1.000000f, 0.000000f, 3.000000f}, {1.000000f, 0.000000f, 9.000000f}, {6.000000f, 0.000000f, 9.000000f}, {6.000000f, 0.000000f, 3.000000f}, {9.000000f, 0.000000f, 3.000000f}, {9.000000f, 0.000000f, 10.000000f}, {14.007325f, 0.000000f, 10.099076f}, {15.940928f, 0.000000f, 10.099076f}, {17.275387f, 0.000000f, 9.935673f}, {18.446444f, 0.000000f, 9.282061f}, {18.909418f, 0.000000f, 7.974836f}, {18.065170f, 0.000000f, 7.239522f}, {16.512840f, 0.000000f, 7.076118f}, {15.341784f, 0.000000f, 6.994416f}, {14.034558f, 0.000000f, 6.967182f}, {13.054140f, 0.000000f, 6.939949f}, {13.026906f, 0.000000f, 4.053161f}, {13.026906f, 0.000000f, 1.629347f}, {15.423485f, 0.000000f, 1.493177f}, {16.676243f, 0.000000f, 1.547646f}, {17.547726f, 0.000000f, 2.174024f}, {18.228573f, 0.000000f, 2.691469f}, {18.909418f, 0.000000f, 3.236145f}, {19.753668f, 0.000000f, 3.426783f}, {20.652386f, 0.000000f, 3.372314f}, {21.796207f, 0.000000f, 3.454016f}, {22.749392f, 0.000000f, 3.454016f}, {23.593641f, 0.000000f, 3.317847f}, {24.328957f, 0.000000f, 2.882105f}, {24.546825f, 0.000000f, 2.065089f}, {24.111084f, 0.000000f, 1.275308f}, {23.185135f, 0.000000f, 0.812331f}, {22.449820f, 0.000000f, 1.084670f}, {22.286417f, 0.000000f, 1.738283f}, {22.286417f, 0.000000f, 2.364661f}, {22.313650f, 0.000000f, 3.835290f}, {22.313650f, 0.000000f, 5.659958f}, {22.177483f, 0.000000f, 10.888858f}};
};
#define PERSON_ARCHER 0
#define PERSON_CANNON 1
#define PERSON_TURRET 2
#define PERSON_TANK 3
#define PERSON_GOLD_MINE 4

#define PROJECTILE_ARROW 0
#define PROJECTILE_CANNONBALL 1
#define PROJECTILE_BULLET 2
#define PROJECTILE_MISSILE 3
#define PROJECTILE_SPARK 4
class Projectile {
	public:
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 velocity{0.f, 0.f, 0.f};
	Vec3 size{0.3f, 0.3f, 0.3f};
	int type;
	float age = 0.f;
	float damage = 1.f;
	float health = 1.f;
	float u;
	float speed = 0.1f;
	bool guided = false;
	Projectile(int tyape, Vec3 asdpos, Vec3 asdvelocity) {
		type = tyape;
		pos = asdpos;
		velocity = asdvelocity;
		switch (type) {
		case PROJECTILE_ARROW:
			damage = 1.3f;
			u = 0.f;
			speed = 1.143f;
			guided = true;
			health = 1.f;
			break;
		case PROJECTILE_CANNONBALL:
			damage = 3.5f;
			u = 1.f;
			speed = 0.6f;
			health = 3.f;
			break;
		case PROJECTILE_BULLET:
			damage = 1.5f;
			u = 2.f;
			speed = 1.3f;
			break;
		case PROJECTILE_MISSILE:
			damage = 6.f;
			u = 3.f;
			speed = 0.5f;
			health = 5.f;
			break;
		case PROJECTILE_SPARK:
			damage = 0.f;
			u = 3.f;
			speed = 0.01f;
			health = 10000000.f;
			break;
		}
	}
};
struct PersonUpgrade {
	string name;
	float price;
};
class PersonStats {
	public:
	Projectile projectile{PROJECTILE_ARROW, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
	float shootDelay = 0.f;
	float range = 5.f;
	float price;
	int level = 0;
	string levelText = "default";
	Vec3 size{1.f, 1.f, 1.f};
	PersonStats(int type) {
		switch (type) {
		case PERSON_ARCHER:
			price = 50.f;
			range = 3.f;
			shootDelay = 0.85f;
			projectile = {PROJECTILE_ARROW, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			size = {0.6f, 1.7f, 0.6f};
			break;
		case PERSON_CANNON:
			price = 100.f;
			range = 2.f;
			shootDelay = 1.5f;
			projectile = {PROJECTILE_CANNONBALL, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			size = {0.5f, 0.1f, 0.5f};
			break;
		case PERSON_TURRET:
			price = 500.f;
			range = 10.f;
			shootDelay = 0.3f;
			projectile = {PROJECTILE_BULLET, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			size = {0.7f, 0.6f, 0.7f};
			break;
		case PERSON_TANK:
			price = 1600.f;
			range = 16.f;
			shootDelay = 0.4f;
			projectile = {PROJECTILE_MISSILE, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			projectile.damage = 8.f;
			size = {3.5f, 2.5f, 3.5f};
			break;
		case PERSON_GOLD_MINE:
			price = 650.f;
			range = 3.f;
			shootDelay = 5.f;
			projectile = {PROJECTILE_MISSILE, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			projectile.damage = 10.f;
			size = {3.f, 2.5f, 3.f};
			break;
		}
	}
};
class Person {
	public:
	Vec3 pos{0.f, 0.f, 0.f};
	int type;
	PersonStats stats{PERSON_ARCHER};
	float shootDelayTimer = 0.f;
	float yRotation = 0.f;
	float u;
	bool selected = false;
	Person(int tyape, Vec3 asdpos) {
		type = tyape;
		stats = {type};
		pos = asdpos;
	}
	bool isPlacable(Level* level, vector<Person>* people) {
		if (pos.x < 0.f || pos.x > 25.f || pos.z < 0.f || pos.z > 25.f) return false;
		for (int i = 0; i < (int)level->path.size() - 1; i++) {
			if (lineCircleIntersects(level->path[i].x, level->path[i].z, level->path[i + 1].x, level->path[i + 1].z, pos.x, pos.z, (stats.size.x + stats.size.z) / 4.f + level->pathWidth / 2.f)) {
				return false;
			}
		}
		for (int i = 0; i < (int)people->size(); i++) {
			if (
				pos.x + stats.size.x / 2.f > people[0][i].pos.x - people[0][i].stats.size.x / 2.f && 
				pos.x - stats.size.x / 2.f < people[0][i].pos.x + people[0][i].stats.size.x / 2.f &&
				pos.y + stats.size.y       > people[0][i].pos.y                                   && 
				pos.y                      < people[0][i].pos.y + people[0][i].stats.size.y       &&
				pos.z + stats.size.z / 2.f > people[0][i].pos.z - people[0][i].stats.size.z / 2.f && 
				pos.z - stats.size.z / 2.f < people[0][i].pos.z + people[0][i].stats.size.z / 2.f) {
				return false;
			}
		}
		return true;
	}
	vector<PersonUpgrade> getUpgrades() {
		switch (type) {
		case PERSON_ARCHER:
			return {{"default", 0.f}, {"sharpened arrows", 100.f}, {"scope", 200.f}, {"crossbow", 500.f}};
		case PERSON_CANNON:
			return {{"default", 0.f}, {"shorter fuse", 175.f}, {"long barrel", 350.f}, {"missile launcher", 800.f}, {"op cannon", 500000.f}};
		case PERSON_TURRET:
			return {{"default", 0.f}, {"fast", 300.f}, {"mega fast", 500.f}, {"super fast", 700.f}};
		case PERSON_TANK:
			return {{"default", 0.f}, {"strong missiles", 700.f}, {"guided missiles not working", 1220.f}, {"coming soon", 70032436598263495.f}}; // move around
		case PERSON_GOLD_MINE:
			return {{"default", 0.f}, {"faster minecart", 300.f}, {"electric minecart", 420.f}, {"mega fast minecart", 587.f}};
		}
		return {{"default", 0.f}, {"level 1", 100.f}, {"level 2", 200.f}, {"level 3", 300.f}};
	}
};
Controls controls;
float roundToPlace(float x, float place) {
	return round(x / place) * place;
}
struct EntitySpawningInfo {
	int id;
	float delay;
	int count;
};
class Wave {
	public:
		vector<EntitySpawningInfo> entities;
		string message;
		Wave(string messaage, vector<EntitySpawningInfo> them) {
			entities = them;
			message = messaage;
		}
};
float frameTime = 1.f;
long long lastFrameTime = 0LL;
class GameState {
public:
	vector<Entity> entities = {};
	vector<Person> people = {};
	vector<Projectile> projectiles = {};
	Camera camera;
	Level levels[1] = {{}};
	int activeLevel = 0;
	float health = 100.f;
	float entitySpawnDelay = 0.f;
	Person placingPerson{PERSON_ARCHER, {2.f, 0.f, 2.f}};
	bool isPlacingPerson = false;
	float money = 300.f;
	bool tankUnlocked = !false;
	Wave waveCurrentlySpawning{"", {}};
	vector<Wave> waves = {
		{"", {{ENTITY_NORMAL, 0.3f, 10}}},
		{"", {{ENTITY_NORMAL, 0.25f, 15}}},
		{"", {{ENTITY_NORMAL, 0.3f, 30}}},
		{"", {{ENTITY_NORMAL, 0.2f, 15}, {ENTITY_FAST, 0.3f, 10}}},
		{"", {{ENTITY_NORMAL, 0.15f, 30}}},
		{"", {{ENTITY_NORMAL, 0.15f, 10}, {ENTITY_FAST, 0.2f, 10}, {ENTITY_NORMAL, 0.2f, 20}}},
		{"", {{ENTITY_NORMAL, 0.2f}, {ENTITY_NORMAL, .2f}, {ENTITY_NORMAL, .2f}, {ENTITY_NORMAL, .2f}, {ENTITY_NORMAL, .2f}, {ENTITY_NORMAL, .2f}, {ENTITY_NORMAL, .2f}, {ENTITY_NORMAL, .2f}}},
		{"", {{ENTITY_NORMAL, 0.1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}}},
		{"", {{ENTITY_NORMAL, 0.05f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .05f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .05f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .05f}, {ENTITY_NORMAL, .1f}}},
		{"", {{ENTITY_NORMAL, 0.1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .08f}, {ENTITY_NORMAL, .08f}, {ENTITY_NORMAL, .08f}, {ENTITY_NORMAL, .08f}, {ENTITY_NORMAL, .08f}, {ENTITY_NORMAL, .08f}}},
		{"", {{ENTITY_NORMAL, 0.1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_NORMAL, 2.f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}}},
		{"", {{ENTITY_FAST, 0.1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_FAST, 1.f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}, {ENTITY_FAST, .04f}}},
		{"", {{ENTITY_NORMAL, 0.1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f},   {ENTITY_NORMAL, .1f}}},
		{"", {{ENTITY_FAST, 0.1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}  , {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}}},
		{"monster this wave", {{ENTITY_MONSTER, 0.1f}}},
		{"", {{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f},{ENTITY_FAST, 0.05f}}},
		{"", {{ENTITY_FAST, 0.1f}, {ENTITY_FAST, 0.1f}, {ENTITY_NORMAL, 0.1f}, {ENTITY_FAST, .1f}, {ENTITY_MONSTER, 0.1f}}},
		{"", {{ENTITY_MONSTER, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}, {ENTITY_FAST, .1f}}},
		{"", {{ENTITY_MONSTER, .05f}, {ENTITY_MONSTER, .05f}, {ENTITY_MONSTER, .05f}, {ENTITY_MONSTER, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_NORMAL, .1f}, {ENTITY_FAST, .1f}}},
		{"", {{ENTITY_FAST, .5f}, {ENTITY_FAST, .5f}, {ENTITY_FAST, .5f}, {ENTITY_FAST, .5f}, {ENTITY_FAST, .5f}, {ENTITY_FAST, .5f}, {ENTITY_FAST, .5f}, {ENTITY_NORMAL, .5f}, {ENTITY_FAST, .5f}}},
		{"iron maiden has alot of hp", {{ENTITY_MONSTER, 0.01f}, {ENTITY_NORMAL, 0.1f}, {ENTITY_FAST, 0.1f}, {ENTITY_FAST, 0.1f}, {ENTITY_MONSTER, 0.01f}, {ENTITY_MONSTER, 0.01f}, {ENTITY_MONSTER, 0.01f}, {ENTITY_IRON_MAIDEN, 0.01f}}},
		{"", {{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f}}},
		{"", {{ENTITY_NORMAL, .1f},{ENTITY_FAST, .1f},{ENTITY_NORMAL, .1f},{ENTITY_FAST, .1f},{ENTITY_NORMAL, .1f},{ENTITY_FAST, .1f},{ENTITY_NORMAL, .1f},{ENTITY_NORMAL, .1f},{ENTITY_FAST, .1f},{ENTITY_NORMAL, .1f},{ENTITY_FAST, .1f},{ENTITY_NORMAL, .1f}}},
		{"", {{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, 1.f},{ENTITY_NORMAL, .05f},{ENTITY_NORMAL, .05f},{ENTITY_NORMAL, .05f},{ENTITY_NORMAL, .05f},{ENTITY_NORMAL, .05f},{ENTITY_NORMAL, .05f},{ENTITY_NORMAL, 1.f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, .05f},{ENTITY_FAST, 1.f}}},
		{"", {{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_MONSTER, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_MONSTER, .1f},{ENTITY_FAST, .1f},{ENTITY_FAST, .1f},{ENTITY_MONSTER, .1f}}},
		{"", {{ENTITY_IRON_MAIDEN, 2.f},{ENTITY_IRON_MAIDEN, 2.f},{ENTITY_IRON_MAIDEN, 2.f}}},
		{"boss round", {{ENTITY_GENERAL, 2.f}}},
		{"twins drop 2 fasts", {{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f}}},
		{"", {{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f}}},
		{"", {{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f}}},
		{"", {{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_IRON_MAIDEN, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_IRON_MAIDEN, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f}}},
		{"", {{ENTITY_TWIN, 0.05f},{ENTITY_TWIN, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f}}},
		{"tungsten maiden this round", {{ENTITY_TUNGSTEN_MAIDEN, 0.1f}}},
		{"", {{ENTITY_NORMAL, 0.1f},{ENTITY_NORMAL, 0.1f},{ENTITY_NORMAL, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_MONSTER, 0.1f},{ENTITY_MONSTER, 0.1f},{ENTITY_MONSTER, 0.1f},{ENTITY_IRON_MAIDEN, 0.1f},{ENTITY_IRON_MAIDEN, 0.1f},{ENTITY_IRON_MAIDEN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f}}},
		{"admin this round", {{ENTITY_ADMIN, 0.1f}}},
		{"FINAL BOSS", {{ENTITY_NORMAL, 0.1f}}}
	};
	int waveNumber = 0;
	float d = 1.0;
	string message;
	float messageTime;
	GameState() {

	}
	
	void tick(int width, int height) {
		if (controls.w) camera.pos.z -= 0.1f * d;
		if (controls.s) camera.pos.z += 0.1f * d;
		if (controls.up) camera.pos.y += 0.1f * d;
		if (controls.down) camera.pos.y -= 0.1f * d;
		if (controls.left) camera.pos.x -= 0.1f * d;
		if (controls.right)  camera.pos.x += 0.1f * d;
		if (camera.pos.y < 0.1f) {
			camera.pos.y = 0.1f;
		}
		controls.previousClipMouse = controls.clipMouse;
		controls.clipMouse.x = (controls.mouse.x / (float)width - 0.5f) * 2.f;
		controls.clipMouse.y = (0.5f - controls.mouse.y / (float)height) * 2.f;
		controls.worldMouse.x = controls.clipMouse.x * (camera.pos.z) * (float)width / (float)height + camera.pos.x;
		controls.worldMouse.y = controls.clipMouse.y * (camera.pos.z) + camera.pos.y;

		messageTime -= d / 60.f;

		if (controls.clipMouse.x < -0.7f && controls.previousClipMouse.x >= -0.7f) {
			isPlacingPerson = false;
		}

		if (entitySpawnDelay <= 0.f && waveCurrentlySpawning.entities.size() > 0) {
			entities.push_back({waveCurrentlySpawning.entities[0].id, levels[activeLevel].path[0]});
			entitySpawnDelay = waveCurrentlySpawning.entities[0].delay;
			waveCurrentlySpawning.entities[0].count--;
			if (waveCurrentlySpawning.entities[0].count <= 0) {
				waveCurrentlySpawning.entities.erase(waveCurrentlySpawning.entities.begin());
			}
		} else {
			entitySpawnDelay -= d / 60.f;
		};
		if (isPlacingPerson) {
			placingPerson.pos.x = controls.worldMouse.y;
			placingPerson.pos.z = controls.worldMouse.x;
		}

		for (int i = 0; i < (int)people.size(); i++) {
			if (people[i].shootDelayTimer <= 0.f) {
				if (!waveEnded() && (entities.size() > 0 || people[i].type == PERSON_GOLD_MINE)) {
					people[i].shootDelayTimer = people[i].stats.shootDelay;
					if (people[i].type == PERSON_GOLD_MINE) {
						money += people[i].stats.projectile.damage;
					} else {
						int closestEntityIndex = getClosestEntity(people[i].pos, people[i].stats.range);
						if (closestEntityIndex != -1) {
							people[i].yRotation = atan2(entities[closestEntityIndex].pos.z - people[i].pos.z, entities[closestEntityIndex].pos.x - people[i].pos.x);
							projectiles.push_back(people[i].stats.projectile);
							Vec3 projectileVel = vec3Subtract(entities[closestEntityIndex].pos, people[i].pos).normalise(projectiles[projectiles.size() - 1].speed);
							projectiles[projectiles.size() - 1].pos = people[i].pos;
							projectiles[projectiles.size() - 1].pos.y += 1.f;
							projectiles[projectiles.size() - 1].velocity = projectileVel;
							projectiles.push_back({PROJECTILE_SPARK, vec3Add(people[i].pos, {0.f, 1.f, 0.f}), projectileVel.normalise(0.01f)});
						}
					}
				}
			} else {
				people[i].shootDelayTimer -= d / 60.f;
			}
		}
		for (int i = (int)projectiles.size() - 1; i >= 0; i--) {
			if (projectiles[i].guided) {
				int closestEntityIndex = getClosestEntity(projectiles[i].pos, 10.f);
				if (closestEntityIndex != -1) projectiles[i].velocity = vec3Subtract(entities[closestEntityIndex].pos, projectiles[i].pos).normalise(projectiles[i].speed);
			}
			projectiles[i].pos = vec3Add(projectiles[i].pos, vec3Mul(projectiles[i].velocity, d));
			projectiles[i].age += d / 60.f;
			if (projectiles[i].age > 5.f || (projectiles[i].type == PROJECTILE_SPARK && projectiles[i].age > 0.05f)) {
				projectiles.erase(projectiles.begin() + i);
				continue;
			}
			for (int j = 0; j < (int)entities.size(); j++) {
				if (
					projectiles[i].pos.x - projectiles[i].size.x / 2.f < entities[j].pos.x + entities[j].size.x / 2.f &&
					projectiles[i].pos.x + projectiles[i].size.x / 2.f > entities[j].pos.x - entities[j].size.x / 2.f &&
					projectiles[i].pos.y                               < entities[j].pos.y + entities[j].size.y       &&
					projectiles[i].pos.y + projectiles[i].size.y       > entities[j].pos.y                            &&
					projectiles[i].pos.z - projectiles[i].size.z / 2.f < entities[j].pos.z + entities[j].size.z / 2.f &&
					projectiles[i].pos.z + projectiles[i].size.z / 2.f > entities[j].pos.z - entities[j].size.z / 2.f
				) {
					projectiles[i].health -= 1.f;
					entities[j].health -= projectiles[i].damage;
					if (entities[j].health <= 0.f) {
						money += entities[j].reward;
						if (entities[j].type == ENTITY_TUNGSTEN_MAIDEN) {
							for (int k = 0; k < 5; k++) {
								entities.push_back({ENTITY_TUNGSTEN_MAIDEN, {entities[j].pos.x + randFloat() * 1.f - 0.5f, entities[j].pos.y, entities[j].pos.z + randFloat() * 1.f - 0.5f}});
								entities[entities.size() - 1].targetPoint = entities[j].targetPoint;
							}
						} else if (entities[j].type == ENTITY_TWIN) {
							for (int k = 0; k < 2; k++) {
								entities.push_back({ENTITY_FAST, {entities[j].pos.x + randFloat() * 0.4f - 0.2f, entities[j].pos.y, entities[j].pos.z + randFloat() * 0.4f - 0.2f}});
								entities[entities.size() - 1].targetPoint = entities[j].targetPoint;
							}
						} else if (entities[j].type == ENTITY_GENERAL) tankUnlocked = true;
						if (getWave(waveNumber).message.length() > 0 && entities.size() == 1 && waveCurrentlySpawning.entities.size() == 0) {
							showMessage(getWave(waveNumber).message, 1.5f);
						}
					};
					if (projectiles[i].health <= 0.f) {
						projectiles.erase(projectiles.begin() + i);
					}
					break;
				}
			}
		}
		for (int i = (int)entities.size() - 1; i >= 0; i--) {
			bool die = entities[i].health <= 0.f;
			Vec3 targetPoint = levels[activeLevel].path[entities[i].targetPoint];
			float distanceToTargetPoint = distance3D(entities[i].pos, targetPoint);
			if (distanceToTargetPoint < entities[i].speed * d) {
				entities[i].targetPoint++;
				if (entities[i].targetPoint >= (int)levels[activeLevel].path.size()) {
					health -= entities[i].damage;
					die = true;
				}
				targetPoint = levels[activeLevel].path[entities[i].targetPoint];
				distanceToTargetPoint = distance3D(entities[i].pos, targetPoint);
			}
			float xDelta = (targetPoint.x - entities[i].pos.x) / distanceToTargetPoint * entities[i].speed;
			float zDelta = (targetPoint.z - entities[i].pos.z) / distanceToTargetPoint * entities[i].speed;
			float rotation = atan2(zDelta, xDelta);
			if (abs(entities[i].yRotation - rotation) > 0.1f * d) {
				if (entities[i].yRotation < rotation) {
					entities[i].yRotation += 0.1f * d;
				} else if (entities[i].yRotation > rotation) {
					entities[i].yRotation -= 0.1f * d;
				}
			} else {
				entities[i].yRotation = rotation;
			};
			entities[i].pos.x += xDelta * d;
			entities[i].pos.y += (targetPoint.y - entities[i].pos.y) / distanceToTargetPoint * entities[i].speed * d;
			entities[i].pos.z += zDelta * d;
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
	void spawnProjectile(int type, float x, float z, Vec3 vel) {
		Vec3 randomVel{randFloat() * 0.1f - 0.05f, 0.f, randFloat() * 0.1f - 0.05f};
		projectiles.push_back({type, {x, 2.f, z}, vec3Add(vel, randomVel)});
	}
	void spawnWave(Wave wave) {
		waveCurrentlySpawning = wave;
	}
	bool waveEnded() {
		return waveCurrentlySpawning.entities.size() == 0 && entities.size() == 0;
	}
	void showMessage(string text, float time) {
		message = text;
		messageTime = time;
	}
	int getClosestEntity(Vec3 pos, float searchRadius) {
		int nearestIndex = -1;
		float nearestDist;
		for (int i = 0; i < (int)entities.size(); i++) {
			float currentDistance = distance3D(entities[i].pos, pos);
			if ((nearestIndex == -1 || currentDistance < nearestDist) && currentDistance <= searchRadius) {
				nearestIndex = i;
				nearestDist = currentDistance;
			}
		}
		return nearestIndex;
	}
	Wave getWave(int number) {
		if (number < (int)waves.size()) {
			return waves[number];
		} else {
			Wave wave = {"", {}};
			for (int i = 0; i < number; i++) {
				if (number < 100) {
					wave.entities.push_back({i % 3 == 0 ? ENTITY_MONSTER : (i % 3 == 1 ? ENTITY_TWIN : ENTITY_IRON_MAIDEN), 0.05f});
				} else {
					wave.entities.push_back({ENTITY_GENERAL, 0.05f});
				}
			}
			return wave;
		}
	}
	void upgradePerson(Person* person) {
		vector<PersonUpgrade> upgrades = person->getUpgrades();
		if (person->stats.level > (int)upgrades.size() - 1) return;
		if (money >= upgrades[person->stats.level + 1].price) {
			money -= upgrades[person->stats.level + 1].price;
			person->stats.level++;
			switch (person->type) {
			case PERSON_ARCHER://sharpened arrows: go through many, scope: more range, crossbow: faster and damage
				if (person->stats.level == 1) {
					person->stats.projectile.health = 5.f;
					person->stats.projectile.damage *= 2.f;
				} else if (person->stats.level == 2) {
					person->stats.range *= 2.f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay = 0.4f;
					person->stats.projectile.damage = 2.f;
					person->stats.range *= 1.1f;
				}
				break;
			case PERSON_CANNON: // shorter fuse: faster, long barrel: faster cannonballs more range, missile launcher: missiles
				if (person->stats.level == 1) {
					person->stats.shootDelay = 1.1f;
				} else if (person->stats.level == 2) {
					person->stats.range *= 1.3f;
					person->stats.shootDelay *= 0.8f;
				} else if (person->stats.level == 3) {
					person->stats.range *= 2.f;
					person->stats.projectile = {PROJECTILE_MISSILE, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
				} else if (person->stats.level == 4) {
					person->stats.shootDelay = 0.0f;
				}
				break;
			case PERSON_TURRET: // mega fast: fast
				if (person->stats.level == 1) {
					person->stats.shootDelay *= 0.8f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.8f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.8f;
				}
				break;
			case PERSON_TANK: // them
				if (person->stats.level == 1) {
					person->stats.projectile.damage *= 1.3;
				} else if (person->stats.level == 2) {
					person->stats.projectile.guided = true;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.008f;
				}
				break;
			case PERSON_GOLD_MINE: // them
				if (person->stats.level == 1) {
					person->stats.shootDelay *= 0.8f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.8f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.8f;
				}
				break;
			}
		}
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
string getBeforeChar(string s, char c) {
	string::size_type pos = s.find(c);
	if (pos != string::npos) return s.substr(0, pos);
	else return s;
}
class objFile {
	public:
		vector<Vec3> vertices = {};
		vector<Vec2> texcoords = {};
		vector<vector<unsigned int>> faces = {};
		objFile(string path) {
			// load file
			ifstream file{path};
			if (!file.is_open()) {
				cerr << "ERROR: obj file \"" << path << "\" not found" << endl;
				file.close();
				return;
			}

			// loop over every line
			string lineText;
			while (file) {
				getline(file, lineText);
				lineText += ' ';
				// vertex
				if (lineText.rfind("v ", 0) == 0) {
					Vec3 vertex = {0.f, 0.f, 0.f};
					string word = "";
					int wordNumber = 0;
					for (char c : lineText) {
						if (c == ' ') {
							if (wordNumber == 1) vertex.x = stof(word);
							else if (wordNumber == 2) vertex.y = stof(word);
							else if (wordNumber == 3) vertex.z = stof(word);
							word = "";
							wordNumber++;
						} else word += c;
					}
					vertices.push_back(vertex);
				} else if (lineText.rfind("vt ", 0) == 0) { // texcoord
					Vec2 texcoord = {0.f, 0.f};
					string word = "";
					int wordNumber = 0;
					for (char c : lineText) {
						if (c == ' ') {
							if (wordNumber == 1) texcoord.x = stof(word);
							else if (wordNumber == 2) texcoord.y = stof(word);
							word = "";
							wordNumber++;
						} else word += c;
					}
					texcoords.push_back(texcoord);
				} else if (lineText.rfind("f ", 0) == 0) { // texcoord
					vector<unsigned int> face = {};
					string word = "";
					int wordNumber = 0;
					for (char c : lineText) {
						if (c == ' ') {
							if (wordNumber != 0) face.push_back((unsigned int)stoul(getBeforeChar(word, '/')) - 1U);
							word = "";
							wordNumber++;
						} else word += c;
					}
					faces.push_back(face);
				}
			}
			file.close();
		}
};
class Mesh {
	public:
		vector<Vertex> vertices = {};
		vector<unsigned int> indices = {};
		Mesh(vector<Vertex> avertices, vector<unsigned int> aindices) {
			vertices = avertices;
			indices = aindices;
		}
		Mesh(string path) {
			objFile obj{path};
			for (int i = 0; i < (int)obj.vertices.size(); i++) {
				vertices.push_back({obj.vertices[i].x, obj.vertices[i].y, obj.vertices[i].z, obj.texcoords[i].x, obj.texcoords[i].y, 1.f});
			}
			for (int i = 0; i < (int)obj.faces.size(); i++) {
				indices.insert(indices.end(), obj.faces[i].begin(), obj.faces[i].end());
			}
		}
};
struct personButton {
	int type;
	float u;
	float v;
};
vector<personButton> personButtons = {
	{PERSON_ARCHER, 0.f, 2.f},
	{PERSON_CANNON, 1.f, 2.f},
	{PERSON_TURRET, 2.f, 2.f},
	{PERSON_TANK, 3.f, 2.f},
	{PERSON_GOLD_MINE, 4.f, 2.f}
};
class GameStateVertexBuilder {
public:
	Mesh turretMesh{"resources/model/person/turret.obj"};
	Mesh cannonMesh{"resources/model/person/cannon.obj"};
	Mesh archerMesh{"resources/model/person/archer.obj"};
	Mesh tankMesh{"resources/model/person/tank.obj"};
	Mesh goldMineMesh{"resources/model/person/goldmine.obj"};

	Mesh normalMesh{"resources/model/entity/normal.obj"};
	Mesh fastMesh{"resources/model/entity/fast.obj"};
	Mesh monsterMesh{"resources/model/entity/monster.obj"};
	Mesh generalMesh{"resources/model/entity/general.obj"};
	Mesh ironMaidenMesh{"resources/model/entity/ironmaiden.obj"};

	Mesh missileMesh{"resources/model/projectile/missile.obj"};

	Mesh planeMesh{"resources/model/plane.obj"};

	GameState* game;
	GameStateVertexBuilder(GameState* gam) {
		game = gam;
	}
	void buildThem(int width, int height) {
		clearVertices();
		// ground
		for (int x = 0; x < 25; x++) {
			for (int z = 0; z < 25; z++) {
				addPlane((float)x, 0.f, (float)z, 1.f, 1.f, 0.f, 0.f);
			}
		}
		// point path
		for (int i = 0; i < (int)game->levels[game->activeLevel].path.size() - 1; i++) {
			addPath(game->levels[game->activeLevel].path[i], game->levels[game->activeLevel].path[i + 1], 1.f, 0.f, game->levels[game->activeLevel].pathWidth);
		}
		// entities
		for (Entity entity : game->entities) {
			switch (entity.type) {
			case ENTITY_NORMAL:
				addMesh(normalMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation, {0.f, 1.f});
				break;
			case ENTITY_FAST:
				addMesh(fastMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation, {1.f, 1.f});
				break;
			case ENTITY_MONSTER:
				addMesh(monsterMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation, {2.f, 1.f});
				break;
			case ENTITY_GENERAL:
				addMesh(generalMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation, {3.f, 8.f});
				break;
			case ENTITY_IRON_MAIDEN:
				addMesh(ironMaidenMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation, {4.f, 1.f});
				break;
			default:
				addCube(entity.pos.x - entity.size.x / 2.f, entity.pos.y, entity.pos.z - entity.size.z / 2.f, entity.size.x, entity.size.y, entity.size.z, entity.u, 1.f);
				break;
			}
		}
		//people
		for (Person person : game->people) {
			addPerson(&person, false);
		}
		if (game->isPlacingPerson) {
			addPerson(&game->placingPerson, true);
		}
		//projectiles
		for (Projectile projectile : game->projectiles) {
			switch (projectile.type) {
			case PROJECTILE_MISSILE:
				addMesh(missileMesh, projectile.pos, {1.f, 1.f, 1.f}, atan2(projectile.velocity.z, projectile.velocity.x), {3.f, 6.f});
				break;
			case PROJECTILE_SPARK:
				addMesh(planeMesh, vec3Add(projectile.pos, projectile.velocity.normalise(0.7f)), {.3f, .3f, .3f}, atan2(projectile.velocity.z, projectile.velocity.x), {6.f, 7.f});
				break;
			default:
				addPath(projectile.pos, vec3Add(projectile.pos, projectile.velocity), projectile.u, 6.f, 0.1f);
			}
		}


		// clip space: smaller z is in front

		addText("HEALTH: " + to_string((int)game->health), -0.67f, 0.88f, 0.1f, 0.07f, 0.8f);
		addText("WAVE " + to_string(game->waveNumber), -0.3f, -0.97f, 0.1f, 0.07f, 0.8f);
		addText("$" + to_string((long long)game->money), -0.1f, 0.75f, 0.1f, 0.07f, 0.8f);
		addText("fps:" + to_string(1.f / (float)(frameTime)), -0.1f, 0.65f, 0.1f, 0.07f, 0.8f);
		/*if (game->waveNumber == game->waves.size() && game->waveEnded() && game->health > 0.f) {
			addText("YOU WON", -0.9f, -0.9f, 0.1f, 0.2f, 0.8f);
		}*/
		if (game->waveEnded()) {
			addRect(0.7f, -0.95f, 0.2f, 0.25f, 0.25f, 0.f, 7.f);
		} else {
			addRect(0.7f, -0.95f, 0.2f, 0.25f, 0.25f, 1.f, 7.f);
		}
		for (Person person : game->people) {
			if (person.selected) {
				addRect(-0.65f, 0.55f, 0.2f, 0.2f, 0.2f, (person.stats.level <= (int)person.getUpgrades().size() - 1 && game->money >= person.getUpgrades()[person.stats.level + 1].price) ? 7.f : 8.f, 7.f);
				vector<PersonUpgrade> upgrades = person.getUpgrades();
				if (person.stats.level > (int)upgrades.size() - 2) {
					addText("max upgrades", -0.43f, 0.6f, 0.1f, 0.04f, 0.8f);
				} else {
					addText(upgrades[person.stats.level + 1].name + ": $" + to_string((long long)upgrades[person.stats.level + 1].price), -0.43f, 0.6f, 0.1f, 0.04f, 0.8f);
				}
				addText("dps: " + to_string(person.stats.projectile.damage / person.stats.shootDelay), -0.65f, 0.38f, 0.1f, 0.04f, 0.8f);
				break;
			}
		}
		addRect(-1.f, -1.f, 0.25f, 0.3f, 2.f, 2.f, 7.f); // eople
		for (int i = 0; i < (int)personButtons.size(); i++) {
			float x = -0.98f + (0.15f * fmod(i, 2.f));
			float y = 0.7f - floor(i / 2.f) * 0.15f;
			addRect(x, y, 0.205f, 0.13f, 0.13f, personButtons[i].u, personButtons[i].v);
			addText("$" + to_string((long long)(PersonStats){personButtons[i].type}.price), x, y - 0.02f, 0.1f, 0.03f, 0.7f);
		}
		//if (game->tankUnlocked) addRect(-0.83f, 0.55f, 0.205f, 0.13f, 0.13f, 3.f, 2.f);
		if (game->messageTime > 0.f) {
			addRect(-0.5f, -0.9f, 0.2f, 1.f, 0.5f, 2.f, 7.f);
			addText(game->message, -0.45f, -0.6f, 0.1f, 0.07f, 0.7f);
		}
	}
private:
	void clearVertices() {
		vertices.clear();
		indices.clear();
	}

	//////////////////////////////
	//// world space /////////////
	//////////////////////////////
	void addPerson(Person* person, bool isPlacingPerson) {
		switch (person->type) {
		case PERSON_ARCHER:
			addMesh(archerMesh, person->pos, {.5f, .7f, .5f}, person->yRotation, {0.f, 8.f});
			break;
		case PERSON_CANNON:
			addMesh(cannonMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation, {1.f, 8.f});
			break;
		case PERSON_TURRET:
			addMesh(turretMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation, {2.f, 8.f});
			break;
		case PERSON_TANK:
			addMesh(tankMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation, {3.f, 8.f});
			break;
		case PERSON_GOLD_MINE:
			addMesh(goldMineMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation, {4.f, 8.f});
			break;
		default:
			addCube(person->pos.x - person->stats.size.x / 2.f, person->pos.y, person->pos.z - person->stats.size.z / 2.f, person->stats.size.x, person->stats.size.y, person->stats.size.z, person->u, 2.f);
			break;
		}
		if (isPlacingPerson) {
			if (person->isPlacable(&game->levels[game->activeLevel], &game->people)) {
				addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, 4.f, 7.f);
			} else {
				addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, 5.f, 7.f);

			}
		} else if (person->selected) {
			addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, 3.f, 7.f);
		}
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
	void addPath(Vec3 p1, Vec3 p2, float u, float v, float width) {
		unsigned int end = vertices.size();
		float dist = distance3D(p1, p2);
		float xDiff = (p2.x - p1.x) / dist * width / 2.f;
		float zDiff = (p2.z - p1.z) / dist * width / 2.f;
		vertices.insert(vertices.end(), {
			{p1.x - zDiff - xDiff, p1.y + 0.01f, p1.z + xDiff - zDiff, u      , v + 1.f, 1.f},
			{p1.x + zDiff - xDiff, p1.y + 0.01f, p1.z - xDiff - zDiff, u + 1.f, v + 1.f, 1.f},
			{p2.x - zDiff + xDiff, p2.y + 0.02f, p2.z + xDiff + zDiff, u      , v      , 1.f},
			{p2.x + zDiff + xDiff, p2.y + 0.02f, p2.z - xDiff + zDiff, u + 1.f, v      , 1.f}
		});
		indices.insert(indices.end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
	}
	void addMesh(Mesh mesh, Vec3 position, Vec3 scale, float yRotation, Vec2 uvOffset) {
		unsigned int end = vertices.size();
		for (int i = 0; i < (int)mesh.vertices.size(); i++) {
			if (yRotation != 0.f) {
				float x = mesh.vertices[i].x;
				float z = mesh.vertices[i].z;
				mesh.vertices[i].x = x * cos(yRotation) - z * sin(yRotation);
				mesh.vertices[i].z = z * cos(yRotation) + x * sin(yRotation);
			}
			mesh.vertices[i].x *= scale.x;
			mesh.vertices[i].y *= scale.y;
			mesh.vertices[i].z *= scale.z;
			mesh.vertices[i].x += position.x;
			mesh.vertices[i].y += position.y;
			mesh.vertices[i].z += position.z;
			mesh.vertices[i].u += uvOffset.x;
			mesh.vertices[i].v += uvOffset.y;
		}
		for (int i = 0; i < (int)mesh.indices.size(); i++) {
			mesh.indices[i] += end;
		}
		vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
	}

	//////////////////////////////
	//// clip space //////////////
	//////////////////////////////
	
	void addRect(float x, float y, float z, float w, float h, float u, float v) {
		unsigned int end = vertices.size();
		indices.insert(indices.end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y+h, z, u    , v    , 0.f},
			{x+w, y+h, z, u+1.f, v    , 0.f},
			{x  , y  , z, u    , v+1.f, 0.f},
			{x+w, y  , z, u+1.f, v+1.f, 0.f}
		});
	}
	void addCharacter(char character, float x, float y, float z, float w) {
		float u = getCharacterCoords(character).x;
		float v = getCharacterCoords(character).y;
		addRect(x, y, z, w, w, u, v);
	}
	void addText(string text, float x, float y, float z, float w, float spacing) {
		for (int i = 0; i < (int)text.length(); i++) {
			addCharacter(text.at(i), x + (float)i * w * spacing, y, z, w*1.5);
			z -= 0.001;
		}
	}
};
GameState game;
GameStateVertexBuilder vBuilder{&game};
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
		/*else if (key == GLFW_KEY_K) {
			game.levels[game.activeLevel].path.push_back({controls.worldMouse.y, 0.f, controls.worldMouse.x});
		} else if (key == GLFW_KEY_L) {
			cout << "{";
			for (Vec3 point : game.levels[game.activeLevel].path) {
				cout << "{" << to_string(point.x) << "f, " << to_string(point.y) << "f, " << to_string(point.z) << "f}, ";
			}
			cout << "}" << endl;
		};*/
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
		if (controls.clipMouse.x > 0.7f && controls.clipMouse.y < -0.7f && controls.clipMouse.x < 0.95f && controls.clipMouse.y > -0.95f && game.waveEnded()) {
			game.spawnWave(game.getWave(game.waveNumber));
			game.waveNumber++;
			return;
		}
		if (controls.clipMouse.x > -0.65f && controls.clipMouse.y < 0.8f && controls.clipMouse.x < -0.4f && controls.clipMouse.y > 0.55f) {
			for (int i = 0; i < (int)game.people.size(); i++) {
				if (game.people[i].selected) {
					game.upgradePerson(&game.people[i]);
					break;
				}
			}
			return;
		}
		//PEOPLE PLACNG BUTTONS
		for (int i = 0; i < (int)personButtons.size(); i++) {
			float rectLeft = -0.98f + (0.15f * fmod(i, 2.f));
			float rectRight = rectLeft + 0.13f;
			float rectBottom = 0.7f - floor(i / 2.f) * 0.15f;
			float rectTop = rectBottom + 0.13f;
			if (controls.clipMouse.x > rectLeft && controls.clipMouse.x < rectRight && controls.clipMouse.y > rectBottom && controls.clipMouse.y < rectTop) {
				game.isPlacingPerson = true;
				game.placingPerson = {personButtons[i].type, {2.f, 0.f, 2.f}};
				if (game.money < game.placingPerson.stats.price) {
					game.isPlacingPerson = false;
				}
			}
		}

		if (game.isPlacingPerson && game.money >= game.placingPerson.stats.price && game.placingPerson.isPlacable(&game.levels[game.activeLevel], &game.people) && controls.clipMouse.x > -0.7f) {
			game.people.push_back(game.placingPerson);
			game.money -= game.placingPerson.stats.price;
			game.isPlacingPerson = false;
		};
		bool personSelected = false;
		for (int i = 0; i < (int)game.people.size(); i++) {
			game.people[i].selected = !personSelected && (controls.worldMouse.x > game.people[i].pos.z - game.people[i].stats.size.z / 2.f && controls.worldMouse.x < game.people[i].pos.z + game.people[i].stats.size.z / 2.f && controls.worldMouse.y > game.people[i].pos.x - game.people[i].stats.size.x / 2.f && controls.worldMouse.y < game.people[i].pos.x + game.people[i].stats.size.x / 2.f);
			if (game.people[i].selected) personSelected = true;
		}

		controls.mouseDown = true;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
       double xpos, ypos;
       //getting cursor position
       glfwGetCursorPos(window, &xpos, &ypos);
	   controls.mouseDown = false;
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	double xpos, ypos;
    	//getting cursor position
    	glfwGetCursorPos(window, &xpos, &ypos);

		game.isPlacingPerson = false;

		controls.mouseDown = true;
    }
}
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	controls.mouse = {(float)xpos, (float)ypos};
}

static void error_callback(int error, const char* description) {
	fprintf(stderr, "ERROR: %s\n", description);
}

int main(void) {
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vtexcoord_location, vspace_location, texture1_location;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Human Tower Defense", NULL, NULL);
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
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	while (!glfwWindowShouldClose(window)) {
		long long newFrameTime = (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch())).count();
		frameTime = (float)(newFrameTime - lastFrameTime) / 1000.f;
		lastFrameTime = newFrameTime;
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		game.d = 0.2 * (frameTime / (1.f / 60.f));
		for (int i = 0; i < 5; i++) {
			game.tick(width, height);
		}
		vBuilder.buildThem(width, height);
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
