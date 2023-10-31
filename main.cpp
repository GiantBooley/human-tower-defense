#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <linmath.h>
#include <AudioFile.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;


GLFWwindow* window;

struct Vertex {
	float x, y, z;
	float u, v;
	float space;
};

float randFloat() {
	return static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
}
float lerp(float a, float b, float t) {
	return (b - a) * t + a;
}

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
	   bool fast = false;
	   bool slow = false;
	   bool shift = false;
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
			speed = 1.f;
			u = 3.f;
			health = 5.f;
			reward = 700.f;
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
			speed = 0.03f;
			u = 8.f;
			health = 700.f;
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
string ftos(float f, int places) {
	stringstream stream;
	stream << fixed << setprecision(places) << f;
	return stream.str();
}
string commas(float f) {
	string it = ftos(f, 0);
	string otherit = "";
	for (int i = 0; i < (int)it.size(); i++) {
		otherit += it.at(i);
		if (((int)it.size() - i - 1) % 3 == 0 && (int)it.size() - i - 1 != 0) {
			otherit += ",";
		}
	}
	return otherit;
}
class Level {
public:
	int width = 25;
	int height = 25;
	float pathWidth = 0.6f;
	vector<Vec3> path = {{2.000000f, 0.000000f, 0.000000f}, {5.000000f, 0.000000f, 2.000000f}, {2.000000f, 0.000000f, 8.000000f}, {4.000000f, 0.000000f, 8.000000f}, {4.000000f, 0.000000f, 3.000000f}, {1.000000f, 0.000000f, 3.000000f}, {1.000000f, 0.000000f, 9.000000f}, {6.000000f, 0.000000f, 9.000000f}, {6.000000f, 0.000000f, 3.000000f}, {9.000000f, 0.000000f, 3.000000f}, {9.000000f, 0.000000f, 10.000000f}, {14.007325f, 0.000000f, 10.099076f}, {15.940928f, 0.000000f, 10.099076f}, {17.275387f, 0.000000f, 9.935673f}, {18.446444f, 0.000000f, 9.282061f}, {18.909418f, 0.000000f, 7.974836f}, {18.065170f, 0.000000f, 7.239522f}, {16.512840f, 0.000000f, 7.076118f}, {15.341784f, 0.000000f, 6.994416f}, {14.034558f, 0.000000f, 6.967182f}, {13.054140f, 0.000000f, 6.939949f}, {13.026906f, 0.000000f, 4.053161f}, {13.026906f, 0.000000f, 1.629347f}, {15.423485f, 0.000000f, 1.493177f}, {16.676243f, 0.000000f, 1.547646f}, {17.547726f, 0.000000f, 2.174024f}, {18.228573f, 0.000000f, 2.691469f}, {18.909418f, 0.000000f, 3.236145f}, {19.753668f, 0.000000f, 3.426783f}, {20.652386f, 0.000000f, 3.372314f}, {21.796207f, 0.000000f, 3.454016f}, {22.749392f, 0.000000f, 3.454016f}, {23.593641f, 0.000000f, 3.317847f}, {24.328957f, 0.000000f, 2.882105f}, {24.546825f, 0.000000f, 2.065089f}, {24.111084f, 0.000000f, 1.275308f}, {23.185135f, 0.000000f, 0.812331f}, {22.449820f, 0.000000f, 1.084670f}, {22.286417f, 0.000000f, 1.738283f}, {22.286417f, 0.000000f, 2.364661f}, {22.313650f, 0.000000f, 3.835290f}, {22.313650f, 0.000000f, 5.659958f}, {22.177483f, 0.000000f, 10.888858f}};
};
#define PERSON_ARCHER 0
#define PERSON_CANNON 1
#define PERSON_TURRET 2
#define PERSON_TANK 3
#define PERSON_GOLD_MINE 4
#define PERSON_BATTERY 5

