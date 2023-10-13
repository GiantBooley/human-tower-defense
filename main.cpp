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
struct Vertex {
	float x, y, z;
	float u, v;
	float space;
};
vector<Vertex> vertices = {};
vector<unsigned int> indices = {};

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
/*void getShaderText() {
	string line,text;
	ifstream vshFile("vertex.vsh");
	if (vshFile.is_open()) {
		while(getline(vshFile, line)) {
			text += line + "\n";
		}
		vertex_shader_text = text; 
		cout << "successfully loaded vish file" << endl;
	} else {
		cout << "EROEROREOROEROROOREORORR: vertex shader file not found" << endl;
	};
	vshFile.close();
	
	text = "";
	ifstream fshFile("fragment.fsh");
	if (fshFile.is_open()) {
		while(getline(fshFile, line)) {
			text += line + "\n";
		}
		fragment_shader_text = text;
		cout << "successfully loaded fish file" << endl;
	} else {
		cout << "EREEREREROOERO: fish shader file not found" << endl;
	};
	fshFile.close();
}*/

class Keyboard {
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
	void normalise() {
		float magnitude = getMagnitude();
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}
};
Vec3 vec3Add(Vec3 a, Vec3 b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}
Vec3 vec3Add(Vec3 a, float b) {
	return {a.x + b, a.y + b, a.z + b};
}
Vec3 vec3Mul(Vec3 a, Vec3 b) {
	return {a.x * b.x, a.y * b.y, a.z * b.z};
}
Vec3 vec3Mul(Vec3 a, float b) {
	return {a.x * b, a.y * b, a.z * b};
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
class Entity {
public:
	Vec3 pos{0.f, 0.f, 0.f};
	Vec3 size{0.5f, 1.5f, 0.5f};
	int targetPoint = 0;
	float health = 1.f;
	Entity(Vec3 position) {
		pos = position;
	}
};
class Camera {
public:
	Vec3 pos{5.f, 6.f, 8.f};
	float fov = 1.57f;
	Vec3 rotation{1.3f, 1.57f, 0.f};
};
float distance3D(Vec3 p1, Vec3 p2) {
	return sqrt(pow(p2.x - p1.x, 2.f) + pow(p2.y - p1.y, 2.f) + pow(p2.z - p1.z, 2.f));
}
class Level {
public:
	Vec3 path[4] = {{2.f, 1.f, 0.f}, {5.f, 1.f, 2.f}, {2.f, 1.f, 8.f}, {3.f, 1.f, 10.f}};
};
Keyboard keyboard;
float roundToPlace(float x, float place) {
	return round(x / place) * place;
}
class GameState {
public:
	vector<Entity> entities = {};
	Camera camera;
	Level levels[1] = {{}};
	int activeLevel = 0;
	float health = 100.f;
	
	void tick() {
		if (keyboard.w) {
			camera.pos.z -= 0.1f;
		}
		if (keyboard.a) {
			camera.pos.x -= 0.1f;
		}
		if (keyboard.s) {
			camera.pos.z += 0.1f;
		}
		if (keyboard.d) {
			camera.pos.x += 0.1f;
		}
		if (keyboard.up) {
			camera.rotation.x += 0.01f;
		}
		if (keyboard.down) {
			camera.rotation.x -= 0.01f;
		}
		if (keyboard.left) {
			camera.rotation.y -= 0.01f;
		}
		if (keyboard.right) {
			camera.rotation.y += 0.01f;
		}
		if (keyboard.space) {
			spawnEntity();
		}
		for (int i = entities.size() - 1; i >= 0; i--) {
			bool die = false;
			Vec3 targetPoint = levels[activeLevel].path[entities[i].targetPoint];
			float distanceToTargetPoint = distance3D(entities[i].pos, targetPoint);
			if (distanceToTargetPoint < 0.03f) {
				entities[i].targetPoint++;
				if (entities[i].targetPoint >= sizeof(levels[activeLevel].path)/sizeof(levels[activeLevel].path[0])) {
					health -= 1.f;
					die = true;
				}
				targetPoint = levels[activeLevel].path[entities[i].targetPoint];
				distanceToTargetPoint = distance3D(entities[i].pos, targetPoint);
			}
			entities[i].pos.x += (targetPoint.x - entities[i].pos.x) / distanceToTargetPoint * 0.03f;
			entities[i].pos.y += (targetPoint.y - entities[i].pos.y) / distanceToTargetPoint * 0.03f;
			entities[i].pos.z += (targetPoint.z - entities[i].pos.z) / distanceToTargetPoint * 0.03f;
			if (die) {
				entities.erase(entities.begin() + i);
			}
		}
		if (health <= 0.f) {
			camera.pos.y = 123987.f;
		}
	}
	void spawnEntity() {
		entities.push_back({levels[activeLevel].path[0]});
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
	void buildThem(GameState* game) {
		clearVertices();
		// ground
		for (int x = 0; x < 10; x++) {
			for (int z = 0; z < 10; z++) {
				addPlane((float)x, 1.f, (float)z, 1.f, 1.f, 1.f, 0.f);
			}
		}
		// entities
		for (Entity entity : game->entities) {
			addCube(entity.pos.x - entity.size.x / 2.f, entity.pos.y, entity.pos.z - entity.size.z / 2.f, entity.size.x, entity.size.y, entity.size.z, 0.f, 1.f);
		}
		// point path
		for (Vec3 point : game->levels[game->activeLevel].path) {
			addCube(point.x, point.y, point.z, 0.1f, 0.2f, 0.2f, 0.f, 1.f);
		}
		addText("health: " + to_string((int)game->health), -0.9f, 0.8f, 0.1f, 0.8f);
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
};
GameState game;
GameStateVertexBuilder vBuilder;

static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}
float randFloat() {
	return static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		else if (key == GLFW_KEY_W) keyboard.w = true;
		else if (key == GLFW_KEY_A) keyboard.a = true;
		else if (key == GLFW_KEY_S) keyboard.s = true;
		else if (key == GLFW_KEY_D) keyboard.d = true;
		else if (key == GLFW_KEY_SPACE) keyboard.space = true;
		else if (key == GLFW_KEY_LEFT) keyboard.left = true;
		else if (key == GLFW_KEY_RIGHT) keyboard.right = true;
		else if (key == GLFW_KEY_UP) keyboard.up = true;
		else if (key == GLFW_KEY_DOWN) keyboard.down = true;
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) keyboard.w = false;
		else if (key == GLFW_KEY_A) keyboard.a = false;
		else if (key == GLFW_KEY_S) keyboard.s = false;
		else if (key == GLFW_KEY_D) keyboard.d = false;
		else if (key == GLFW_KEY_SPACE) keyboard.space = false;
		else if (key == GLFW_KEY_LEFT) keyboard.left = false;
		else if (key == GLFW_KEY_RIGHT) keyboard.right = false;
		else if (key == GLFW_KEY_UP) keyboard.up = false;
		else if (key == GLFW_KEY_DOWN) keyboard.down = false;
	};
}

int main(void) {
	//getShaderText();
	GLFWwindow* window;
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
	
	GLint success = GL_FALSE;//error handeling    
	char infoLog[512];

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
		cerr << "Shader compilation error\n" << infoLog << endl;

		glDeleteShader(vertex_shader);
		return -1;
	} else {
		cout << "vertex shader sucucues" << endl;
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);

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
		cout << "successfully loaded texture atlas" << endl;
	} else {
		cout << "failed to load texture atlas. make sure resources/atlas.png exists" << endl;
	};


	stbi_image_free(bytes);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	while (!glfwWindowShouldClose(window)) {
		game.tick();
		vBuilder.buildThem(&game);
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		
		glViewport(0, 0, width, height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
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
