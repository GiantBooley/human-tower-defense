#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <linmath.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;
struct Vertex {
	float x, y, z;
	float u, v;
};
vector<Vertex> vertices = {
	{-0.5f, -0.5f,  0.5f, 0.f, 0.f},
	{ 0.5f, -0.5f,  0.5f, 1.f, 0.f},
	{-0.5f,  0.5f,  0.5f, 0.f, 1.f},
	{ 0.5f, 0.5f,  0.5f, 1.f, 1.f},
	{-0.5f, -0.5f, -0.5f, 0.f, 1.f},
	{ 0.5f, -0.5f, -0.5f, 1.f, 1.f},
	{-0.5f,  0.5f, -0.5f, 0.f, 0.f},
	{ 0.5f,  0.5f, -0.5f, 1.f, 0.f}
};
vector<unsigned int> indices = {
		2, 6, 7,
		2, 3, 7,

		0, 4, 5,
		0, 1, 5,

		0, 2, 6,
		0, 4, 6,

		1, 3, 7,
		1, 5, 7,

		0, 2, 3,
		0, 1, 3,

		4, 6, 7,
		4, 5, 7
};
static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec2 vTexCoord;\n"
"attribute vec3 vPos;\n"
"varying vec2 texCoord;\n"
"varying vec3 pos;\n"
"void main()\n"
"{\n"
"	vec4 position = MVP * vec4(vPos, 1.0);\n"
"	gl_Position = position;\n"
"	texCoord = vTexCoord / 32.0;\n"
"	pos = vPos;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec2 texCoord;\n"
"varying vec3 pos;\n"
"uniform sampler2D texture1;"
"uniform sampler2D texture2;"
"void main()\n"
"{\n"
"   vec4 color = texture2D(texture1, texCoord);"
"	gl_FragColor = color;\n"
"}\n";
class Keyboard {
	public:
	   bool w = false;
	   bool a = false;
	   bool s = false;
	   bool d = false;
};
class Entity {
public:
	float x = 5.f;
	float y = 1.f;
	float z = 5.f;
	float width = 1.f;
	float height = 1.f;
	int targetPoint = 0;
	float depth = 1.f;
	float health = 1.f;
};
class Camera {
public:
	float x = 0.f;
	float y = 0.f;
	float z = 5.f;
};
class Point {
public:
	float x;
	float y;
	Point(float asdx, float asdy) {
		x = asdx;
		y = asdy;
	};
};
float distance2D(Point p1, Point p2) {
	return sqrt(pow(p2.x - p1.x, 2.f) + pow(p2.y - p1.y, 2.f));
}
class Level {
public:
	Point path[2] = {{0.f, 0.f}, {1.f, 5.f}};
};
Keyboard keyboard;
class GameState {
public:
	vector<Entity> entities = {{}};
	Camera camera;
	Level levels[1] = {{}};
	int activeLevel = 0;
	float health = 100.f;
	
	void tick() {
		camera.x = 5.f;
		camera.y = 6.f;
		camera.z = 8.f;
		if (keyboard.w) {
			entities.push_back({});
		}
		for (int i = entities.size() - 1; i >= 0; i--) {
			bool die = false;
			Point targetPoint = levels[activeLevel].path[entities[i].targetPoint];
			float distanceToTargetPoint = distance2D((Point){entities[i].z, entities[i].x}, targetPoint);
			entities[i].x += (targetPoint.y - entities[i].x) / distanceToTargetPoint * 0.1f;
			entities[i].z += (targetPoint.x - entities[i].z) / distanceToTargetPoint * 0.1f;
			if (distanceToTargetPoint < 0.1f) {
				entities[i].targetPoint++;
				if (entities[i].targetPoint >= sizeof(levels[activeLevel].path)/sizeof(levels[activeLevel].path[0])) {
					health -= 1.f;
					die = true;
				}
			}
			if (die) {
				entities.erase(entities.begin() + i);
			}
		}
	}
};
class GameStateVertexBuilder {
public:
	void buildThem(GameState* game) {
		clearVertices();
		// ground
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 1; y++) {
				for (int z = 0; z < 10; z++) {
					addCube((float)x, (float)y, (float)z, 1.f, 1.f, 1.f, 1.f, 0.f);
				}
			}
		}
		// entities
		for (Entity entity : game->entities) {
			addCube(entity.x, entity.y, entity.z, entity.width, entity.height, entity.depth, 0.f, 1.f);
		}
		// point path
		for (Point point : game->levels[game->activeLevel].path) {
			addCube(point.y, 1.1f, point.x, 0.1f, 0.1f, 0.1f, 0.f, 0.f);
		}
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
			{x  , y  , z+d, u    , v    },
			{x+w, y  , z+d, u+1.f, v    },
			{x  , y+h, z+d, u    , v+1.f},
			{x+w, y+h, z+d, u+1.f, v+1.f},
			{x  , y  , z  , u    , v+1.f},
			{x+w, y  , z  , u+1.f, v+1.f},
			{x  , y+h, z  , u    , v    },
			{x+w, y+h, z  , u+1.f, v    }
		});
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
float roundToPlace(float x, float place) {
	return round(x / place) * place;
}
string toFixed(float x, unsigned int places) {
	return to_string(roundToPlace(x, 1.f / pow((float)places, 10.f))).substr(0, places - 1U);
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
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) keyboard.w = false;
		else if (key == GLFW_KEY_A) keyboard.a = false;
		else if (key == GLFW_KEY_S) keyboard.s = false;
		else if (key == GLFW_KEY_D) keyboard.d = false;
	};
}

int main(void) {
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vtexcoord_location, texture1_location, texture2_location;

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

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	mvp_location = glGetUniformLocation(program, "MVP");
	texture1_location = glGetUniformLocation(program, "texture1");
	texture2_location = glGetUniformLocation(program, "texture2");
	vpos_location = glGetAttribLocation(program, "vPos");
	vtexcoord_location = glGetAttribLocation(program, "vTexCoord");

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)0);
	glEnableVertexAttribArray(vtexcoord_location);
	glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)(sizeof(float) * 3));

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



	while (!glfwWindowShouldClose(window)) {
		game.tick();
		vBuilder.buildThem(&game);
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		int verticesLength = vertices.size();
		Vertex newVertices[verticesLength];
		for (int i = 0; i < verticesLength; i++) {
			newVertices[i] = vertices[i];
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(newVertices), newVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		
		glViewport(0, 0, width, height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, texture);

		mat4x4_identity(m);
		mat4x4_translate(m, -game.camera.x, -game.camera.y, -game.camera.z);
		mat4x4_rotate_X(m, m, 1.3f);
		mat4x4_rotate_Y(m, m, 1.57f);
		//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_perspective(p, 1.57f, ratio, 0.1f, 200.f);
		mat4x4_mul(mvp, p, m);

		glUniform1i(texture1_location, 0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(texture2_location, 1);
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawElements(
			GL_TRIANGLES,	 // mode
			indices.size(), // count
			GL_UNSIGNED_INT,   // type
			(void*)0		   // element array buffer offset
		);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteTextures(1, &texture);
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