#define PROJECTILE_ARROW 0
#define PROJECTILE_CANNONBALL 1
#define PROJECTILE_BULLET 2
#define PROJECTILE_MISSILE 3
#define PROJECTILE_SPARK 4
#define PROJECTILE_ELECTRICITY 5
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
		case PROJECTILE_ELECTRICITY:
			damage = 0.5f;
			u = 3.f;
			health = 3.f;
			guided = true;
			speed = 1.5f;
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
			shootDelay = 0.5f;
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
			price = 600.f;
			range = 3.f;
			shootDelay = 5.f;
			projectile = {PROJECTILE_MISSILE, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			projectile.damage = 10.f;
			size = {3.f, 2.5f, 3.f};
			break;
		case PERSON_BATTERY:
			price = 750.f;
			range = 3.f;
			shootDelay = 0.1f;
			projectile = {PROJECTILE_ELECTRICITY, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			size = {0.8f, 2.2f, 0.8f};
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
		if (type == PERSON_GOLD_MINE) yRotation = 3.141359f;
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
			return {{"default", 0.f}, {"fast", 300.f}, {"extra pierce", 500.f}, {"powerful bullets", 700.f}};
		case PERSON_TANK:
			return {{"default", 0.f}, {"strong missiles", 700.f}, {"guided missiles not working", 1220.f}, {"coming soon", 70032436598263495.f}}; // move around
		case PERSON_GOLD_MINE:
			return {{"default", 0.f}, {"faster minecart", 300.f}, {"electric minecart", 420.f}, {"mega fast minecart", 587.f}};
		case PERSON_BATTERY:
			return {{"default", 0.f}, {"more volts", 300.f}, {"li ion", 450.f}, {"op", 100100.f}};
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
float frameTime = 0.0f;
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
	float money = 301231231230.f;
	bool tankUnlocked = !false;
	Wave waveCurrentlySpawning{"", {}};
	vector<Wave> waves = {
		{"", {{ENTITY_NORMAL, 0.3f, 10}}},
		{"", {{ENTITY_NORMAL, 0.25f, 15}}},
		{"", {{ENTITY_NORMAL, 0.3f, 30}}},
		{"", {{ENTITY_NORMAL, 0.2f, 15}, {ENTITY_FAST, 0.3f, 10}}},
		{"", {{ENTITY_NORMAL, 0.15f, 30}}},
		{"", {{ENTITY_NORMAL, 0.15f, 10}, {ENTITY_FAST, 0.2f, 6}, {ENTITY_NORMAL, 0.2f, 20}}},
		{"", {{ENTITY_FAST, 0.15f, 10}, {ENTITY_NORMAL, 0.2f, 6}, {ENTITY_FAST, 0.2f, 20}}},
		{"", {{ENTITY_FAST, 0.1f, 8}, {ENTITY_NORMAL, 0.2f, 30}}},
		{"", {{ENTITY_NORMAL, 0.1f, 5}, {ENTITY_FAST, 0.2f, 6}, {ENTITY_NORMAL, 0.1f, 5}, {ENTITY_FAST, 0.2f, 6}, {ENTITY_NORMAL, 0.1f, 5}, {ENTITY_FAST, 0.2f, 6}}},
		{"monsters this wave", {{ENTITY_MONSTER, 1.f, 3}}},
		{"", {{ENTITY_FAST, 0.1f, 40}}},
		{"", {{ENTITY_NORMAL, 0.2f, 20}, {ENTITY_MONSTER, 0.5f, 2}, {ENTITY_FAST, 0.1f, 10}}},
		{"", {{ENTITY_FAST, 0.5f, 100}, {ENTITY_NORMAL, 0.1f, 5}}},
		{"iron maiden has alot of hp", {{ENTITY_FAST, 0.5f, 10}, {ENTITY_MONSTER, 0.5f, 10}, {ENTITY_NORMAL, 0.15f, 10}, {ENTITY_IRON_MAIDEN, 0.5f, 1}}},
		{"", {{ENTITY_NORMAL, 0.1f, 40}}},
		{"", {{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13},{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13},{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13}}},
		{"", {{ENTITY_FAST, 0.05f, 20}, {ENTITY_NORMAL, 0.05f, 20}, {ENTITY_FAST, 0.05f, 20}}},
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
		if (camera.pos.z < 0.5f) {
			camera.pos.z = 0.5f;
		}
		controls.previousClipMouse = controls.clipMouse;
		controls.clipMouse.x = (controls.mouse.x / (float)width - 0.5f) * 2.f;
		controls.clipMouse.y = (0.5f - controls.mouse.y / (float)height) * 2.f;
		controls.worldMouse.x = controls.clipMouse.x * (camera.pos.z) * (float)width / (float)height + camera.pos.x;
		controls.worldMouse.y = controls.clipMouse.y * (camera.pos.z) + camera.pos.y;

		messageTime -= d / 60.f;

		if ((controls.clipMouse.x < -0.7f && controls.previousClipMouse.x >= -0.7f) || (controls.clipMouse.x > 0.7f && controls.previousClipMouse.x <= 0.7f)) {
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
							if (people[i].type == PERSON_TURRET) projectiles.push_back({PROJECTILE_SPARK, vec3Add(people[i].pos, {0.f, 1.f, 0.f}), projectileVel.normalise(0.01f)});
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
			if (projectiles[i].age > 5.f || (projectiles[i].type == PROJECTILE_SPARK && projectiles[i].age > 0.05f) || (projectiles[i].type == PROJECTILE_ELECTRICITY && projectiles[i].age > 0.25)) {
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
								entities.push_back({ENTITY_IRON_MAIDEN, {entities[j].pos.x + randFloat() * 1.f - 0.5f, entities[j].pos.y, entities[j].pos.z + randFloat() * 1.f - 0.5f}});
								entities[entities.size() - 1].targetPoint = entities[j].targetPoint;
							}
						} else if (entities[j].type == ENTITY_IRON_MAIDEN) {
							for (int k = 0; k < 5; k++) {
								entities.push_back({ENTITY_MONSTER, {entities[j].pos.x + randFloat() * 1.f - 0.5f, entities[j].pos.y, entities[j].pos.z + randFloat() * 1.f - 0.5f}});
								entities[entities.size() - 1].targetPoint = entities[j].targetPoint;
							}
						} else if (entities[j].type == ENTITY_TWIN) {
							for (int k = 0; k < 2; k++) {
								entities.push_back({ENTITY_FAST, {entities[j].pos.x + randFloat() * 0.4f - 0.2f, entities[j].pos.y, entities[j].pos.z + randFloat() * 0.4f - 0.2f}});
								entities[entities.size() - 1].targetPoint = entities[j].targetPoint;
							}
						} else if (entities[j].type == ENTITY_GENERAL) tankUnlocked = true;
						if (getWave(waveNumber).message.length() > 0 && entities.size() == 1 && waveCurrentlySpawning.entities.size() == 0) {
							showMessage(getWave(waveNumber).message, 2.5f);
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
		if ((person->stats.level < (int)upgrades.size() - 1 && money >= upgrades[person->stats.level + 1].price)) {
			money -= upgrades[person->stats.level + 1].price;
			person->stats.price += upgrades[person->stats.level + 1].price;
			person->stats.level++;
			switch (person->type) { // make level x+1 dpspp > level x dpspp
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
					person->stats.shootDelay *= 0.6f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.8f;
					person->stats.projectile.health = 2.f;
				} else if (person->stats.level == 3) {
					person->stats.projectile.damage *= 1.5f;
					person->stats.shootDelay *= 0.9f;
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
			case PERSON_BATTERY: // them
				if (person->stats.level == 1) {
					person->stats.range *= 1.5f;
					person->stats.projectile.damage *= 2.f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.7f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.01f;
				}
				break;
			}
		}
	}
};
Vec2 getCharacterCoords(char c) {
	Vec2 coords{31.f, 4.f};
	switch (c) {
		case 'a':coords = {0.f, 0.f};break;
		case 'b':coords = {1.f, 0.f};break;
		case 'c':coords = {2.f, 0.f};break;
		case 'd':coords = {3.f, 0.f};break;
		case 'e':coords = {4.f, 0.f};break;
		case 'f':coords = {5.f, 0.f};break;
		case 'g':coords = {6.f, 0.f};break;
		case 'h':coords = {7.f, 0.f};break;
		case 'i':coords = {8.f, 0.f};break;
		case 'j':coords = {9.f, 0.f};break;
		case 'k':coords = {10.f, 0.f};break;
		case 'l':coords = {11.f, 0.f};break;
		case 'm':coords = {12.f, 0.f};break;
		case 'n':coords = {13.f, 0.f};break;
		case 'o':coords = {14.f, 0.f};break;
		case 'p':coords = {15.f, 0.f};break;
		case 'q':coords = {16.f, 0.f};break;
		case 'r':coords = {17.f, 0.f};break;
		case 's':coords = {18.f, 0.f};break;
		case 't':coords = {19.f, 0.f};break;
		case 'u':coords = {20.f, 0.f};break;
		case 'v':coords = {21.f, 0.f};break;
		case 'w':coords = {22.f, 0.f};break;
		case 'x':coords = {23.f, 0.f};break;
		case 'y':coords = {24.f, 0.f};break;
		case 'z':coords = {25.f, 0.f};break;

		case '1':coords = {0.f, 2.f};break;
		case '2':coords = {1.f, 2.f};break;
		case '3':coords = {2.f, 2.f};break;
		case '4':coords = {3.f, 2.f};break;
		case '5':coords = {4.f, 2.f};break;
		case '6':coords = {5.f, 2.f};break;
		case '7':coords = {6.f, 2.f};break;
		case '8':coords = {7.f, 2.f};break;
		case '9':coords = {8.f, 2.f};break;
		case '0':coords = {9.f, 2.f};break;
		
		case 'A':coords = {0.f, 1.f};break;
		case 'B':coords = {1.f, 1.f};break;
		case 'C':coords = {2.f, 1.f};break;
		case 'D':coords = {3.f, 1.f};break;
		case 'E':coords = {4.f, 1.f};break;
		case 'F':coords = {5.f, 1.f};break;
		case 'G':coords = {6.f, 1.f};break;
		case 'H':coords = {7.f, 1.f};break;
		case 'I':coords = {8.f, 1.f};break;
		case 'J':coords = {9.f, 1.f};break;
		case 'K':coords = {10.f, 1.f};break;
		case 'L':coords = {11.f, 1.f};break;
		case 'M':coords = {12.f, 1.f};break;
		case 'N':coords = {13.f, 1.f};break;
		case 'O':coords = {14.f, 1.f};break;
		case 'P':coords = {15.f, 1.f};break;
		case 'Q':coords = {16.f, 1.f};break;
		case 'R':coords = {17.f, 1.f};break;
		case 'S':coords = {18.f, 1.f};break;
		case 'T':coords = {19.f, 1.f};break;
		case 'U':coords = {20.f, 1.f};break;
		case 'V':coords = {21.f, 1.f};break;
		case 'W':coords = {22.f, 1.f};break;
		case 'X':coords = {23.f, 1.f};break;
		case 'Y':coords = {24.f, 1.f};break;
		case 'Z':coords = {25.f, 1.f};break;
		
		case '!':coords = {26.f, 0.f};break;
		case '@':coords = {27.f, 0.f};break;
		case '#':coords = {28.f, 0.f};break;
		case '$':coords = {29.f, 0.f};break;
		case '%':coords = {30.f, 0.f};break;
		case '^':coords = {31.f, 0.f};break;
		case '&':coords = {26.f, 1.f};break;
		case '*':coords = {27.f, 1.f};break;
		case '(':coords = {28.f, 1.f};break;
		case ')':coords = {29.f, 1.f};break;
		case '-':coords = {30.f, 1.f};break;
		case '=':coords = {31.f, 1.f};break;
		
		case '`':coords = {10.f, 2.f};break;
		case '~':coords = {11.f, 2.f};break;
		case '_':coords = {12.f, 2.f};break;
		case '+':coords = {13.f, 2.f};break;
		case '[':coords = {14.f, 2.f};break;
		case ']':coords = {15.f, 2.f};break;
		case '{':coords = {16.f, 2.f};break;
		case '}':coords = {17.f, 2.f};break;
		case '\\':coords = {18.f, 2.f};break;
		case '|':coords = {19.f, 2.f};break;
		case ';':coords = {20.f, 2.f};break;
		case '\'':coords = {21.f, 2.f};break;
		case ':':coords = {22.f, 2.f};break;
		case '"':coords = {23.f, 2.f};break;
		case ',':coords = {24.f, 2.f};break;
		case '.':coords = {25.f, 2.f};break;
		case '<':coords = {26.f, 2.f};break;
		case '>':coords = {27.f, 2.f};break;
		case '/':coords = {28.f, 2.f};break;
		case '?':coords = {29.f, 2.f};break;
		case ' ':coords = {30.f, 2.f};break;
	};
	return coords;
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
	{PERSON_GOLD_MINE, 4.f, 2.f},
	{PERSON_BATTERY, 5.f, 2.f},
};
string getFileText(string path) {
	ifstream file{path};
	if (!file.is_open()) {
		return "";
		cerr << "ERROR: file \"" << path << "\" not found" << endl;
		file.close();
	}
	string text;
	string lineText;
	while (file) {
		if (!file.good()) {
			break;
		}
		getline(file, lineText);
		text += lineText + "\n";
	}
	file.close();
	return text;
}
class Material {
	public:
		const char* vertexText;
		const char* fragmentText;
		GLuint program,vertexShader,fragmentShader;
		GLint mvp_location, texture1_location, vpos_location, vtexcoord_location;

		int textureWidth, textureHeight, textureColorChannels;
		unsigned char* textureBytes;
		unsigned int texture;

		Material(string vertexFile, string fragmentFile, string textureFile) {
			const string vText = getFileText(vertexFile);
			const string fText = getFileText(fragmentFile);
			loadShaders(vText, fText);

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			
			textureBytes = stbi_load(textureFile.c_str(), &textureWidth, &textureHeight, &textureColorChannels, 0);
			if (textureBytes) {
				glTexImage2D(GL_TEXTURE_2D, 0, textureColorChannels == 4 ? GL_RGBA : GL_RGB, textureWidth, textureHeight, 0, textureColorChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, textureBytes);
				glGenerateMipmap(GL_TEXTURE_2D);
				cout << "INFO: successfully loaded texture file \"" << textureFile << "\"" << endl;
			} else {
				cerr << "ERROR: failed to load texture file \"" << textureFile << "\"" << endl;
			};
			stbi_image_free(textureBytes);

			program = glCreateProgram();
		
			glAttachShader(program, vertexShader);
			glAttachShader(program, fragmentShader);
			glLinkProgram(program);

			mvp_location = glGetUniformLocation(program, "MVP");
			texture1_location = glGetUniformLocation(program, "texture1");
			vpos_location = glGetAttribLocation(program, "vPos");
			vtexcoord_location = glGetAttribLocation(program, "vTexCoord");

			cout << endl;
		}
		void loadShaders(const string vText, const string fText) {
			vertexText = vText.c_str();
			fragmentText = fText.c_str();
			//vertex shader
			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexText, NULL);
			glCompileShader(vertexShader);

			// vertex erors
			GLint vertexSuccess = GL_FALSE;  
			GLint vertexInfoLogLength;
			char vertexInfoLog[512];
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &vertexInfoLogLength);
			glGetShaderInfoLog(vertexShader, 512, NULL, vertexInfoLog);
			if (vertexInfoLogLength > 0) cout << vertexInfoLog << endl;
			if (!vertexSuccess){
				glDeleteShader(vertexShader);
			} else cout << "INFO: Successfuly loaded vertex shader" << endl;

			// fragment shader
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentText, NULL);
			glCompileShader(fragmentShader);

			//fragment erros
			GLint fragmentSuccess = GL_FALSE;
			GLint fragmentInfoLogLength;
			char fragmentInfoLog[512];
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &fragmentInfoLogLength);
			glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentInfoLog);
			if (fragmentInfoLogLength > 0) cout << fragmentInfoLog << endl;

			if (!fragmentSuccess){
				glDeleteShader(fragmentShader);
			} else cout << "INFO: Successfuly loaded framgnet shader!" << endl;
		}
};
string getBeforeChar(string s, char c) {
	string::size_type pos = s.find(c);
	if (pos != string::npos) return s.substr(0, pos);
	else return s;
}
bool isInt(const string& s) {
    string::const_iterator i = s.begin();
    while (i != s.end() && isdigit(*i)) i++;
    return i == s.end() && !s.empty();
}
class objFile {
	public:
		vector<Vec3> vertices = {};
		vector<Vec2> texcoords = {};
		vector<vector<vector<unsigned int>>> faceses = {};
		objFile(vector<Material, allocator<Material>>* materials, string path) {
			for (int i = 0; i < (int)materials->size(); i++) {
				faceses.push_back({});
			}

			// load file
			ifstream file{path};
			if (!file.is_open()) {
				cerr << "ERROR: obj file \"" << path << "\" not found" << endl;
				file.close();
				return;
			}

			// loop over every line
			string lineText;
			int matId = 0;
			while (file) {
				getline(file, lineText);
				lineText += ' ';
				// vertex
				if (lineText.rfind("v ", 0) == 0) { // vert
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
				} else if (lineText.rfind("f ", 0) == 0) { // face
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
					faceses[matId].push_back(face);
				} else if (lineText.rfind("usemtl ", 0) == 0) { // mtl
					string word = "";
					int wordNumber = 0;
					for (char c : lineText) {
						if (c == ' ') {
							if (wordNumber == 1 && isInt(word)) matId = stoi(word);
							word = "";
							wordNumber++;
						} else word += c;
					}
				}
			}
			file.close();
		}
};
class Mesh {
	public:
		vector<Vertex> vertices = {};
		vector<vector<unsigned int>> indiceses = {};
		Mesh(vector<Material, allocator<Material>>* materials, string path) {
			for (int i = 0; i < (int)materials->size(); i++) {
				indiceses.push_back({});
			}
			objFile obj{materials, path};
			for (int i = 0; i < (int)obj.vertices.size(); i++) {
				vertices.push_back({obj.vertices[i].x, obj.vertices[i].y, obj.vertices[i].z, obj.texcoords[i].x, obj.texcoords[i].y, 1.f});
			}
			for (int i = 0; i < (int)obj.faceses.size(); i++) {
				for (int j = 0; j < (int)obj.faceses[i].size(); j++) {
					indiceses[i].insert(indiceses[i].end(), obj.faceses[i][j].begin(), obj.faceses[i][j].end());
				}
			}
		}
};
class GameStateRenderer {
public:
	GameState* game;

