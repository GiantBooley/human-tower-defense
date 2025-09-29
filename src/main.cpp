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
#include <filesystem>
#define PI 3.1415926f

using namespace std;

bool debug = false;

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


int convertFileToOpenALFormat(AudioFile<float>* audioFile) {
	int bitDepth = audioFile->getBitDepth();
	if (bitDepth == 16) {
		return audioFile->isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	} else if (bitDepth == 8) {
		return audioFile->isStereo() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
	} else {
		cerr << "ERROR: bad bit depth for audio file" << endl;
		return -1;
	}
};
class SoundDoerSound {
	public:
	AudioFile<float> monoSoundFile;
	vector<uint8_t> monoPCMDataBytes;
	bool isLooping = false;
	float volume = 1.f;

	SoundDoerSound(bool looping, float vlaume, string path) {
		isLooping = looping;
		volume = vlaume;
		if (!monoSoundFile.load(path)) {
			cerr << "ERROR: failed to load brain sound \"" << path << "\"" << endl;
		}
		monoSoundFile.writePCMToBuffer(monoPCMDataBytes);
	}
};
class SoundDoerBuffer {
	public:
	ALuint monoSoundBuffer;
	ALuint monoSource;
	SoundDoerBuffer(SoundDoerSound sound) {
		alGenBuffers(1, &monoSoundBuffer);
		alBufferData(monoSoundBuffer, convertFileToOpenALFormat(&sound.monoSoundFile), sound.monoPCMDataBytes.data(), sound.monoPCMDataBytes.size(), sound.monoSoundFile.getSampleRate());

		alGenSources(1, &monoSource);
		alSource3f(monoSource, AL_POSITION, 1.f, 0.f, 0.f);
		alSource3f(monoSource, AL_VELOCITY, 0.f, 0.f, 0.f);
		alSourcef(monoSource, AL_PITCH, sound.isLooping ? 1.f : (randFloat() * 0.2f + 0.9f));
		alSourcef(monoSource, AL_GAIN, sound.volume * 0.5f);
		alSourcei(monoSource, AL_LOOPING, sound.isLooping ? AL_TRUE : AL_FALSE);
		alSourcei(monoSource, AL_BUFFER, monoSoundBuffer);
	}
};
class SoundDoer {
	public:
	ALCdevice* device;
	ALCcontext* context;
	#define SOUND_ELECTRICITY 1
	#define SOUND_IRON 2
	#define SOUND_BUTTON 3
	#define SOUND_HURT 4
	#define SOUND_BOSS_DEATH 5
	#define SOUND_DEATH 6
	#define SOUND_PLACE 7
	#define SOUND_MONEY 8
	#define SOUND_MUSIC1 9
	#define SOUND_ROLL 10
	#define SOUND_ROBOT 11
	#define SOUND_MUSIC_HUMAN_PASSAGES 12
	#define SOUND_MUSIC_FROZEN 13
	vector<SoundDoerSound> sounds = {
		{false, 1.f, "resources/audio/brain.wav"},
		{false, 1.f, "resources/audio/electricity.wav"},
		{false, 1.f, "resources/audio/iron.wav"},
		{false, 1.f, "resources/audio/button.wav"},
		{false, 1.f, "resources/audio/hurt.wav"},
		{false, 1.f, "resources/audio/bossDeath.wav"},
		{false, 1.f, "resources/audio/death.wav"},
		{false, 1.f, "resources/audio/place.wav"},
		{false, 1.f, "resources/audio/money.wav"},
		{true, 0.5f, "resources/audio/music1.wav"},
		{false, 1.f, "resources/audio/roll.wav"},
		{false, 1.f, "resources/audio/robot.wav"},
		{true, 0.5f, "resources/audio/humanPassages.wav"},
		{true, 1.f, "resources/audio/Frozenv1.wav"}
	};
	vector<SoundDoerBuffer> buffers = {};
	SoundDoer() {
		const ALCchar* defaultDeviceString = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		device = alcOpenDevice(defaultDeviceString);
		if (!device) {
			cerr << "ERROR: failed loading default sound device" << endl;
		}
		cout << "INFO: openal device name: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << endl;

		context = alcCreateContext(device, nullptr);

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
	}
	void tickSounds() {
		for (int i = (int)buffers.size() - 1; i >= 0; i--) {
			ALint sourceState;
			alGetSourcei(buffers.at(i).monoSource, AL_SOURCE_STATE, &sourceState);
			if (sourceState != AL_PLAYING) {
				alDeleteSources(1, &buffers.at(i).monoSource);
				alDeleteBuffers(1, &buffers.at(i).monoSoundBuffer);
				buffers.erase(buffers.begin() + i);
			}
		}
	}
	void play(SoundDoerSound sound) {
		if (buffers.size() < 1000) {
			buffers.push_back({sound});
			alSourcePlay(buffers[(int)buffers.size() - 1].monoSource);
		}
	}
	void exit() {
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
};
SoundDoer soundDoer;

class Controls {
	public:
	   bool w = false;
	   bool a = false;
	   bool s = false;
	   bool d = false;
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
#define ENTITY_ENTITYTEMPLATE -1
#define ENTITY_NORMAL 0
#define ENTITY_FAST 1
#define ENTITY_MONSTER 2
#define ENTITY_ADMIN 3
#define ENTITY_IRON_MAIDEN 4
#define ENTITY_TUNGSTEN_MAIDEN 5
#define ENTITY_TWIN 6
#define ENTITY_GENERAL 7
#define ENTITY_TROJANHORSE 8
class Entity {
public:
	int type;
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 size{0.5f, 1.5f, 0.5f};
	int targetPoint = 0;
	float health = 1.f;
	float maxHealth = 1.f;
	float speed = 1.8f;
	float u;
	float reward;
	float damage;
	float yRotation = 0.f;
	Entity(int tyipe, Vec3 position) {
		type = tyipe;
		pos = position;
		switch (type) {
		case ENTITY_ENTITYTEMPLATE:
			speed = 60.f;
			u = 0.f;
			reward = 1.f;
			damage = 1.f;
			size = {1.f, 1.f, 1.f};
			break;
		case ENTITY_NORMAL:
			speed = 1.8f;
			u = 0.f;
			health = 1.f;
			reward = 1.f;
			damage = 1.f;
			size = {1.f, 2.5f, 1.f};
			break;
		case ENTITY_FAST:
			speed = 6.f;
			u = 1.f;
			health = 2.f;
			reward = 5.f;
			damage = 2.f;
			size = {1.f, 1.8f, 1.f};
			break;
		case ENTITY_MONSTER:
			speed = 12.f;
			u = 2.f;
			health = 4.f;
			reward = 15.f;
			damage = 15.f;
			size = {1.f, 2.4f, 1.f};
			break;
		case ENTITY_ADMIN:
			speed = 60.f;
			u = 3.f;
			health = 5.f;
			reward = 700.f;
			damage = 2213.f;
			size = {1.f, 2.7f, 1.f};
			break;
		case ENTITY_IRON_MAIDEN:
			speed = 0.6f;
			u = 4.f;
			health = 400.f;
			reward = 50.f;
			damage = 25.f;
			size = {1.f, 2.f, 1.f};
			break;
		case ENTITY_TUNGSTEN_MAIDEN:
			speed = 0.6f;
			u = 5.f;
			health = 1500.f;
			reward = 800.f;
			damage = 99.f;
			size = {1.f, 3.2f, 1.f};
			break;
		case ENTITY_TWIN:
			speed = 9.f;
			u = 7.f;
			health = 5.f;
			reward = 5.f;
			damage = 5.f;
			size = {1.4f, 3.4f, 1.4f};
			break;
		case ENTITY_GENERAL:
			speed = 3.f;
			u = 8.f;
			health = 800.f;
			reward = 2000.f;
			damage = 50318231.f;
			size = {3.35f, 4.9f, 3.35f};
			break;
		case ENTITY_TROJANHORSE:
			speed = 30.f;
			u = 8.f;
			health = 100.f;
			reward = 2000.f;
			damage = 509231589.f;
			size = {3.f, 2.7f, 3.f};
			break;
		}
		maxHealth = health;
	}
};
class Camera {
public:
	Vec3 pos{0.f, 0.f, 0.f};
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
#define PERSON_ARCHER 0
#define PERSON_CANNON 1
#define PERSON_TURRET 2
#define PERSON_TANK 3
#define PERSON_GOLD_MINE 4
#define PERSON_BATTERY 5
#define PERSON_SCIENTIST 6
#define PERSON_ROBOT 7
#define PERSON_SCARECROW 8

#define PROJECTILE_ARROW 0
#define PROJECTILE_CANNONBALL 1
#define PROJECTILE_BULLET 2
#define PROJECTILE_MISSILE 3
#define PROJECTILE_SPARK 4
#define PROJECTILE_ELECTRICITY 5
#define PROJECTILE_POTION 6
class Projectile {
	public:
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 origin{0.f, 0.f, 0.f};
	Vec3 velocity{0.f, 0.f, 0.f};
	Vec3 size{0.3f, 0.3f, 0.3f};
	int type;
	float age = 0.f;
	float damage = 1.f;
	float health = 1.f;
	float u;
	float speed = 0.1f;
	bool guided = false;
	float rewardMultiplier = 1.f;
	Projectile(int tyape, Vec3 asdpos, Vec3 asdvelocity) {
		type = tyape;
		pos = asdpos;
		origin = asdpos;
		velocity = asdvelocity;
		switch (type) {
		case PROJECTILE_ARROW:
			damage = 1.3f;
			u = 0.f;
			speed = 30.f;
			health = 1.f;
			break;
		case PROJECTILE_CANNONBALL:
			damage = 3.5f;
			u = 1.f;
			speed = 36.f;
			health = 5.f;
			break;
		case PROJECTILE_BULLET:
			damage = 1.5f;
			u = 2.f;
			speed = 78.f;
			break;
		case PROJECTILE_MISSILE:
			damage = 6.f;
			u = 3.f;
			speed = 10.f;
			health = 5.f;
			break;
		case PROJECTILE_SPARK:
			damage = 0.f;
			u = 3.f;
			speed = 200.f;
			health = 10000000.f;
			break;
		case PROJECTILE_ELECTRICITY:
			damage = 0.5f;
			u = 3.f;
			health = 3.f;
			guided = true;
			speed = 100.f;
			break;
		case PROJECTILE_POTION:
			damage = 2.f;
			u = 3.f;
			health = 1.f;
			speed = 12.f;
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
	float income = 0.f;
	float incomeDelay = 1.f;
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
			price = 3600.f;
			range = 16.f;
			shootDelay = 0.4f;
			projectile = {PROJECTILE_MISSILE, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			projectile.damage = 11.f;
			size = {3.5f, 2.5f, 3.5f};
			break;
		case PERSON_GOLD_MINE:
			price = 600.f;
			range = 3.f;
			shootDelay = 5.f;
			incomeDelay = 5.f;
			projectile = {PROJECTILE_MISSILE, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			income = 10.f;
			size = {3.f, 2.5f, 3.f};
			break;
		case PERSON_BATTERY:
			price = 750.f;
			range = 3.f;
			shootDelay = 0.1f;
			projectile = {PROJECTILE_ELECTRICITY, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			size = {0.8f, 2.2f, 0.8f};
			break;
		case PERSON_SCIENTIST:
			price = 999.f;
			range = 4.f;
			shootDelay = 0.7f;
			projectile = {PROJECTILE_POTION, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			size = {0.35f, 2.4f, 0.35f};
			break;
		case PERSON_ROBOT:
			price = 1500.f;
			range = 3.5f;
			shootDelay = 0.7f;
			projectile = {PROJECTILE_ELECTRICITY, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			projectile.damage *= 9.f;
			size = {1.f, 2.f, 1.f};
			break;
		case PERSON_SCARECROW:
			price = 150.f;
			range = 3.f;
			shootDelay = 0.3f;
			projectile = {PROJECTILE_ARROW, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}};
			projectile.damage *= 0.5f;
			size = {0.5, 2.5f, 0.5};
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
	float incomeDelayTimer = 0.f;
	Person(int tyape, Vec3 asdpos) {
		type = tyape;
		yRotation = 3.141359f;
		stats = {type};
		pos = asdpos;
	}
	vector<PersonUpgrade> getUpgrades() {
		switch (type) {
		case PERSON_ARCHER:
			return {{"default", 0.f}, {"sharpened arrows", 100.f}, {"scope", 200.f}, {"crossbow", 500.f}};
		case PERSON_CANNON:
			return {{"default", 0.f}, {"shorter fuse", 175.f}, {"missile launcher", 350.f}, {"op", 800.f}, {"the last resort", 500000.f}};
		case PERSON_TURRET:
			return {{"default", 0.f}, {"fast", 300.f}, {"extra pierce", 500.f}, {"ultra fast", 5000.f}};
		case PERSON_TANK:
			return {{"default", 0.f}, {"strong missiles", 700.f}, {"guided missiles", 1220.f}, {"gigatron", 123123123.f}}; // move around
		case PERSON_GOLD_MINE:
			return {{"default", 0.f}, {"faster minecart", 300.f}, {"electric minecart", 420.f}, {"mega fast minecart", 587.f}};
		case PERSON_BATTERY:
			return {{"default", 0.f}, {"more volts", 300.f}, {"radio silence", 250}, {"li ion", 450.f}, {"C.E.L.L", 100100.f}}; // charged electric lightning laser
		case PERSON_SCIENTIST:
			return {{"default", 0.f}, {"tranquilizer", 300.f}, {"open potion", 450.f}, {"mad scientist", 5000.f}, {"the kingpin", 15455.f}};
		case PERSON_ROBOT:
			return {{"default", 0.f}, {"carbon footprint", 300.f}, {"polytechnic infrastructure", 450.f}, {"tech giant", 5000.f}, {"the armageddon", 15455.f}};
		case PERSON_SCARECROW:
			return {{"default", 0.f}, {"umbrella shooter", 500.f}, {"second nature", 700.f}, {"bountiful harvest", 3001.f}, {"the last straw", 1000000.f}};
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
		float getTotalHealth() {
			float total = 0.f;
			for (int i = 0; i < (int)entities.size(); i++) {
				Entity e{entities.at(i).id, {0.f, 0.f, 0.f}};
				total += e.health * (e.speed / 0.03f) * (float)entities.at(i).count;
			}
			return total;
		}
};
float frameTime = 0.0f;
long long lastFrameTime = 0LL;
string getFileNameFromPath(string path) {
	for (int i = (int)path.size() - 1; i >= 0; i--) {
		if (i == 0 || path.at(i) == '/' || path.at(i) == '\\') {
			string extension = path.substr(i + 1);
			return extension.substr(0, extension.find('.'));
		}
	}
	return "idk";
}
class Level {
public:
	Vec3 size{25.f, 5.f, 25.f};
	float pathWidth = 0.6f;
	float pathHeight = 0.01f;
	string pathMaterial = "stone";
	float pathBaseWidth = 0.61f;
	float pathBaseHeight = 0.005f;
	string pathBaseMaterial = "wood";
	string groundMaterial = "grass";
	string wallMaterial = "rock";
	string name = "level";
	int music = SOUND_MUSIC1;
	vector<Vec3> path = {{2.000000f, 0.000000f, 0.000000f}, {5.000000f, 0.000000f, 2.000000f}, {2.000000f, 0.000000f, 8.000000f}, {4.000000f, 0.000000f, 8.000000f}, {4.000000f, 0.000000f, 3.000000f}, {1.000000f, 0.000000f, 3.000000f}, {1.000000f, 0.000000f, 9.000000f}, {6.000000f, 0.000000f, 9.000000f}, {6.000000f, 0.000000f, 3.000000f}, {9.000000f, 0.000000f, 3.000000f}, {9.000000f, 0.000000f, 10.000000f}, {14.007325f, 0.000000f, 10.099076f}, {15.940928f, 0.000000f, 10.099076f}, {17.275387f, 0.000000f, 9.935673f}, {18.446444f, 0.000000f, 9.282061f}, {18.909418f, 0.000000f, 7.974836f}, {18.065170f, 0.000000f, 7.239522f}, {16.512840f, 0.000000f, 7.076118f}, {15.341784f, 0.000000f, 6.994416f}, {14.034558f, 0.000000f, 6.967182f}, {13.054140f, 0.000000f, 6.939949f}, {13.026906f, 0.000000f, 4.053161f}, {13.026906f, 0.000000f, 1.629347f}, {15.423485f, 0.000000f, 1.493177f}, {16.676243f, 0.000000f, 1.547646f}, {17.547726f, 0.000000f, 2.174024f}, {18.228573f, 0.000000f, 2.691469f}, {18.909418f, 0.000000f, 3.236145f}, {19.753668f, 0.000000f, 3.426783f}, {20.652386f, 0.000000f, 3.372314f}, {21.796207f, 0.000000f, 3.454016f}, {22.749392f, 0.000000f, 3.454016f}, {23.593641f, 0.000000f, 3.317847f}, {24.328957f, 0.000000f, 2.882105f}, {24.546825f, 0.000000f, 2.065089f}, {24.111084f, 0.000000f, 1.275308f}, {23.185135f, 0.000000f, 0.812331f}, {22.449820f, 0.000000f, 1.084670f}, {22.286417f, 0.000000f, 1.738283f}, {22.286417f, 0.000000f, 2.364661f}, {22.313650f, 0.000000f, 3.835290f}, {22.313650f, 0.000000f, 5.659958f}, {22.177483f, 0.000000f, 10.888858f}};
	Level(string fileName) {
		name = getFileNameFromPath(fileName);
		ifstream file(fileName);
		if (!file.good()) return;
		string line;
		path.clear();
		while (getline(file, line)) {
			// fix \r\n
			if (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}

			if (line.rfind("size=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				size = vec3FromString(value);
			} else if (line.rfind("path_points=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				string word = "";
				for (char c : value) {
					if (c == ';') {
						path.push_back(vec3FromString(word));
						word = "";
					} else word += c;
				}
				path.push_back(vec3FromString(word));
			} else if (line.rfind("path_material=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				pathMaterial = value;
			} else if (line.rfind("path_base_width=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				pathBaseHeight = stof(value);
			} else if (line.rfind("path_base_height=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				pathBaseHeight = stof(value);
			} else if (line.rfind("path_base_material=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				pathBaseMaterial = value;
			} else if (line.rfind("ground_material=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				groundMaterial = value;
			} else if (line.rfind("wall_material=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				wallMaterial = value;
			} else if (line.rfind("music=", 0) == 0) {
				string value = line.substr(line.find("=") + 1);
				music = stoi(value);
			}
		}
		file.close();
	}
	Vec3 vec3FromString(string s) {
		vector<float> qweqwe = {};
		string number = "";
		for (char c : s) {
			if (c == ',' || c == ']') {
				qweqwe.push_back(stof(number));
				number = "";
			} else if (c != '[') number += c;
		}
		if ((int)qweqwe.size() < 3) {
			cerr << "ERROR: vec3fromstring thing has less then 3 floats" << endl;
			return {0.f, 0.f, 0.f};
		} else {
			return {qweqwe.at(0), qweqwe.at(1), qweqwe.at(2)};
		}
	}
};
void exportLevel(Level level, string fileName) {
	ofstream file(fileName);
	file << "size=[" << level.size.x << "," << level.size.y << "," << level.size.z << "]\n";
	file << "path_width=" << level.pathWidth << "\n";
	file << "path_height=" << level.pathHeight << "\n";
	file << "path_points=";
	for (int i = 0; i < (int)level.path.size(); i++) {
		if (i > 0) file << ";";
		file << "[" << level.path.at(i).x << "," << level.path.at(i).y << "," << level.path.at(i).z << "]";
	}
	file << "\n";
	file << "path_material=" << level.pathMaterial << "\n";
	file << "path_base_width=" << level.pathBaseWidth << "\n";
	file << "path_base_height=" << level.pathBaseHeight << "\n";
	file << "path_base_material=" << level.pathBaseMaterial << "\n";
	file << "ground_material=" << level.groundMaterial << "\n";
	file << "wall_material=" << level.wallMaterial << "\n";
	file << "music=" << level.music << "\n";
	file.close();
};
class World {
	public:
	Level level{"levels/level.htdlvl"};
	vector<Entity> entities = {};
	vector<Person> people = {};
	vector<Projectile> projectiles = {};
	Camera camera;
	float money = 201231231230.f;
	float health = 100.f;
	int waveNumber = 0;
	bool tankUnlocked = false;
	
	float entitySpawnDelay = 0.f;
	World(Level lavel) {
		level = lavel;
	}
	bool isPersonPlacable(Person* person) {
		if (person->pos.x < 0.f || person->pos.x > level.size.x || person->pos.z < 0.f || person->pos.z > level.size.z) return false;
		for (int i = 0; i < (int)level.path.size() - 1; i++) {
			if (lineCircleIntersects(level.path.at(i).x, level.path.at(i).z, level.path.at(i + 1).x, level.path.at(i + 1).z, person->pos.x, person->pos.z, (person->stats.size.x + person->stats.size.z) / 4.f + level.pathWidth / 2.f)) {
				return false;
			}
		}
		for (int i = 0; i < (int)people.size(); i++) {
			if (
				person->pos.x + person->stats.size.x / 2.f > people.at(i).pos.x - people.at(i).stats.size.x / 2.f &&
				person->pos.x - person->stats.size.x / 2.f < people.at(i).pos.x + people.at(i).stats.size.x / 2.f &&
				person->pos.y + person->stats.size.y	   > people.at(i).pos.y								&&
				person->pos.y							  < people.at(i).pos.y + people.at(i).stats.size.y	   &&
				person->pos.z + person->stats.size.z / 2.f > people.at(i).pos.z - people.at(i).stats.size.z / 2.f &&
				person->pos.z - person->stats.size.z / 2.f < people.at(i).pos.z + people.at(i).stats.size.z / 2.f) {
				return false;
			}
		}
		return true;
	}
};
float shortestAngle(float a, float b) {
	float diff = fmod((b - a + 3.f * PI), 2.f * PI) - PI;
	return diff;
}
vector<Level> levels = {};
class GameState {
public:
	World world{{""}};
	
	float d = 1.0; // delta

	bool isPlacingPerson = false;
	
	string message;
	float messageTime;

	Person placingPerson{PERSON_ARCHER, {2.f, 0.f, 2.f}};
	Wave waveCurrentlySpawning{"", {}};
	int gameStatus = 1; // 0:playing,1:menu
	vector<Wave> waves = {
		{"", {{ENTITY_NORMAL, 0.3f, 10}}},
		{"", {{ENTITY_NORMAL, 0.3f, 10}}},
		{"", {{ENTITY_NORMAL, 0.3f, 20}}},
		{"", {{ENTITY_NORMAL, 0.25f, 15}}},
		{"", {{ENTITY_NORMAL, 0.3f, 30}}},
		{"", {{ENTITY_NORMAL, 0.2f, 10}, {ENTITY_FAST, 0.3f, 3}}},
		{"", {{ENTITY_NORMAL, 0.15f, 30}}},
		{"", {{ENTITY_NORMAL, 0.15f, 10}, {ENTITY_FAST, 0.2f, 6}, {ENTITY_NORMAL, 0.2f, 20}}},
		{"", {{ENTITY_FAST, 0.15f, 6}, {ENTITY_NORMAL, 0.2f, 6}, {ENTITY_FAST, 0.2f, 6}}},
		{"", {{ENTITY_FAST, 0.1f, 8}, {ENTITY_NORMAL, 0.2f, 30}}},
		{"", {{ENTITY_NORMAL, 0.1f, 5}, {ENTITY_FAST, 0.2f, 6}, {ENTITY_NORMAL, 0.1f, 5}, {ENTITY_FAST, 0.2f, 6}, {ENTITY_NORMAL, 0.1f, 5}, {ENTITY_FAST, 0.2f, 6}}},
		{"monsters this wave", {{ENTITY_MONSTER, 0.5f, 12}}},
		{"", {{ENTITY_FAST, 0.1f, 30}}},
		{"", {{ENTITY_NORMAL, 0.2f, 20}, {ENTITY_MONSTER, 0.5f, 4}, {ENTITY_FAST, 0.1f, 10}}},
		{"", {{ENTITY_FAST, 0.2f, 10}, {ENTITY_MONSTER, 0.5f, 5}, {ENTITY_NORMAL, 0.1f, 30}}},
		{"", {{ENTITY_FAST, 0.05f, 20}, {ENTITY_NORMAL, 0.1f, 5}}},
		{"", {{ENTITY_NORMAL, 0.1f, 260}}},
		{"", {{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13},{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13},{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13}}},
		{"", {{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13},{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13},{ENTITY_NORMAL, .1f, 13},{ENTITY_FAST, .1f, 13}}},
		{"", {{ENTITY_FAST, 0.05f, 20}, {ENTITY_NORMAL, 0.05f, 20}, {ENTITY_FAST, 0.05f, 20}}},
		{"iron maiden has alot of hp", {{ENTITY_IRON_MAIDEN, 0.5f, 1}}},
		{"", {{ENTITY_FAST, 0.2f, 100}}},
		{"", {{ENTITY_NORMAL, 0.2f, 10}, {ENTITY_FAST, 0.08f, 30}}},
		{"", {{ENTITY_MONSTER, 0.2f, 5}, {ENTITY_NORMAL, 0.1f, 20}}},
		{"", {{ENTITY_MONSTER, 0.2f, 100}}},
		{"", {{ENTITY_FAST, 0.1f, 10}, {ENTITY_NORMAL, 0.05f, 50}}},
		{"", {{ENTITY_IRON_MAIDEN, 2.f, 3}}},
		{"", {{ENTITY_IRON_MAIDEN, 2.f, 5}}},
		{"", {{ENTITY_MONSTER, 0.5f, 10}}},
		{"", {{ENTITY_IRON_MAIDEN, 1.5f, 3}}},
		{"", {{ENTITY_IRON_MAIDEN, 1.5f, 3},{ENTITY_MONSTER, 0.5f, 20}}},
		{"", {{ENTITY_IRON_MAIDEN, 2.f, 5}, {ENTITY_NORMAL, 0.05f, 100}}},
		{"", {{ENTITY_NORMAL, 0.02f, 70}}},
		{"boss round", {{ENTITY_GENERAL, 2.f}}},
		{"tank unlocked!", {{ENTITY_FAST, 0.3f, 20},{ENTITY_IRON_MAIDEN, 2.f, 2}}},
		{"twins drop 2 fasts", {{ENTITY_TWIN, 0.1f, 10}}},
		{"", {{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f},{ENTITY_TWIN, 0.2f},{ENTITY_MONSTER,0.2f}}},
		{"", {{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f}}},
		{"", {{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_IRON_MAIDEN, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_IRON_MAIDEN, 0.05f},{ENTITY_NORMAL, 0.05f},{ENTITY_NORMAL, 0.05f}}},
		{"", {{ENTITY_TWIN, 0.05f},{ENTITY_TWIN, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f},{ENTITY_MONSTER, 0.05f}}},
		{"tungsten maiden this round", {{ENTITY_TUNGSTEN_MAIDEN, 0.1f}}},
		{"", {{ENTITY_NORMAL, 0.1f},{ENTITY_NORMAL, 0.1f},{ENTITY_NORMAL, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_FAST, 0.1f},{ENTITY_MONSTER, 0.1f},{ENTITY_MONSTER, 0.1f},{ENTITY_MONSTER, 0.1f},{ENTITY_IRON_MAIDEN, 0.1f},{ENTITY_IRON_MAIDEN, 0.1f},{ENTITY_IRON_MAIDEN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f},{ENTITY_TWIN, 0.1f}}},
		{"admin", {{ENTITY_ADMIN, 0.1f}}},
		{"", {{ENTITY_TUNGSTEN_MAIDEN, 1.f, 10}, {ENTITY_NORMAL, 0.02f, 1000}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f}}},
		{"trojan horse", {{ENTITY_TROJANHORSE, 0.1f, 20}}},
		{"guys", {{ENTITY_NORMAL, 0.01f, 10000}}},
		{"boss round 2", {{ENTITY_GENERAL, 2.f, 10}}}
	};
	GameState() {
		
	}
	
	void tick(int width, int height) {
		if (controls.w) world.camera.pos.z -= 10.f * d;
		if (controls.s) world.camera.pos.z += 10.f * d;
		if (controls.up) world.camera.pos.y += 10.f * d;
		if (controls.down) world.camera.pos.y -= 10.f * d;
		if (controls.left) world.camera.pos.x -= 10.f * d;
		if (controls.right) world.camera.pos.x += 10.f * d;
		if (world.camera.pos.z < 0.5f) {
			world.camera.pos.z = 0.5f;
		}
		controls.previousClipMouse = controls.clipMouse;
		controls.clipMouse.x = (controls.mouse.x / (float)width - 0.5f) * 2.f;
		controls.clipMouse.y = (0.5f - controls.mouse.y / (float)height) * 2.f;
		controls.worldMouse.x = controls.clipMouse.x * (world.camera.pos.z) * (float)width / (float)height + world.camera.pos.x;
		controls.worldMouse.y = controls.clipMouse.y * (world.camera.pos.z) + world.camera.pos.y;

		messageTime -= d;
		if (gameStatus == 0) {
			if ((controls.clipMouse.x < -0.7f && controls.previousClipMouse.x >= -0.7f) || (controls.clipMouse.x > 0.7f && controls.previousClipMouse.x <= 0.7f)) {
				isPlacingPerson = false;
			}

			if (world.entitySpawnDelay <= 0.f && waveCurrentlySpawning.entities.size() > 0) {
				world.entities.push_back({waveCurrentlySpawning.entities.at(0).id, world.level.path.at(0)});
				world.entitySpawnDelay = waveCurrentlySpawning.entities.at(0).delay;
				waveCurrentlySpawning.entities.at(0).count--;
				if (waveCurrentlySpawning.entities.at(0).count <= 0) {
					waveCurrentlySpawning.entities.erase(waveCurrentlySpawning.entities.begin());
				}
			} else {
				world.entitySpawnDelay -= d;
			};
			if (isPlacingPerson) {
				placingPerson.pos.x = controls.worldMouse.y;
				placingPerson.pos.z = controls.worldMouse.x;
			}
			for (size_t i = 0; i < world.people.size(); i++) {
				if (world.people.at(i).shootDelayTimer <= 0.f) {
					if (!waveEnded() && (world.entities.size() > 0 || world.people.at(i).type == PERSON_GOLD_MINE)) {
						while (world.people.at(i).shootDelayTimer <= 0.f) {
							world.people.at(i).shootDelayTimer += world.people.at(i).stats.shootDelay;
							if (world.people.at(i).type == PERSON_GOLD_MINE) {
								world.money += world.people.at(i).stats.projectile.damage;
							} else {
								int closestEntityIndex = getClosestEntity(world.people.at(i).pos, world.people.at(i).stats.range);
								if (closestEntityIndex != -1) {
									world.people.at(i).yRotation = atan2(world.entities.at(closestEntityIndex).pos.z - world.people.at(i).pos.z, world.entities.at(closestEntityIndex).pos.x - world.people.at(i).pos.x);
									world.projectiles.push_back(world.people.at(i).stats.projectile);
									Vec3 projectileVel = vec3Subtract(world.entities.at(closestEntityIndex).pos, world.people.at(i).pos).normalise(world.projectiles.at(world.projectiles.size() - 1).speed);
									world.projectiles.at(world.projectiles.size() - 1).pos = world.people.at(i).pos;
									world.projectiles.at(world.projectiles.size() - 1).pos.y += 1.f;
									world.projectiles.at(world.projectiles.size() - 1).origin = world.projectiles.at(world.projectiles.size() - 1).pos;
									world.projectiles.at(world.projectiles.size() - 1).velocity = projectileVel;
									if (world.people.at(i).type == PERSON_TURRET) world.projectiles.push_back({PROJECTILE_SPARK, vec3Add(world.people.at(i).pos, {0.f, 1.f, 0.f}), projectileVel.normalise(0.01f)});
								}
							}
						}
					}
				} else {
					world.people.at(i).shootDelayTimer -= d;
				}
				if (world.people.at(i).incomeDelayTimer <= 0.f) {
					if (!waveEnded()) {
						while (world.people.at(i).incomeDelayTimer <= 0.f) {
							world.people.at(i).incomeDelayTimer += world.people.at(i).stats.incomeDelay;
							world.money += world.people.at(i).stats.income;
						}
					}
				} else {
					world.people.at(i).incomeDelayTimer -= d;
				}
			}
			for (int i = (int)world.projectiles.size() - 1; i >= 0; i--) {
				if (world.projectiles.at(i).guided) {
					int closestEntityIndex = getClosestEntity(world.projectiles.at(i).pos, 10.f);
					if (closestEntityIndex != -1) world.projectiles.at(i).velocity = vec3Subtract(world.entities[closestEntityIndex].pos, world.projectiles.at(i).pos).normalise(world.projectiles.at(i).speed);
				}
				world.projectiles.at(i).pos = vec3Add(world.projectiles.at(i).pos, vec3Mul(world.projectiles.at(i).velocity, d));
				world.projectiles.at(i).age += d;
				if (world.projectiles.at(i).pos.z < 0.f || world.projectiles.at(i).pos.z > world.level.size.z || world.projectiles.at(i).pos.x < 0.f || world.projectiles.at(i).pos.x > world.level.size.x || world.projectiles.at(i).age > 5.f || (world.projectiles.at(i).type == PROJECTILE_SPARK && world.projectiles.at(i).age > 0.05f) || (world.projectiles.at(i).type == PROJECTILE_ELECTRICITY && world.projectiles.at(i).age > 30.25)) {
					world.projectiles.erase(world.projectiles.begin() + i);
					continue;
				}
				for (int j = 0; j < (int)world.entities.size(); j++) {
					if (
						world.projectiles.at(i).pos.x - world.projectiles.at(i).size.x / 2.f < world.entities.at(j).pos.x + world.entities.at(j).size.x / 2.f &&
						world.projectiles.at(i).pos.x + world.projectiles.at(i).size.x / 2.f > world.entities.at(j).pos.x - world.entities.at(j).size.x / 2.f &&
						world.projectiles.at(i).pos.y									 < world.entities.at(j).pos.y + world.entities.at(j).size.y	   &&
						world.projectiles.at(i).pos.y + world.projectiles.at(i).size.y	   > world.entities.at(j).pos.y							&&
						world.projectiles.at(i).pos.z - world.projectiles.at(i).size.z / 2.f < world.entities.at(j).pos.z + world.entities.at(j).size.z / 2.f &&
						world.projectiles.at(i).pos.z + world.projectiles.at(i).size.z / 2.f > world.entities.at(j).pos.z - world.entities.at(j).size.z / 2.f
					) {
						world.projectiles.at(i).health -= 1.f;
						world.entities.at(j).health -= world.projectiles[i].damage;
						if (world.projectiles.at(i).type == PROJECTILE_ELECTRICITY) {
							soundDoer.play(soundDoer.sounds[SOUND_ELECTRICITY]);
						}
						if (world.entities.at(j).type == ENTITY_IRON_MAIDEN || world.entities.at(j).type == ENTITY_TUNGSTEN_MAIDEN) {
							soundDoer.play(soundDoer.sounds[SOUND_IRON]);
						} else {
							if (world.entities.at(j).health <= 0.f) {
								if (world.entities.at(j).type == ENTITY_GENERAL) {
									soundDoer.play(soundDoer.sounds[SOUND_BOSS_DEATH]);
								} else {
									soundDoer.play(soundDoer.sounds[SOUND_DEATH]);
								}
							} else {
								soundDoer.play(soundDoer.sounds[SOUND_HURT]);
							}
						}
						if (world.entities.at(j).health <= 0.f) {
							world.money += world.entities.at(j).reward * world.projectiles[i].rewardMultiplier;
							if (world.entities.at(j).type == ENTITY_TUNGSTEN_MAIDEN) {
								for (int k = 0; k < 5; k++) {
									world.entities.push_back({ENTITY_IRON_MAIDEN, {world.entities.at(j).pos.x + randFloat() * 1.f - 0.5f, world.entities.at(j).pos.y, world.entities.at(j).pos.z + randFloat() * 1.f - 0.5f}});
									world.entities[world.entities.size() - 1].targetPoint = world.entities.at(j).targetPoint;
								}
							} else if (world.entities.at(j).type == ENTITY_IRON_MAIDEN) {
								for (int k = 0; k < 5; k++) {
									world.entities.push_back({ENTITY_MONSTER, {world.entities.at(j).pos.x + randFloat() * 1.f - 0.5f, world.entities.at(j).pos.y, world.entities.at(j).pos.z + randFloat() * 1.f - 0.5f}});
									world.entities[world.entities.size() - 1].targetPoint = world.entities.at(j).targetPoint;
								}
							} else if (world.entities.at(j).type == ENTITY_TWIN) {
								for (int k = 0; k < 2; k++) {
									world.entities.push_back({ENTITY_FAST, {world.entities.at(j).pos.x + randFloat() * 0.4f - 0.2f, world.entities.at(j).pos.y, world.entities.at(j).pos.z + randFloat() * 0.4f - 0.2f}});
									world.entities[world.entities.size() - 1].targetPoint = world.entities.at(j).targetPoint;
								}
							} else if (world.entities.at(j).type == ENTITY_GENERAL) world.tankUnlocked = true;
							if (getWave(world.waveNumber).message.length() > 0 && world.entities.size() == 1 && waveCurrentlySpawning.entities.size() == 0) {
								showMessage(getWave(world.waveNumber).message, 2.5f);
							}
							world.entities.erase(world.entities.begin() + j);
						};
						if (world.projectiles.at(i).health <= 0.f) {
							world.projectiles.erase(world.projectiles.begin() + i);
						}
						break;
					}
				}
			}
			for (int i = (int)world.entities.size() - 1; i >= 0; i--) {
				if (world.entities.at(i).health <= 0.f) world.entities.erase(world.entities.begin() + i); // remove if health is less than 0
				Vec3 targetPointPos = world.level.path.at(world.entities.at(i).targetPoint);
				float distanceToTargetPoint = distance3D(world.entities.at(i).pos, targetPointPos);
				if (distanceToTargetPoint < world.entities.at(i).speed * d) { // entity reached next point
					world.entities.at(i).targetPoint++;
					if (world.entities.at(i).targetPoint >= (int)world.level.path.size()) { // reached end
						world.health -= world.entities.at(i).damage;
						world.entities.erase(world.entities.begin() + i);
						continue;
					}
					targetPointPos = world.level.path.at(world.entities.at(i).targetPoint);
					distanceToTargetPoint = distance3D(world.entities.at(i).pos, targetPointPos);
				}
				float xDelta = (targetPointPos.x - world.entities.at(i).pos.x) / distanceToTargetPoint * world.entities.at(i).speed;
				float zDelta = (targetPointPos.z - world.entities.at(i).pos.z) / distanceToTargetPoint * world.entities.at(i).speed;
				// correct rotation
				float rotation = atan2(zDelta, xDelta);
				float angleDelta = -shortestAngle(rotation, world.entities.at(i).yRotation);
				if (std::abs(angleDelta) > 6.f * d) { // angle is far away
					world.entities.at(i).yRotation += (angleDelta > 0.f ? 1.f : -1.f) * 6.f * d;
				} else { // angle is close
					world.entities.at(i).yRotation = rotation;
				}

				world.entities.at(i).pos.x += xDelta * d;
				world.entities.at(i).pos.y += (targetPointPos.y - world.entities.at(i).pos.y) / distanceToTargetPoint * world.entities.at(i).speed * d;
				world.entities.at(i).pos.z += zDelta * d;
				if ((world.entities.at(i).type == ENTITY_GENERAL || world.entities.at(i).type == ENTITY_TROJANHORSE) && fmod(glfwGetTime(), 0.563f) < d) {
					soundDoer.play(soundDoer.sounds[SOUND_ROLL]);
				}
			}
			if (world.health <= 0.f) {
				world.camera.pos.y = 123987.f;
			}
		} else {

		}
	}
	void spawnEntity() {
		if (randFloat() < 0.5) {
			world.entities.push_back({ENTITY_NORMAL, world.level.path.at(0)});
		} else {
			world.entities.push_back({ENTITY_FAST, world.level.path.at(0)});
		}
	}
	void spawnWave(Wave wave) {
		waveCurrentlySpawning = wave;
	}
	bool waveEnded() {
		return waveCurrentlySpawning.entities.size() == 0 && world.entities.size() == 0;
	}
	void showMessage(string text, float time) {
		message = text;
		messageTime = time;
	}
	int getClosestEntity(Vec3 pos, float searchRadius) {
		int nearestIndex = -1;
		float nearestDist;
		for (int i = 0; i < (int)world.entities.size(); i++) {
			float currentDistance = distance3D(world.entities.at(i).pos, pos);
			if ((nearestIndex == -1 || currentDistance < nearestDist) && currentDistance <= searchRadius) {
				nearestIndex = i;
				nearestDist = currentDistance;
			}
		}
		return nearestIndex;
	}
	Wave getWave(int number) {
		if (number < (int)waves.size()) {
			return waves.at(number);
		} else {
			Wave wave = {"", {}};
			for (int i = 0; i < number; i++) {
				if (number < 100) {
					wave.entities.push_back({i % 3 == 0 ? ENTITY_MONSTER : (i % 3 == 1 ? ENTITY_TWIN : ENTITY_TUNGSTEN_MAIDEN), 0.05f});
				} else if (number < 200) {
					wave.entities.push_back({i % 2 == 0 ? ENTITY_GENERAL : ENTITY_TUNGSTEN_MAIDEN, 0.05f});
				} else {
					int typeToAdd;
					switch (i % 5) {
						case 0:
						typeToAdd = ENTITY_ADMIN; break;
						case 1:
						typeToAdd = ENTITY_TUNGSTEN_MAIDEN; break;
						case 2:
						typeToAdd = ENTITY_TROJANHORSE; break;
						case 3:
						typeToAdd = ENTITY_GENERAL; break;
						case 4:
						typeToAdd = ENTITY_TWIN; break;
					}
					wave.entities.push_back({typeToAdd, 0.03f});
				}
			}
			return wave;
		}
	}
	void upgradePerson(Person* person) {
		vector<PersonUpgrade> upgrades = person->getUpgrades();
		if ((person->stats.level < (int)upgrades.size() - 1 && world.money >= upgrades.at(person->stats.level + 1).price)) {
			soundDoer.play(soundDoer.sounds[SOUND_BUTTON]);
			world.money -= upgrades.at(person->stats.level + 1).price;
			person->stats.price += upgrades.at(person->stats.level + 1).price;
			person->stats.level++;
			switch (person->type) { // make level x+1 dpspp > level x dpspp
			case PERSON_ARCHER://sharpened arrows: go through many, scope: more range, crossbow: faster and damage
				if (person->stats.level == 1) {
					person->stats.projectile.health = 5.f;
					person->stats.projectile.damage *= 1.5f;
				} else if (person->stats.level == 2) {
					person->stats.range *= 2.f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.6f;
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
					person->stats.shootDelay = 0.02f;
				}
				break;
			case PERSON_TURRET: // mega fast: fast
				if (person->stats.level == 1) {
					person->stats.shootDelay *= 0.6f;
				} else if (person->stats.level == 2) {
					person->stats.projectile.damage *= 1.3f;
					person->stats.projectile.health = 2.f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.1f;
				}
				break;
			case PERSON_TANK: // them
				if (person->stats.level == 1) {
					person->stats.projectile.damage *= 1.3;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.8f;
					person->stats.projectile.guided = true;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.008f;
				}
				break;
			case PERSON_GOLD_MINE: // them
				if (person->stats.level == 1) {
					person->stats.incomeDelay *= 0.8f;
				} else if (person->stats.level == 2) {
					person->stats.incomeDelay *= 0.8f;
				} else if (person->stats.level == 3) {
					person->stats.incomeDelay *= 0.8f;
					person->stats.income *= 2.f;
				}
				break;
			case PERSON_BATTERY: // them
				if (person->stats.level == 1) {
					person->stats.range *= 1.5f;
					person->stats.projectile.damage *= 2.f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.7f;
				} else if (person->stats.level == 3) {
					// make silent
				} else if (person->stats.level == 4) {
					person->stats.shootDelay *= 0.025f * 2.f;
					person->stats.projectile.damage *= 2.f;
				}
				break;
			case PERSON_SCIENTIST: // them
				if (person->stats.level == 1) {
					person->stats.range *= 1.5f;
					person->stats.shootDelay *= 0.7f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.7f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 0.2f;
				}
				break;
			case PERSON_ROBOT: // them
				if (person->stats.level == 1) {
					person->stats.range *= 1.2f;
					person->stats.shootDelay *= 0.6f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.7f;
					person->stats.projectile.damage *= 1.2f;
				} else if (person->stats.level == 3) {
					person->stats.shootDelay *= 5.f;
					person->stats.projectile.damage *= 8.f;
					person->stats.range *= 2.f;
				} else if (person->stats.level == 4) {
					person->stats.shootDelay *= 0.2f;
				}
				break;
			case PERSON_SCARECROW: // them
				if (person->stats.level == 1) {
					person->stats.range *= 1.5f;
					person->stats.projectile.damage *= 1.5f;
				} else if (person->stats.level == 2) {
					person->stats.shootDelay *= 0.5f;
					person->stats.projectile.damage *= 1.2f;
				} else if (person->stats.level == 3) {
					person->stats.projectile.rewardMultiplier *= 2.f;
				} else if (person->stats.level == 4) { // last straw
					person->stats.projectile.rewardMultiplier *= 2.f;
					person->stats.shootDelay *= 0.2f;
					person->stats.projectile.damage *= 8.f;
					person->stats.projectile.health = 3.f;
				}
				break;
			}
		}
	}
	void sellSelectedPerson() {
		for (int i = 0; i < (int)world.people.size(); i++) {
			if (world.people.at(i).selected) {
				soundDoer.play(soundDoer.sounds[SOUND_MONEY]);
				world.money += world.people.at(i).stats.price * 0.9f;
				world.people.erase(world.people.begin() + i);
				soundDoer.play(soundDoer.sounds[SOUND_BUTTON]);
				break;
			}
		}
	}
	void playButton() {
		if (waveEnded()) {
			spawnWave(getWave(world.waveNumber));
			soundDoer.play(soundDoer.sounds[SOUND_BUTTON]);
			world.waveNumber++;
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
	{PERSON_SCIENTIST, 6.f, 2.f},
	{PERSON_ROBOT, 7.f, 2.f},
	{PERSON_SCARECROW, 7.f, 2.f},
};
string getFileText(string path) {
	ifstream file{path};
	if (!file.is_open()) {
		return "";
		cerr << "ERROR: file \"" << path << "\" not found" << endl;
		file.close();
	}
	string text = "";
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
class Shader {
	public:
	GLuint shader;
	Shader(string fileName, int shaderType) {

		string shaderText = getFileText(fileName);
		
		const char* text = shaderText.c_str();
		shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &text, NULL);
		glCompileShader(shader);

		// erors
		GLint success = GL_FALSE;  
		GLint infoLogLength;
		char infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		if (infoLogLength > 0) {
			cerr << "ERROR: " << infoLog << endl;
		}
		if (!success){
			glDeleteShader(shader);
		} else cout << "INFO: Successfuly loaded " << (shaderType == GL_VERTEX_SHADER ? "vertex shader" : "fragment shader") << " \"" << fileName << "\"" << endl;
	}
};
class Material {
	public:
		GLint mvp_location, texture1_location, vpos_location, vtexcoord_location;
		GLuint program;

		int textureWidth, textureHeight, textureColorChannels;
		unsigned char* textureBytes;
		unsigned int texture;
		string name = "";

		Material(string namea, GLuint vertexShader, GLuint fragmentShader, string textureFile) {
			name = namea;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
		}
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
		vector<vector<vector<unsigned int>>> faceses = {};
		
		int getMatID(vector<Material, allocator<Material>>* materials, string name) {
			int size = (int)materials->size();
			for (int i = 0; i < size; i++) {
				if (name == materials[0][i].name) return i;
			}
			return 0;
		}
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
			cout << "INFO: successfully loaded obj file \"" << path << "\"" << endl;

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
							if (wordNumber == 1) matId = getMatID(materials, word);
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
				bool good = i < (int)obj.texcoords.size();
				vertices.push_back({obj.vertices[i].x, obj.vertices[i].y, obj.vertices[i].z, good ? obj.texcoords[i].x : 0.f, good ? obj.texcoords[i].y : 0.f, 1.f});
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

	Shader solidV{"resources/shader/solid.vsh", GL_VERTEX_SHADER};
	Shader guiV{"resources/shader/gui.vsh", GL_VERTEX_SHADER};
	Shader fontV{"resources/shader/font.vsh", GL_VERTEX_SHADER};

	Shader solidF{"resources/shader/solid.fsh", GL_FRAGMENT_SHADER};
	Shader guiF{"resources/shader/gui.fsh", GL_FRAGMENT_SHADER};
	Shader worldUvRandomF{"resources/shader/world_uv_random.fsh", GL_FRAGMENT_SHADER};
	Shader worldUvF{"resources/shader/world_uv.fsh", GL_FRAGMENT_SHADER};
	Shader guiGrayscaleF{"resources/shader/gui_grayscale.fsh", GL_FRAGMENT_SHADER};
	Shader rangeBlueF{"resources/shader/range_blue.fsh", GL_FRAGMENT_SHADER};
	Shader rangeGreenF{"resources/shader/range_green.fsh", GL_FRAGMENT_SHADER};
	Shader rangeRedF{"resources/shader/range_red.fsh", GL_FRAGMENT_SHADER};
	Shader electricityF{"resources/shader/electricity.fsh", GL_FRAGMENT_SHADER};
	Shader worldUvRandomXyF{"resources/shader/world_uv_random_xy.fsh", GL_FRAGMENT_SHADER};
	Shader worldUvRandomYzF{"resources/shader/world_uv_random_yz.fsh", GL_FRAGMENT_SHADER};

	vector<Material> materials = {
		{"solid"					  , solidV.shader, solidF.shader		  , "resources/texture/brain.png"},
		{"gui"						, guiV.shader  , guiF.shader			, "resources/texture/screen.png"},
		{"grass"					  , solidV.shader, worldUvRandomF.shader  , "resources/texture/grass.png"},
		{"stone"					  , solidV.shader, solidF.shader		  , "resources/texture/stone.png"},
		{"wood"					   , solidV.shader, solidF.shader		  , "resources/texture/wood.png"},
		{"skin"					   , solidV.shader, solidF.shader		  , "resources/texture/skin.png"},
		{"metal"					  , solidV.shader, solidF.shader		  , "resources/texture/metal.png"},
		{"gold"					   , solidV.shader, solidF.shader		  , "resources/texture/gold.png"},
		{"camo"					   , solidV.shader, solidF.shader		  , "resources/texture/camo.png"},
		{"rock"					   , solidV.shader, solidF.shader		  , "resources/texture/rock.png"},
		{"snow"					   , solidV.shader, solidF.shader		  , "resources/texture/snow.png"},
		{"ice"					   , solidV.shader, solidF.shader		  , "resources/texture/ice.png"},
		{"metal_black"				, solidV.shader, solidF.shader		  , "resources/texture/metal_black.png"},
		{"straw"					  , solidV.shader, solidF.shader		  , "resources/texture/straw.png"},

		{"fire"					   , solidV.shader, solidF.shader		  , "resources/texture/fire.png"},
		{"electricity"				, solidV.shader, electricityF.shader	, "resources/texture/icon_battery.png"},
		{"rock_world_uv_random_xy"	, solidV.shader, worldUvRandomXyF.shader, "resources/texture/rock.png"},
		{"rock_world_uv_random_yz"	, solidV.shader, worldUvRandomYzF.shader, "resources/texture/rock.png"},

		{"range_blue"				 , solidV.shader, rangeBlueF.shader	  , "resources/texture/brain.png"},
		{"range_green"				, solidV.shader, rangeGreenF.shader	 , "resources/texture/brain.png"},
		{"range_red"				  , solidV.shader, rangeRedF.shader	   , "resources/texture/brain.png"},

		{"gui_container"			  , guiV.shader  , guiF.shader			, "resources/texture/gui_container.png"},
		{"gui_button"				 , guiV.shader  , guiF.shader			, "resources/texture/button.png"},
		{"gui_button_disabled"		, guiV.shader  , guiGrayscaleF.shader   , "resources/texture/button.png"},
		{"gui_icon_archer"			, guiV.shader  , guiF.shader			, "resources/texture/icon_archer.png"},
		{"gui_icon_cannon"			, guiV.shader  , guiF.shader			, "resources/texture/icon_cannon.png"},
		{"gui_icon_turret"			, guiV.shader  , guiF.shader			, "resources/texture/icon_turret.png"},
		{"gui_icon_tank"			  , guiV.shader  , guiF.shader			, "resources/texture/icon_tank.png"},
		{"gui_icon_gold_mine"		 , guiV.shader  , guiF.shader			, "resources/texture/icon_gold_mine.png"},
		{"gui_icon_battery"		   , guiV.shader  , guiF.shader			, "resources/texture/icon_battery.png"},
		{"gui_icon_scientist"		 , guiV.shader  , guiF.shader,			 "resources/texture/icon_scientist.png"},
		{"gui_icon_robot"			 , guiV.shader  , guiF.shader,			 "resources/texture/icon_robot.png"},
		{"gui_icon_scarecrow"		 , guiV.shader  , guiF.shader,			 "resources/texture/icon_scarecrow.png"},
		{"gui_button_upgrade"		 , guiV.shader  , guiF.shader			, "resources/texture/button_upgrade.png"},
		{"gui_button_upgrade_disabled", guiV.shader  , guiGrayscaleF.shader   , "resources/texture/button_upgrade.png"},
		{"gui_play"				   , guiV.shader  , guiF.shader			, "resources/texture/button_play.png"},
		{"gui_play_disabled"		  , guiV.shader  , guiGrayscaleF.shader   , "resources/texture/button_play.png"},
		{"icon_level"				 , guiV.shader  , guiF.shader			, "resources/texture/icon_level.png"},
		{"gui_font"				   , fontV.shader , guiF.shader			, "resources/texture/font.png"}
	};

	vector<Vertex> vertices = {};
	vector<vector<unsigned int>> indiceses = {};
	
	Mesh turretMesh{&materials, "resources/model/person/turret.obj"};
	Mesh cannonMesh{&materials, "resources/model/person/cannon.obj"};
	Mesh archerMesh{&materials, "resources/model/person/archer.obj"};
	Mesh tankMesh{&materials, "resources/model/person/tank.obj"};
	Mesh goldMineMesh{&materials, "resources/model/person/goldmine.obj"};
	Mesh batteryMesh{&materials, "resources/model/person/battery.obj"};
	Mesh scientistMesh{&materials, "resources/model/person/scientist.obj"};
	Mesh robotMesh{&materials, "resources/model/person/robot.obj"};
	Mesh scarecrowMesh{&materials, "resources/model/person/scarecrow.obj"};

	Mesh entityTemplateMesh{&materials, "resources/model/entity/normal.obj"};
	Mesh normalMesh{&materials, "resources/model/entity/normal.obj"};
	Mesh fastMesh{&materials, "resources/model/entity/fast.obj"};
	Mesh monsterMesh{&materials, "resources/model/entity/monster.obj"};
	Mesh generalMesh{&materials, "resources/model/entity/general.obj"};
	Mesh trojanHorseMesh{&materials, "resources/model/entity/trojanhorse.obj"};
	Mesh ironMaidenMesh{&materials, "resources/model/entity/ironmaiden.obj"};
	Mesh tungstenMaidenMesh{&materials, "resources/model/entity/tungstenmaiden.obj"};
	Mesh twinMesh{&materials, "resources/model/entity/twin.obj"};

	Mesh missileMesh{&materials, "resources/model/projectile/missile.obj"};
	Mesh cannonballMesh{&materials, "resources/model/projectile/cannonball.obj"};
	Mesh sparkMesh{&materials, "resources/model/projectile/spark.obj"};
	Mesh electricityMesh{&materials, "resources/model/projectile/electricity.obj"};
	Mesh potionMesh{&materials, "resources/model/projectile/potion.obj"};

	Mesh planeMesh{&materials, "resources/model/plane.obj"};

	int getMatID(string name) {
		int size = (int)materials.size();
		for (int i = 0; i < size; i++) {
			if (name == materials[i].name) {
				return i;
			}
		}
		return 0;
	}
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

		if (game->gameStatus == 0) {
			// ground
			for (int x = -10; x < game->world.level.size.x + 10; x++) {
				for (int z = -10; z < game->world.level.size.z + 10; z++) {
					if (x < 0 || z < 0 || x >= game->world.level.size.x || z >= game->world.level.size.z) {
						addPlane((float)x, game->world.level.size.y, (float)z, 1.f, 1.f, false, getMatID(game->world.level.groundMaterial));
					}
				}
			}
			float sx = game->world.level.size.x;
			float sy = game->world.level.size.y;
			float sz = game->world.level.size.z;
			addQuad({0.f, sy , 0.f}, {0.f, sy, sz }, {0.f, 0.f, sz }, {0.f, 0.f, 0.f}, getMatID(game->world.level.wallMaterial));
			addQuad({sx , sy , sz }, {sx , sy, 0.f}, {sx , 0.f, 0.f}, {sx , 0.f, sz }, getMatID(game->world.level.wallMaterial));
			addQuad({sx , sy, 0.f}, {0.f, sy , 0.f}, {0.f, 0.f, 0.f}, {sx , 0.f, 0.f}, getMatID(game->world.level.wallMaterial));
			addQuad({0.f, sy, sz }, {sx , sy , sz }, {sx , 0.f, sz }, {0.f, 0.f, sz }, getMatID(game->world.level.wallMaterial));
			addPlane(0.f, 0.f, 0.f, game->world.level.size.x, game->world.level.size.z, true, getMatID(game->world.level.groundMaterial));
			// point path
			for (int i = 0; i < (int)game->world.level.path.size(); i++) {
				if (i < (int)game->world.level.path.size() - 1) {
					addPath(game->world.level.path[i], game->world.level.path[i + 1], game->world.level.pathWidth + 0.1f, 0.01f, getMatID(game->world.level.pathBaseMaterial));
					addPath(game->world.level.path[i], game->world.level.path[i + 1], game->world.level.pathWidth, 0.02f, getMatID(game->world.level.pathMaterial));
				}
				addDisc(vec3Add(game->world.level.path[i], {0.f, 0.01f, 0.f}), (game->world.level.pathWidth + 0.1f) / 2.f, 15, getMatID(game->world.level.pathBaseMaterial));
				addDisc(vec3Add(game->world.level.path[i], {0.f, 0.02f, 0.f}), game->world.level.pathWidth / 2.f, 15, getMatID(game->world.level.pathMaterial));
			}
			// entities
			for (Entity entity : game->world.entities) {
				switch (entity.type) {
				case ENTITY_ENTITYTEMPLATE:
					addMesh(entityTemplateMesh, entity.pos, {.5f, .8f, .5f}, entity.yRotation);
					break;
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
				case ENTITY_TROJANHORSE:
					addMesh(trojanHorseMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation);
					break;
				case ENTITY_IRON_MAIDEN:
					addMesh(ironMaidenMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation);
					break;
				case ENTITY_TUNGSTEN_MAIDEN:
					addMesh(tungstenMaidenMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation);
					break;
				case ENTITY_TWIN:
					addMesh(twinMesh, entity.pos, {1.f, 1.f, 1.f}, entity.yRotation);
					break;
				default:
					addCube(entity.pos.x - entity.size.x / 2.f, entity.pos.y, entity.pos.z - entity.size.z / 2.f, entity.size.x, entity.size.y, entity.size.z, 0);
					break;
				}
				addPlane(entity.pos.x, entity.pos.y + entity.size.y + 0.25f, entity.pos.z - 0.5f, 0.2f, 1.f, false, getMatID("metal_black"));
				
				addPlane(entity.pos.x, entity.pos.y + entity.size.y + 0.26f, entity.pos.z - 0.5f, 0.2f, entity.health / entity.maxHealth, false, getMatID("gold"));
			}
			//people
			for (Person person : game->world.people) {
				addPerson(&person, false);
			}
			if (game->isPlacingPerson) {
				addPerson(&game->placingPerson, true);
			}
			//projectiles
			for (Projectile projectile : game->world.projectiles) {
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
					addArc(projectile.origin, projectile.pos);
					break;
				case PROJECTILE_POTION:
					addMesh(potionMesh, projectile.pos, {1.f, 1.f, 1.f}, atan2(projectile.velocity.z, projectile.velocity.x));
					break;
				default:
					addPath(projectile.pos, vec3Add(projectile.pos, projectile.velocity), 0.1f, 0.f, 1);
				}
			}


			// clip space: smaller z is in front

			addText("HEALTH: " + to_string((int)game->world.health), -0.67f, 0.8f, 0.1f, 0.07f, 0.8f, 2.f, false);
			addText("WAVE " + to_string(game->world.waveNumber), -0.3f, -0.95f, 0.1f, 0.07f, 0.8f, 2.f, false);
			addText("$" + commas(game->world.money), -0.25f, 0.5f, 0.1f, 0.07f, 0.8f, 14.f * 0.07f * 0.8f, false);
			addText("fps:" + ftos(1.f / (float)(frameTime), 2), 0.f, 0.8f, 0.1f, 0.07f, 0.8f, 2.f, false);
			if (debug) {
				addText("ents:" + to_string((int)game->world.entities.size()), -0.7f, 0.7f, 0.1f, 0.07f, 0.8f, 2.f, false);
				addText("prjs:" + to_string((int)game->world.projectiles.size()), -0.7f, 0.55f, 0.1f, 0.07f, 0.8f, 2.f, false);
				addText("snds:" + to_string((int)soundDoer.buffers.size()), -0.7f, 0.4f, 0.1f, 0.07f, 0.8f, 2.f, false);
			}
			/*if (game->world.waveNumber == game->waves.size() && game->waveEnded() && game->health > 0.f) {
				addText("YOU WON", -0.9f, -0.9f, 0.1f, 0.2f, 0.8f);
			}*/
			addRect(0.7f, -1.f, 0.25f, 0.3f, 2.f, getMatID("gui_container")); // right
			addRect(0.725f, -0.95f, 0.2f, 0.25f, 0.25f, game->waveEnded() ? getMatID("gui_play") : getMatID("gui_play_disabled")); // play button
			for (Person person : game->world.people) {
				if (person.selected) {
					addRect(0.75f, 0.55f, 0.2f, 0.15f, 0.15f, (person.stats.level < (int)person.getUpgrades().size() - 1 && game->world.money >= person.getUpgrades()[person.stats.level + 1].price) ? getMatID("gui_button_upgrade") : getMatID("gui_button_upgrade_disabled"));
					vector<PersonUpgrade> upgrades = person.getUpgrades();
					bool isMax = person.stats.level > (int)upgrades.size() - 2;
					string upgradeText = isMax ? "max upgrades" : upgrades[person.stats.level + 1].name + ": $" + commas(upgrades[person.stats.level + 1].price);
					addText(upgradeText, 0.72f, 0.5f, 0.1f, 0.02f, 0.8f, 0.27f, false);
					addRect(0.75f, 0.38f, 0.2f, 0.25f, 0.08f, getMatID("gui_button"));
					addText("sell ($" + commas(person.stats.price * 0.9f) + ")", 0.75f, 0.39f, 0.1f, 0.02f, 0.8f, 0.25f, false);
					float dps = person.stats.projectile.damage / person.stats.shootDelay * person.stats.projectile.health;
					//float rangeDps = dps * (PI * powf(person.stats.range, 2.f));
					addText("dps: " + ftos(dps, 2), 0.72f, 0.32f, 0.1f, 0.02f, 0.8f, 2.f, false);
					addText("dpspp: " + ftos(dps / person.stats.price / (person.stats.size.x * person.stats.size.z), 2), 0.72f, 0.28f, 0.1f, 0.02f, 0.8f, 2.f, false);
					break;
				}
			}
			addRect(-1.f, -1.f, 0.25f, 0.3f, 2.f, getMatID("gui_container")); // people placing buttons
			for (int i = 0; i < (int)personButtons.size(); i++) {
				float x = -0.98f + (0.15f * fmod(i, 2.f));
				float y = 0.7f - floor(i / 2.f) * 0.15f;
				PersonStats personButtonStats = {personButtons[i].type};
				if (personButtons[i].type == PERSON_TANK && !game->world.tankUnlocked) continue;
				addRect(x, y, 0.206f, 0.13f, 0.13f, game->world.money >= personButtonStats.price ? getMatID("gui_button") : getMatID("gui_button_disabled"));
				int matId;
				switch (personButtons[i].type) {
				case PERSON_ARCHER:
					matId = getMatID("gui_icon_archer");
					break;
				case PERSON_CANNON:
					matId = getMatID("gui_icon_cannon");
					break;
				case PERSON_TURRET:
					matId = getMatID("gui_icon_turret");
					break;
				case PERSON_TANK:
					matId = getMatID("gui_icon_tank");
					break;
				case PERSON_GOLD_MINE:
					matId = getMatID("gui_icon_gold_mine");
					break;
				case PERSON_BATTERY:
					matId = getMatID("gui_icon_battery");
					break;
				case PERSON_SCIENTIST:
					matId = getMatID("gui_icon_scientist");
					break;
				case PERSON_ROBOT:
					matId = getMatID("gui_icon_robot");
					break;
				case PERSON_SCARECROW:
					matId = getMatID("gui_icon_scarecrow");
					break;
				}
				if (!game->isPlacingPerson || game->placingPerson.type != personButtons[i].type) {
					addRect(x, y, 0.205f, 0.13f, 0.13f, matId);
				}
				PersonStats stats{personButtons[i].type};
				addText("$" + to_string((long long)stats.price), x, y - 0.02f, 0.1f, 0.03f, 0.7f, 2.f, false);
			}
		} else if (game->gameStatus == 1) {
			addRect(-.9f, -.9f, 0.3f, 1.8f, 1.6f, getMatID("gui_container"));
			addText("LEVEL SELECT", -0.5f, 0.7f, 0.1f, 0.1f, 0.9f, 2.f, false);
			for (int i = 0; i < (int)levels.size(); i++) {
				int x = i % 4;
				int y = (int)floor((float)i / 4.f);
				addRect(-.8f + x * 0.36f, 0.6f - 0.36f * ((float)y + 1.f), 0.29f, 0.36f, 0.36f, getMatID("gui_button"));
				addRect(-.8f + x * 0.36f + 0.06f, 0.6f - 0.36f * ((float)y + 1.f) + 0.06f, 0.28f, 0.36f - 0.12f, 0.36f - 0.12f, getMatID("icon_level"));
				addText(levels[i].name, -.85f + x * 0.36f + 0.29f / 2.f, 0.6f - 0.36f * ((float)y + 1.f), 0.1f, 0.05f, 0.8f, 0.3f, true);
			}
		}
		if (game->messageTime > 0.f) {
			addRect(-0.5f, -0.9f, 0.2f, 1.f, 0.5f, getMatID("gui_container"));
			addText(game->message, -0.45f, -0.6f, 0.1f, 0.07f, 0.7f, 0.9f, false);
		}
		
		if (debug) {
			int tris = 0;
			int verts = (int)vertices.size();
			for (int i = 0; i < (int)indiceses.size(); i++) {
				tris += (int)indiceses[i].size() / 3;
			}
			addText("tris:" + to_string(tris), 0.f, 0.1f, 0.1f, 0.07f, 0.8f, 2.f, false);
			addText("verts:" + to_string(verts), 0.f, 0.0f, 0.1f, 0.07f, 0.8f, 2.f, false);
		}
	}
	void renderMaterials(int width, int height) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);

		buildThem(width, height);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_DYNAMIC_DRAW);
		for (int i = 0; i < (int)materials.size(); i++) {
			renderMaterial(i, width, height);
		}
	}
	void renderMaterial(int id, int width, int height) {
		if (indiceses.at(id).size() == 0) return; // skip if no faces

		glEnableVertexAttribArray(materials.at(id).vpos_location);
		glVertexAttribPointer(materials.at(id).vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
	
		glEnableVertexAttribArray(materials.at(id).vtexcoord_location);
		glVertexAttribPointer(materials.at(id).vtexcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 3));
		float ratio = (float)width / (float)height;
		mat4x4 m, p, mvp;

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceses.at(id).size() * sizeof(unsigned int), &indiceses.at(id)[0], GL_DYNAMIC_DRAW);
		
		glBindTexture(GL_TEXTURE_2D, materials.at(id).texture);

		mat4x4_identity(m);
		mat4x4_translate(m, -game->world.camera.pos.x, -game->world.camera.pos.y, -game->world.camera.pos.z);
		mat4x4_rotate_X(m, m, game->world.camera.rotation.x);
		mat4x4_rotate_Y(m, m, game->world.camera.rotation.y);
		mat4x4_rotate_Z(m, m, game->world.camera.rotation.z);
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_perspective(p, game->world.camera.fov, ratio, 0.1f, 200.f);
		mat4x4_mul(mvp, p, m);

		glUniform1i(materials.at(id).texture1_location, 0);
		glUseProgram(materials.at(id).program);
		glUniformMatrix4fv(materials.at(id).mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawElements(GL_TRIANGLES, indiceses.at(id).size(), GL_UNSIGNED_INT, (void*)0);
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
		case PERSON_SCIENTIST:
			addMesh(scientistMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		case PERSON_ROBOT:
			addMesh(robotMesh, person->pos, {1.f, 1.f, 1.f}, person->yRotation);
			break;
		case PERSON_SCARECROW:
			addMesh(scarecrowMesh, person->pos, {1.f, person->stats.level < 4 ? 1.f : 2.f, 1.f}, person->yRotation);
			break;
		default:
			addCube(person->pos.x - person->stats.size.x / 2.f, person->pos.y, person->pos.z - person->stats.size.z / 2.f, person->stats.size.x, person->stats.size.y, person->stats.size.z, 0);
			break;
		}
		if (isPlacingPerson) {
			if (game->world.isPersonPlacable(&game->placingPerson)) {
				addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, false, getMatID("range_green"));
			} else {
				addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, false, getMatID("range_red"));

			}
		} else if (person->selected) {
			addPlane(person->pos.x - person->stats.range, person->pos.y + 0.1f, person->pos.z - person->stats.range, person->stats.range * 2.f, person->stats.range * 2.f, false, getMatID("range_blue"));
		}
	}

	void addCube(float x, float y, float z, float w, float h, float d, int matId) {
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
			2U+end, 7U+end, 6U+end,
			2U+end, 3U+end, 7U+end,

			0U+end, 5U+end, 4U+end,
			0U+end, 1U+end, 5U+end,

			0U+end, 6U+end, 4U+end,
			0U+end, 2U+end, 6U+end,

			1U+end, 7U+end, 3U+end,
			1U+end, 5U+end, 7U+end,

			0U+end, 3U+end, 2U+end,
			0U+end, 1U+end, 3U+end,

			4U+end, 6U+end, 7U+end,
			4U+end, 7U+end, 5U+end
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
	void addPlane(float x, float y, float z, float w, float d, bool worldUv, int matId) {
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 1U+end, 2U+end,
			1U+end, 3U+end, 2U+end
		});
		vertices.insert(vertices.end(), {
			{x  , y , z+d, 0.f, 0.f, 1.f},
			{x+w, y , z+d, worldUv ? w : 1.f, 0.f, 1.f},
			{x  , y , z  , 0.f, worldUv ? d : 1.f, 1.f},
			{x+w, y , z  , worldUv ? w : 1.f, worldUv ? d : 1.f, 1.f}
		});
	}
	void addQuad(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4, int matId) { // clockwise
		unsigned int end = vertices.size();
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 1U+end, 2U+end,
			0U+end, 2U+end, 3U+end
		});
		vertices.insert(vertices.end(), {
			{p1.x, p1.y, p1.z, 0.f, 0.f, 1.f},
			{p2.x, p2.y, p2.z, 1.f, 0.f, 1.f},
			{p3.x, p3.y, p3.z, 0.f, 1.f, 1.f},
			{p4.x, p4.y, p4.z, 1.f, 1.f, 1.f}
		});
	}
	void addPath(Vec3 p1, Vec3 p2, float width, float yOffset, int matId) {
		unsigned int end = vertices.size();
		float dist = distance3D(p1, p2);
		float xDiff = (p2.x - p1.x) / dist * width / 2.f;
		float zDiff = (p2.z - p1.z) / dist * width / 2.f;
		float endfake = 0.f;
		vertices.insert(vertices.end(), {
			{p1.x - zDiff - xDiff * endfake, p1.y + yOffset, p1.z + xDiff - zDiff * endfake, dist / width, 0.f, 1.f},
			{p1.x + zDiff - xDiff * endfake, p1.y + yOffset, p1.z - xDiff - zDiff * endfake, dist / width, 1.f, 1.f},
			{p2.x - zDiff + xDiff * endfake, p2.y + yOffset, p2.z + xDiff + zDiff * endfake, 0.f,		  0.f, 1.f},
			{p2.x + zDiff + xDiff * endfake, p2.y + yOffset, p2.z - xDiff + zDiff * endfake, 0.f,		  1.f, 1.f}
		});
		indiceses[matId].insert(indiceses[matId].end(), {
			0U+end, 2U+end, 1U+end,
			1U+end, 2U+end, 3U+end
		});
	}
	void addMesh(Mesh mesh, Vec3 position, Vec3 scale, float yRotation) {
		unsigned int end = vertices.size();
		for (int i = 0; i < (int)mesh.vertices.size(); i++) {
			mesh.vertices[i].x *= scale.x;
			mesh.vertices[i].y *= scale.y;
			mesh.vertices[i].z *= scale.z;
			if (yRotation != 0.f) {
				float x = mesh.vertices[i].x;
				float z = mesh.vertices[i].z;
				mesh.vertices[i].x = x * cos(yRotation) - z * sin(yRotation);
				mesh.vertices[i].z = z * cos(yRotation) + x * sin(yRotation);
			}
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
		int i = 0;
		while (distance3D(pos, start) > 0.1f) {
			if (i >= 100) break;
			i++;
			Vec3 dir = vec3Add({randFloat() * 0.2f - 0.1f, 0.f, randFloat() * 0.2f - 0.1f}, vec3Subtract(start, pos).normalise(0.1f));//random + start - end
			addMesh(electricityMesh, pos, {0.2f, 1.f, 1.f}, atan2(dir.z, dir.x));
			pos = vec3Add(pos, dir);
		}
	}
	void addDisc(Vec3 pos, float r, int sides, int matID) {
		if (sides < 3) return;
		vector<Vertex> discVertices = {{pos.x, pos.y, pos.z, pos.x, pos.z}};
		vector<unsigned int> discIndices = {};
		unsigned int end = vertices.size();
		for (int i = 0; i < sides; i++) {
			float d = i / (float)sides * PI * 2.f;
			float x = sin(d) * r + pos.x;
			float z = cos(d) * r + pos.z;
			discVertices.push_back({x, pos.y, z, x, z});
		}
		for (int i = 1; i < (int)discVertices.size(); i++) {
			discIndices.push_back(0U + end);
			discIndices.push_back(i + end);
			discIndices.push_back((i % ((int)discVertices.size() - 1)) + 1U + end);
		}
		vertices.insert(vertices.end(), discVertices.begin(), discVertices.end());
		indiceses[matID].insert(indiceses[matID].end(), discIndices.begin(), discIndices.end());
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
		addRect(x, y, z, w, w * aspect, getMatID("gui_font"), uv);
	}
	void addText(string text, float x, float y, float z, float w, float spacing, float maxWidth, bool centered) {
		for (int i = 0; i < (int)text.length(); i++) {
			float noWrapX = (float)i * w * spacing;
			float wrapX = fmod(noWrapX, maxWidth);
			if (centered) wrapX -= maxWidth / 4.f;
			addCharacter(text.at(i), wrapX + x, y - floor(noWrapX / maxWidth) * w * 1.6f, z, w * 1.5f);
			z -= 0.001;
		}
	}
};
GameState game;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE && (mods & GLFW_MOD_CONTROL)) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		else if (key == GLFW_KEY_W) controls.w = true;
		else if (key == GLFW_KEY_A) controls.a = true;
		else if (key == GLFW_KEY_S) controls.s = true;
		else if (key == GLFW_KEY_D) controls.d = true;
		else if (key == GLFW_KEY_LEFT) controls.left = true;
		else if (key == GLFW_KEY_RIGHT) controls.right = true;
		else if (key == GLFW_KEY_UP) controls.up = true;
		else if (key == GLFW_KEY_DOWN) controls.down = true;
		else if (key == GLFW_KEY_P) controls.p = true;
		else if (key == GLFW_KEY_RIGHT_BRACKET) controls.fast = true;
		else if (key == GLFW_KEY_LEFT_BRACKET) controls.slow = true;
		else if (key == GLFW_KEY_LEFT_SHIFT) controls.shift = true;
		else if (key == GLFW_KEY_SPACE) game.playButton();
		else if (key == GLFW_KEY_BACKSPACE) game.sellSelectedPerson();
		else if (key == GLFW_KEY_F3) debug = !debug;
		else if (key == GLFW_KEY_U) {
			for (int i = 0; i < (int)game.world.people.size(); i++) {
				if (game.world.people[i].selected) {
					game.upgradePerson(&game.world.people[i]);
					break;
				}
			}
		} else if (key == GLFW_KEY_K) {
			game.world.level.path.clear();
		} else if (key == GLFW_KEY_L) {
			game.world.level.path.push_back({controls.worldMouse.y, 0.f, controls.worldMouse.x});
		} else if (key == GLFW_KEY_F4) {
			exportLevel(game.world.level, "levels/level.htdlvl");
			game.showMessage("level saved", 1.f);
		} else if (key == GLFW_KEY_X) {
			game.world.level.path.pop_back();
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) controls.w = false;
		else if (key == GLFW_KEY_A) controls.a = false;
		else if (key == GLFW_KEY_S) controls.s = false;
		else if (key == GLFW_KEY_D) controls.d = false;
		else if (key == GLFW_KEY_LEFT) controls.left = false;
		else if (key == GLFW_KEY_RIGHT) controls.right = false;
		else if (key == GLFW_KEY_UP) controls.up = false;
		else if (key == GLFW_KEY_DOWN) controls.down = false;
		else if (key == GLFW_KEY_P) controls.p = false;
		else if (key == GLFW_KEY_RIGHT_BRACKET) controls.fast = false;
		else if (key == GLFW_KEY_LEFT_BRACKET) controls.slow = false;
		else if (key == GLFW_KEY_LEFT_SHIFT) controls.shift = false;
	};
}
bool mouseIntersectsClipRect(float x, float y, float w, float h) {
	return controls.clipMouse.x > x && controls.clipMouse.x < x + w && controls.clipMouse.y > y && controls.clipMouse.y < y + h;
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		
		if (game.gameStatus == 0) {
			if (controls.clipMouse.x <= -0.7f || controls.clipMouse.x >= 0.7f) {
				for (int i = 0; i < (int)personButtons.size(); i++) {
					if (personButtons[i].type == PERSON_TANK && !game.world.tankUnlocked) continue;
					float rectLeft = -0.98f + (0.15f * fmod(i, 2.f));
					float rectRight = rectLeft + 0.13f;
					float rectBottom = 0.7f - floor(i / 2.f) * 0.15f;
					float rectTop = rectBottom + 0.13f;
					if (controls.clipMouse.x > rectLeft && controls.clipMouse.x < rectRight && controls.clipMouse.y > rectBottom && controls.clipMouse.y < rectTop) {
						PersonStats stats = {personButtons[i].type};
						if (game.world.money >= stats.price) {
							soundDoer.play(soundDoer.sounds[SOUND_BUTTON]);
							game.placingPerson = {personButtons[i].type, {2.f, 0.f, 2.f}};
							game.isPlacingPerson = true;
							for (int i = 0; i < (int)game.world.people.size(); i++) {
								game.world.people[i].selected = false;
							}
							//soundDoer.play(soundDoer.sounds[0]);
						}
					}
				}
				if (controls.clipMouse.x > 0.725f && controls.clipMouse.y < -0.7f && controls.clipMouse.x < 0.975f && controls.clipMouse.y > -0.95f) {
					game.playButton();
					return;
				}
				if (controls.clipMouse.x > 0.75f && controls.clipMouse.y > 0.55f && controls.clipMouse.x < 0.9f && controls.clipMouse.y < 0.7f) {
					for (int i = 0; i < (int)game.world.people.size(); i++) {
						if (game.world.people[i].selected) {
							game.upgradePerson(&game.world.people[i]);
							break;
						}
					}
					return;
				}
				if (controls.clipMouse.x > 0.75f && controls.clipMouse.y > 0.38f && controls.clipMouse.x < 1.f && controls.clipMouse.y < 0.46f) {
					game.sellSelectedPerson();
				}
			} else {
				bool placed = false;
				if (game.isPlacingPerson && game.world.money >= game.placingPerson.stats.price && game.world.isPersonPlacable(&game.placingPerson) && controls.clipMouse.x > -0.7f) {
					game.world.people.push_back(game.placingPerson);
					game.world.money -= game.placingPerson.stats.price;
					if (!controls.shift) game.isPlacingPerson = false;
					soundDoer.play(soundDoer.sounds[SOUND_PLACE]);
					placed = true;
				};
				bool personSelected = false;
				for (int i = 0; i < (int)game.world.people.size(); i++) {
					game.world.people[i].selected = !personSelected && !game.isPlacingPerson && (controls.worldMouse.x > game.world.people[i].pos.z - game.world.people[i].stats.size.z / 2.f && controls.worldMouse.x < game.world.people[i].pos.z + game.world.people[i].stats.size.z / 2.f && controls.worldMouse.y > game.world.people[i].pos.x - game.world.people[i].stats.size.x / 2.f && controls.worldMouse.y < game.world.people[i].pos.x + game.world.people[i].stats.size.x / 2.f);
					if (game.world.people[i].selected) {
						if (!placed) soundDoer.play(soundDoer.sounds[SOUND_BUTTON]);
						if (game.world.people[i].type == PERSON_ROBOT) soundDoer.play(soundDoer.sounds[SOUND_ROBOT]);
						personSelected = true;
					}
				}
			}
		} else if (game.gameStatus == 1) {
			for (int i = 0; i < (int)levels.size(); i++) {
				int x = i % 4;
				int y = (int)floor((float)i / 4.f);
				if (mouseIntersectsClipRect(-.8f + (float)x * 0.36f, 0.6f - 0.36f * (float)(y + 1), 0.36f, 0.36f)) {
					game.world = {levels[i]};
					soundDoer.play(soundDoer.sounds[game.world.level.music]);
					std::cout << game.world.level.size.x << ", " << game.world.level.size.y << ", " << game.world.level.size.z << std::endl;
					game.gameStatus = 0;
					game.world.camera.pos.x = game.world.level.size.z / 2.f;
					game.world.camera.pos.y = game.world.level.size.x / 2.f;
					game.world.camera.pos.z = game.world.level.size.y * 2.f; // up
				};
			}
		}

		controls.mouseDown = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
	   double xpos, ypos;
	   //getting cursor position
	   glfwGetCursorPos(window, &xpos, &ypos);
	   controls.mouseDown = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
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
	for (const auto & entry : filesystem::directory_iterator("levels")) {
		levels.push_back({entry.path().u8string()});
	}

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
		frameTime = (float)(newFrameTime - lastFrameTime);
		lastFrameTime = newFrameTime;
		int width, height;
		
		glfwGetFramebufferSize(window, &width, &height);

		game.d = (frameTime / 1000.f) / 5.f;
		for (int i = 0; i < ((controls.fast && controls.slow) ? 200 : (controls.slow ? 1 : (controls.fast ? 20 : 5))); i++) {
			game.tick(width, height);
		}

		renderer.renderMaterials(width, height);
		soundDoer.tickSounds();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	for (int i = 0; i < (int)renderer.materials.size(); i++) {
		glDeleteTextures(1, &renderer.materials[i].texture);
	}
	soundDoer.exit();
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