	float aspect;
	GLuint vertexBuffer,elementBuffer;

	#define MAT_SOLID 0
	#define MAT_GUI 1
	#define MAT_GRASS 2
	#define MAT_STONE 3
	#define MAT_GUI_CONTAINER 4
	#define MAT_GUI_BUTTON 5
	#define MAT_GUI_BUTTON_DISABLED 6
	#define MAT_GUI_ICON_ARCHER 7
	#define MAT_GUI_ICON_CANNON 8
	#define MAT_GUI_ICON_TURRET 9
	#define MAT_GUI_ICON_TANK 10
	#define MAT_RANGE_BLUE 11
	#define MAT_RANGE_GREEN 12
	#define MAT_RANGE_RED 13
	#define MAT_GUI_FONT 14
	#define MAT_GUI_BUTTON_UPGRADE 15
	#define MAT_GUI_BUTTON_UPGRADE_DISABLED 16
	#define MAT_METAL_BLACK 17
	#define MAT_WOOD 18
	#define MAT_SKIN 19
	#define MAT_METAL 20
	#define MAT_GOLD 21
	#define MAT_GUI_ICON_GOLD_MINE 22
	#define MAT_CAMO 23
	#define MAT_GUI_PLAY 24
	#define MAT_GUI_PLAY_DISABLED 25
	#define MAT_ROCK 26
	#define MAT_FIRE 27
	#define MAT_GUI_ICON_BATTERY 28
	#define MAT_ELECTRICITY 29
	vector<Material> materials = {
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/brain.png"},
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/screen.png"},
		{"resources/shader/solid.vsh", "resources/shader/world_uv_random.fsh", "resources/texture/grass.png"},
		{"resources/shader/solid.vsh", "resources/shader/world_uv.fsh", "resources/texture/stone.png"},
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/gui_container.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/button.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui_grayscale.fsh", "resources/texture/button.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/icon_archer.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/icon_cannon.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/icon_turret.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/icon_tank.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/range_blue.fsh", "resources/texture/brain.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/range_green.fsh", "resources/texture/brain.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/range_red.fsh", "resources/texture/brain.png"}, 
		{"resources/shader/font.vsh", "resources/shader/gui.fsh", "resources/texture/font.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/button_upgrade.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui_grayscale.fsh", "resources/texture/button_upgrade.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/metal_black.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/wood.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/skin.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/metal.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/gold.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/icon_gold_mine.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/camo.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/button_play.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui_grayscale.fsh", "resources/texture/button_play.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/rock.png"}, 
		{"resources/shader/solid.vsh", "resources/shader/solid.fsh", "resources/texture/fire.png"}, 
		{"resources/shader/gui.vsh", "resources/shader/gui.fsh", "resources/texture/icon_battery.png"},
		{"resources/shader/solid.vsh", "resources/shader/electricity.fsh", "resources/texture/icon_battery.png"}
	};

	vector<Vertex> vertices = {};
	vector<vector<unsigned int>> indiceses = {};
	
	Mesh turretMesh{&materials, "resources/model/person/turret.obj"};
	Mesh cannonMesh{&materials, "resources/model/person/cannon.obj"};
	Mesh archerMesh{&materials, "resources/model/person/archer.obj"};
	Mesh tankMesh{&materials, "resources/model/person/tank.obj"};
	Mesh goldMineMesh{&materials, "resources/model/person/goldmine.obj"};
	Mesh batteryMesh{&materials, "resources/model/person/battery.obj"};

	Mesh normalMesh{&materials, "resources/model/entity/normal.obj"};
	Mesh fastMesh{&materials, "resources/model/entity/fast.obj"};
	Mesh monsterMesh{&materials, "resources/model/entity/monster.obj"};
	Mesh generalMesh{&materials, "resources/model/entity/general.obj"};
	Mesh ironMaidenMesh{&materials, "resources/model/entity/ironmaiden.obj"};

	Mesh missileMesh{&materials, "resources/model/projectile/missile.obj"};
	Mesh cannonballMesh{&materials, "resources/model/projectile/cannonball.obj"};
	Mesh sparkMesh{&materials, "resources/model/projectile/spark.obj"};
	Mesh electricityMesh{&materials, "resources/model/projectile/electricity.obj"};

	Mesh planeMesh{&materials, "resources/model/plane.obj"};

	GameStateRenderer(GameState* gam) {
		game = gam;
		
		// create index buffers
		for (int i = 0; i < (int)materials.size(); i++) {
			indiceses.push_back({});
		}

		// vertex buffer
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		// index buffer
		glGenBuffers(1, &elementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	}
	void buildThem(int width, int height) {
		aspect = (float)width / (float)height;
		clearVertices();
		// ground
		for (int x = 0; x < game->levels[game->activeLevel].width; x++) {
			for (int z = 0; z < game->levels[game->activeLevel].height; z++) {
				addPlane((float)x, 0.f, (float)z, 1.f, 1.f, MAT_GRASS);
			}
		}
		// point path
		for (int i = 0; i < (int)game->levels[game->activeLevel].path.size() - 1; i++) {
			addPath(game->levels[game->activeLevel].path[i], game->levels[game->activeLevel].path[i + 1], game->levels[game->activeLevel].pathWidth + 0.1f, 0.01f, MAT_WOOD);
			addPath(game->levels[game->activeLevel].path[i], game->levels[game->activeLevel].path[i + 1], game->levels[game->activeLevel].pathWidth, 0.02f, MAT_STONE);
		}
		// entities
		for (Entity entity : game->entities) {
			switch (entity.type) {
			case ENTITY_NORMAL:
				addMesh(normalMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation);
				break;
			case ENTITY_FAST:
				addMesh(fastMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation);
				break;
			case ENTITY_MONSTER:
				addMesh(monsterMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation);
				break;
			case ENTITY_GENERAL:
				addMesh(generalMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation);
				break;
			case ENTITY_IRON_MAIDEN:
				addMesh(ironMaidenMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation);
				break;
			default:
				addCube(entity.pos.x - entity.size.x / 2.f, entity.pos.y, entity.pos.z - entity.size.z / 2.f, entity.size.x, entity.size.y, entity.size.z, 0);
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
				addMesh(missileMesh, projectile.pos, {1.f, 1.f, 1.f}, atan2(projectile.velocity.z, projectile.velocity.x));
				break;
			case PROJECTILE_SPARK:
				addMesh(sparkMesh, vec3Add(projectile.pos, projectile.velocity.normalise(0.7f)), {1.f, 1.f, 1.f}, atan2(projectile.velocity.z, projectile.velocity.x));
				break;
			case PROJECTILE_CANNONBALL:
				addMesh(cannonballMesh, projectile.pos, {1.f, 1.f, 1.f}, atan2(projectile.velocity.z, projectile.velocity.x));
				break;
			case PROJECTILE_ELECTRICITY:
				addArc(projectile.pos, vec3Add(projectile.pos, projectile.velocity.normalise(1.f)));
				break;
			default:
				addPath(projectile.pos, vec3Add(projectile.pos, projectile.velocity), 0.1f, 0.f, 1);
			}
		}


		// clip space: smaller z is in front

		addText("HEALTH: " + to_string((int)game->health), -0.67f, 0.8f, 0.1f, 0.07f, 0.8f, 2.f);
		addText("WAVE " + to_string(game->waveNumber), -0.3f, -0.95f, 0.1f, 0.07f, 0.8f, 2.f);
		addText("$" + commas(game->money), -0.25f, 0.5f, 0.1f, 0.07f, 0.8f, 14.f * 0.07f * 0.8f);
		//addText("fps:" + to_string(1.f / (float)(frameTime)), -0.1f, 0.65f, 0.1f, 0.07f, 0.8f);
		/*if (game->waveNumber == game->waves.size() && game->waveEnded() && game->health > 0.f) {
			addText("YOU WON", -0.9f, -0.9f, 0.1f, 0.2f, 0.8f);
		}*/
		addRect(0.7f, -1.f, 0.25f, 0.3f, 2.f, MAT_GUI_CONTAINER); // right
		addRect(0.725f, -0.95f, 0.2f, 0.25f, 0.25f, game->waveEnded() ? MAT_GUI_PLAY : MAT_GUI_PLAY_DISABLED); // play button
		for (Person person : game->people) {
			if (person.selected) {
				addRect(0.75f, 0.55f, 0.2f, 0.15f, 0.15f, (person.stats.level < (int)person.getUpgrades().size() - 1 && game->money >= person.getUpgrades()[person.stats.level + 1].price) ? MAT_GUI_BUTTON_UPGRADE : MAT_GUI_BUTTON_UPGRADE_DISABLED);
				vector<PersonUpgrade> upgrades = person.getUpgrades();
				bool isMax = person.stats.level > (int)upgrades.size() - 2;
				string upgradeText = isMax ? "max upgrades" : upgrades[person.stats.level + 1].name + ": $" + to_string((long long)upgrades[person.stats.level + 1].price);
				addText(upgradeText, 0.72f, 0.5f, 0.1f, 0.02f, 0.8f, 0.27f);
				addRect(0.75f, 0.38f, 0.2f, 0.25f, 0.08f, MAT_GUI_BUTTON);
				addText("sell ($" + commas(person.stats.price * 0.9f) + ")", 0.75f, 0.39f, 0.1f, 0.02f, 0.8f, 0.25f);
				float dps = person.stats.projectile.damage / person.stats.shootDelay * person.stats.projectile.health * (3.14159f * powf(person.stats.range, 2.f));
				addText("dps: " + ftos(dps, 2), 0.72f, 0.32f, 0.1f, 0.02f, 0.8f, 2.f);
				addText("dpspp: " + ftos(dps / person.stats.price, 2), 0.72f, 0.28f, 0.1f, 0.02f, 0.8f, 2.f);
				break;
			}
		}
		addRect(-1.f, -1.f, 0.25f, 0.3f, 2.f, MAT_GUI_CONTAINER); // people placing buttons
		for (int i = 0; i < (int)personButtons.size(); i++) {
			float x = -0.98f + (0.15f * fmod(i, 2.f));
			float y = 0.7f - floor(i / 2.f) * 0.15f;
			PersonStats personButtonStats = {personButtons[i].type};
			addRect(x, y, 0.206f, 0.13f, 0.13f, game->money >= personButtonStats.price ? MAT_GUI_BUTTON : MAT_GUI_BUTTON_DISABLED);
			int matId;
			switch (personButtons[i].type) {
			case PERSON_ARCHER:
				matId = MAT_GUI_ICON_ARCHER;
				break;
			case PERSON_CANNON:
				matId = MAT_GUI_ICON_CANNON;
				break;
			case PERSON_TURRET:
				matId = MAT_GUI_ICON_TURRET;
				break;
			case PERSON_TANK:
				matId = MAT_GUI_ICON_TANK;
				break;
			case PERSON_GOLD_MINE:
				matId = MAT_GUI_ICON_GOLD_MINE;
				break;
			case PERSON_BATTERY:
				matId = MAT_GUI_ICON_BATTERY;
				break;
			}
			if (!game->isPlacingPerson || game->placingPerson.type != personButtons[i].type) addRect(x, y, 0.205f, 0.13f, 0.13f, matId);
			PersonStats stats{personButtons[i].type};
			addText("$" + to_string((long long)stats.price), x, y - 0.02f, 0.1f, 0.03f, 0.7f, 2.f);
		}
		//if (game->tankUnlocked) addRect(-0.83f, 0.55f, 0.205f, 0.13f, 0.13f, 3.f, 2.f);
		if (game->messageTime > 0.f) {
			addRect(-0.5f, -0.9f, 0.2f, 1.f, 0.5f, MAT_GUI_CONTAINER);
			addText(game->message, -0.45f, -0.6f, 0.1f, 0.07f, 0.7f, 0.9f);
		}
	}
	void renderMaterials(int width, int height) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);

		buildThem(width, height);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
		for (int i = 0; i < (int)materials.size(); i++) {
			renderMaterial(i, width, height);
		}
	}
	void renderMaterial(int id, int width, int height) {


		glEnableVertexAttribArray(materials[id].vpos_location);
		glVertexAttribPointer(materials[id].vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
	
		glEnableVertexAttribArray(materials[id].vtexcoord_location);
		glVertexAttribPointer(materials[id].vtexcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 3));
		float ratio = (float)width / (float)height;
		mat4x4 m, p, mvp;

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceses[id].size() * sizeof(unsigned int), &indiceses[id][0], GL_STATIC_DRAW);
		
		glBindTexture(GL_TEXTURE_2D, materials[id].texture);

		mat4x4_identity(m);
		mat4x4_translate(m, -game->camera.pos.x, -game->camera.pos.y, -game->camera.pos.z);
		mat4x4_rotate_X(m, m, game->camera.rotation.x);
		mat4x4_rotate_Y(m, m, game->camera.rotation.y);
		mat4x4_rotate_Z(m, m, game->camera.rotation.z);
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_perspective(p, 1.57f, ratio, 0.1f, 200.f);
		mat4x4_mul(mvp, p, m);

		glUniform1i(materials[id].texture1_location, 0);
		glUseProgram(materials[id].program);
		glUniformMatrix4fv(materials[id].mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawElements(GL_TRIANGLES, indiceses[id].size(), GL_UNSIGNED_INT, (void*)0);
	}
private:
	void clearVertices() {
		vertices.clear();
		for (int i = 0; i < (int)indiceses.size(); i++) {
			indiceses[i].clear();
		}
	}

	//============================
	//== world space =============
	//===========================
	void addPerson(Person* person, bool isPlacingPerson) {
		switch (person->type) {
		case PERSON_ARCHER:
			addMesh(archerMesh, person->pos, {.5f, .7f, .5f}, person->yRotation);
			break;
		case PERSON_CANNON:
			addMesh(cannonMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		case PERSON_TURRET:
			addMesh(turretMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		case PERSON_TANK:
			addMesh(tankMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		case PERSON_GOLD_MINE:
			addMesh(goldMineMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		case PERSON_BATTERY:
			addMesh(batteryMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		default:
			addCube(person->pos.x - person->stats.size.x / 2.f, person->pos.y, person->pos.z - person->stats.size.z / 2.f, person->stats.size.x, person->stats.size.y, person->stats.size.z, 0);
			break;
		}
		if (isPlacingPerson) {
			if (person->isPlacable(&game->levels[game->activeLevel], &game->people)) {
				addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, MAT_RANGE_GREEN);
			} else {
				addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, MAT_RANGE_RED);

			}
		} else if (person->selected) {
			addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, MAT_RANGE_BLUE);
		}
	}

	void addCube(float x, float y, float z, float w, float h, float d, int matId) {
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
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
			{x  , y  , z+d, 0.f, 0.f, 1.f},
			{x+w, y  , z+d, 1.f, 0.f, 1.f},
			{x  , y+h, z+d, 0.f, 1.f, 1.f},
			{x+w, y+h, z+d, 1.f, 1.f, 1.f},
			{x  , y  , z  , 0.f, 1.f, 1.f},
			{x+w, y  , z  , 1.f, 1.f, 1.f},
			{x  , y+h, z  , 0.f, 0.f, 1.f},
			{x+w, y+h, z  , 1.f, 0.f, 1.f}
		});
	}
	void addPlane(float x, float y, float z, float w, float d, int matId) {
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y , z+d, 0.f, 0.f, 1.f},
			{x+w, y , z+d, 1.f, 0.f, 1.f},
			{x  , y , z  , 0.f, 1.f, 1.f},
			{x+w, y , z  , 1.f, 1.f, 1.f}
		});
	}
	void addPath(Vec3 p1, Vec3 p2, float width, float yOffset, int matId) {
		unsigned int end = vertices.size();
		float dist = distance3D(p1, p2);
		float xDiff = (p2.x - p1.x) / dist * width / 2.f;
		float zDiff = (p2.z - p1.z) / dist * width / 2.f;
		vertices.insert(vertices.end(), {
			{p1.x - zDiff - xDiff, p1.y + yOffset, p1.z + xDiff - zDiff, 0.f, 1.f, 1.f},
			{p1.x + zDiff - xDiff, p1.y + yOffset, p1.z - xDiff - zDiff, 1.f, 1.f, 1.f},
			{p2.x - zDiff + xDiff, p2.y + yOffset, p2.z + xDiff + zDiff, 0.f, 0.f, 1.f},
			{p2.x + zDiff + xDiff, p2.y + yOffset, p2.z - xDiff + zDiff, 1.f, 0.f, 1.f}
		});
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 2U+end, 1U+end,
			1U+end, 2U+end, 3U+end
		});
	}
	void addMesh(Mesh mesh, Vec3 position, Vec3 scale, float yRotation) {
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
		}
		for (int i = 0; i < (int)mesh.indiceses.size(); i++) {
			for (int j = 0; j < (int)mesh.indiceses[i].size(); j++) {
				mesh.indiceses[i][j] += end;
			}
		}
		vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		for (int i = 0; i < (int)indiceses.size(); i++) {
			indiceses[i].insert(indiceses[i].end(), mesh.indiceses[i].begin(), mesh.indiceses[i].end());
		}
	}
	void addArc(Vec3 start, Vec3 end) {
		Vec3 pos = end;
		for (int i = 0; i < 10; i++) {
			Vec3 it = vec3Add({randFloat() * 2.f - 1.f, 0.f, randFloat() * 2.f - 1.f}, vec3Subtract(start, end).normalise(0.2f));
			addMesh(electricityMesh, pos, {1.f, 1.f, 1.f}, atan2(it.z, it.x));
			pos = vec3Add(pos, it);
		}
	}

	//============================
	//== clip space ==============
	//============================

	void addRect(float x, float y, float z, float w, float h, int matId, Vec2 uvOffset) {
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 2U+end, 1U+end,
			1U+end, 2U+end, 3U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y+h, z, 0.f + uvOffset.x, 0.f + uvOffset.y, 0.f},
			{x+w, y+h, z, 1.f + uvOffset.x, 0.f + uvOffset.y, 0.f},
			{x  , y  , z, 0.f + uvOffset.x, 1.f + uvOffset.y, 0.f},
			{x+w, y  , z, 1.f + uvOffset.x, 1.f + uvOffset.y, 0.f}
		});
	}
	void addRect(float x, float y, float z, float w, float h, int matId) {
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 2U+end, 1U+end,
			1U+end, 2U+end, 3U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y+h, z, 0.f, 0.f, 0.f},
			{x+w, y+h, z, 1.f, 0.f, 0.f},
			{x  , y  , z, 0.f, 1.f, 0.f},
			{x+w, y  , z, 1.f, 1.f, 0.f}
		});
	}
	void addCharacter(char character, float x, float y, float z, float w) {
		Vec2 uv = getCharacterCoords(character);
		addRect(x, y, z, w, w * aspect, MAT_GUI_FONT, uv);
	}
	void addText(string text, float x, float y, float z, float w, float spacing, float maxWidth) {
		for (int i = 0; i < (int)text.length(); i++) {
			float fakeX = (float)i * w * spacing;
			addCharacter(text.at(i), x + fmod(fakeX, maxWidth), y - floor(fakeX / maxWidth) * w * 1.6f, z, w * 1.5f);
			z -= 0.001;
		}
	}
};
GameState game;
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
		else if (key == GLFW_KEY_RIGHT_BRACKET) controls.fast = true;
		else if (key == GLFW_KEY_LEFT_BRACKET) controls.slow = true;
		else if (key == GLFW_KEY_LEFT_SHIFT) controls.shift = true;
		else if (key == GLFW_KEY_U) {
			for (int i = 0; i < (int)game.people.size(); i++) {
				if (game.people[i].selected) {
					game.upgradePerson(&game.people[i]);
					break;
				}
			}
		}
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
		else if (key == GLFW_KEY_RIGHT_BRACKET) controls.fast = false;
		else if (key == GLFW_KEY_LEFT_BRACKET) controls.slow = false;
	};
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	double xpos, ypos;
    	//getting cursor position
    	glfwGetCursorPos(window, &xpos, &ypos);
		
		if (controls.clipMouse.x <= -0.7f || controls.clipMouse.x >= 0.7f) {
			for (int i = 0; i < (int)personButtons.size(); i++) {
				float rectLeft = -0.98f + (0.15f * fmod(i, 2.f));
				float rectRight = rectLeft + 0.13f;
				float rectBottom = 0.7f - floor(i / 2.f) * 0.15f;
				float rectTop = rectBottom + 0.13f;
				if (controls.clipMouse.x > rectLeft && controls.clipMouse.x < rectRight && controls.clipMouse.y > rectBottom && controls.clipMouse.y < rectTop) {
					PersonStats stats = {personButtons[i].type};
					if (game.money >= stats.price) {
						game.placingPerson = {personButtons[i].type, {2.f, 0.f, 2.f}};
						game.isPlacingPerson = true;
						for (int i = 0; i < (int)game.people.size(); i++) {
							game.people[i].selected = false;
						}
					}
				}
			}
			if (controls.clipMouse.x > 0.725f && controls.clipMouse.y < -0.7f && controls.clipMouse.x < 0.975f && controls.clipMouse.y > -0.95f && game.waveEnded()) {
				game.spawnWave(game.getWave(game.waveNumber));
				game.waveNumber++;
				return;
			}
			if (controls.clipMouse.x > 0.75f && controls.clipMouse.y > 0.55f && controls.clipMouse.x < 0.9f && controls.clipMouse.y < 0.7f) {
				for (int i = 0; i < (int)game.people.size(); i++) {
					if (game.people[i].selected) {
						game.upgradePerson(&game.people[i]);
						break;
					}
				}
				return;
			}
			if (controls.clipMouse.x > 0.75f && controls.clipMouse.y > 0.38f && controls.clipMouse.x < 1.f && controls.clipMouse.y < 0.46f) {
				for (int i = 0; i < (int)game.people.size(); i++) {
					if (game.people[i].selected) {
						game.money += game.people[i].stats.price * 0.9f;
						game.people.erase(game.people.begin() + i);
						break;
					}
				}
			}
		} else {
			if (game.isPlacingPerson && game.money >= game.placingPerson.stats.price && game.placingPerson.isPlacable(&game.levels[game.activeLevel], &game.people) && controls.clipMouse.x > -0.7f) {
				game.people.push_back(game.placingPerson);
				game.money -= game.placingPerson.stats.price;
				if (!controls.shift) game.isPlacingPerson = false;
			};
			bool personSelected = false;
			for (int i = 0; i < (int)game.people.size(); i++) {
				game.people[i].selected = !personSelected && !game.isPlacingPerson && (controls.worldMouse.x > game.people[i].pos.z - game.people[i].stats.size.z / 2.f && controls.worldMouse.x < game.people[i].pos.z + game.people[i].stats.size.z / 2.f && controls.worldMouse.y > game.people[i].pos.x - game.people[i].stats.size.x / 2.f && controls.worldMouse.y < game.people[i].pos.x + game.people[i].stats.size.x / 2.f);
				if (game.people[i].selected) personSelected = true;
			}
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
	//defive 
	const ALCchar* defaultDeviceString = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceString);
	if (!device) {
		cerr << "ERROR: failed loading default sound device" << endl;
	}
	cout << "INFO: openal device name: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << endl;

	ALCcontext* context = alcCreateContext(device, nullptr);

	if (!alcMakeContextCurrent(context)) {
		cerr << "ERROR: failed to make openal context current" << endl;
	}
	alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
	alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
	ALfloat forwardAndUpVectors[] = {
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f
	};
	alListenerfv(AL_ORIENTATION, forwardAndUpVectors);

	AudioFile<float> monoSoundFile;
	if (!monoSoundFile.load("resources/audio/brain.wav")) {
		cerr << "ERROR: failed to load brain sound" << endl;
	}
	vector<uint8_t> monoPCMDataBytes;
	monoSoundFile.writePCMToBuffer(monoPCMDataBytes);
	auto convertFileToOpenALFormat = [](const AudioFile<float>& audioFile) {
		int bitDepth = audioFile.getBitDepth();
		if (bitDepth == 16) {
			return audioFile.isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		} else if (bitDepth == 8) {
			return audioFile.isStereo() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		} else {
			cerr << "ERROR: bad bit depth for audio file" << endl;
			return -1;
		}
	};
	ALuint monoSoundBuffer;
	alGenBuffers(1, &monoSoundBuffer);
	alBufferData(monoSoundBuffer, convertFileToOpenALFormat(monoSoundFile), monoPCMDataBytes.data(), monoPCMDataBytes.size(), monoSoundFile.getSampleRate());

	ALuint monoSource;
	alGenSources(1, &monoSource);
	alSource3f(monoSource, AL_POSITION, 1.f, 0.f, 0.f);
	alSource3f(monoSource, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSourcef(monoSource, AL_PITCH, 1.f);
	alSourcef(monoSource, AL_GAIN, 1.f);
	alSourcei(monoSource, AL_LOOPING, AL_FALSE);
	alSourcei(monoSource, AL_BUFFER, monoSoundBuffer);

	alSourcePlay(monoSource);
	ALint sourceState;
	alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState);
	while (sourceState == AL_PLAYING) {
		alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState);
	}
	alDeleteSources(1, &monoSource);
	alDeleteBuffers(1, &monoSoundBuffer);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);


	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Human Tower Defense", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
		return 1;
	}

	GLFWimage images[1]{};
	images[0].pixels = stbi_load("icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_FRONT);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.5f, 0.5f, 0.7f, 1.f);
	lastFrameTime = (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch())).count();
	
	GameStateRenderer renderer{&game};
	while (!glfwWindowShouldClose(window)) {
		long long newFrameTime = (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch())).count();
		frameTime = (float)(newFrameTime - lastFrameTime) / 1000.f;
		lastFrameTime = newFrameTime;
		int width, height;
		
		glfwGetFramebufferSize(window, &width, &height);

		game.d = (controls.slow ? 0.1f : (controls.fast ? 0.5f : 0.2f)) * (frameTime / (1.f / 60.f));
		for (int i = 0; i < (controls.slow ? 1 : (controls.fast ? 10 : 5)); i++) {
			game.tick(width, height);
		}

		renderer.renderMaterials(width, height);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	for (int i = 0; i < (int)renderer.materials.size(); i++) {
		glDeleteTextures(1, &renderer.materials[i].texture);
	}
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}