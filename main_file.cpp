#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream> //dla assimpa
#include <map> //dla map
#include <random> //dla rand
#include <stdlib.h>
#include <stdio.h>
#include "assimp/Importer.hpp" //assimp
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

//0-5 rocks
#define ROCK1 0
#define ROCK2 1
#define ROCK3 2
#define ROCK4 3
#define ROCK5 4
#define ROCK6 5
//6 plant
#define PLANT 6
//7-16 fishes +6
#define FISH1 7
#define FISH2 8
#define FISH3 9
#define FISH4 10
#define FISH5 11
#define FISH6 12
#define FISH7 13
#define FISH8 14
#define FISH9 15
#define FISH10 16

//0-5 rocks
#define TEX_ROCK1 0
#define TEX_ROCK2 1 
#define TEX_ROCK3 2 
#define TEX_ROCK4 3 
#define TEX_ROCK5 4 
#define TEX_ROCK6 5 
//6 plant
#define TEX_PLANT 6
//7-16 fishes +6
#define TEX_FISH1 7
#define TEX_FISH2 8
#define TEX_FISH3 9
#define TEX_FISH4 10
#define TEX_FISH5 11
#define TEX_FISH6 12
#define TEX_FISH7 13
#define TEX_FISH8 14
#define TEX_FISH9 15
#define TEX_FISH10 16
//17 dno
#define TEX_BOTTOM 17

ShaderProgram* waterShader;
ShaderProgram* phongShader;
ShaderProgram* glassShader;

float aspectRatio = 1.0f;

struct Ver {
	std::vector<glm::vec4> Vertices; //wektor przechowujący wierzchołki
	std::vector<glm::vec4> Normals; //wektor przechowujący wektory normalne
	std::vector<glm::vec2> TexCoords; //wektor przechowujący współrzędne tekstur
	std::vector<unsigned int> Indices; //łączenie wierzchołków w trójkąty
};

std::vector<Ver> models; //tablica modeli
std::vector<GLuint> texs; //tablica tekstur

glm::vec4 l1 = glm::vec4(0.0f, 0.0f, 800.0f, 1.0f); //światło 1
glm::vec4 l2 = glm::vec4(0.0f, 0.0f, -800.0f, 1.0f); //światło 2

glm::vec3 camPos = glm::vec3(0.0f, 2.0f, 30.0f); //początkowa pozycja kamery
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, 1.0f); //początkowy kierunek kamery
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f); //oś poruszania kamery

//mapa ryb i tekstur
std::map<int, int> fishTextureMap = {
	{FISH1, TEX_FISH1},
	{FISH2, TEX_FISH2},
	{FISH3, TEX_FISH3},
	{FISH4, TEX_FISH4},
	{FISH5, TEX_FISH5},
	{FISH6, TEX_FISH6},
	{FISH7, TEX_FISH7},
	{FISH8, TEX_FISH8},
	{FISH9, TEX_FISH9},
	{FISH10, TEX_FISH10},
};

//obiekt ryby
struct Fish {
	glm::vec3 position{}; //pozycja
	bool movingRight{}; //kierunek ruchu
	float rotationAngle{}; //kąt obrotu
	float speed{}; //prędkość
	glm::vec3 size{}; //rozmiar
};

const int fishCount{ 10 }; //liczba ryb
Fish fishes[fishCount]; //tablica ryb

//obiekt glonu
struct Algae {
	glm::vec3 position{};
	float size{};
	float angle{};
};

const int algaeCount{ 50 }; //liczba roślin
Algae algaes[algaeCount]; //tablica roślin

//wgrywanie modelu za pomocą biblioteki Assimp z wykładu
void loadModel(std::string filename, int model_i) {
	models.push_back(Ver());
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	std::cout << importer.GetErrorString() << std::endl;

	aiMesh* mesh = scene->mMeshes[0];
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

		aiVector3D vertex = mesh->mVertices[i];
		models[model_i].Vertices.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

		aiVector3D normal = mesh->mNormals[i];
		models[model_i].Normals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

		unsigned int liczba_zest = mesh->GetNumUVChannels();
		unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0];

		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		models[model_i].TexCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			models[model_i].Indices.push_back(face.mIndices[j]);
		}
	}
}

//procedura teksturowania
void readTexture(const char* filename, int tex_i) {
	texs.push_back(0);
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image; //Alokuj wektor do wczytania obrazka
	unsigned width, height; //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);
	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	texs[tex_i] = tex;
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window,int key, int scancode, int action, int mods) {
	const float camSpeed{ 0.5f };

	//Dodawanie/Odejmowanie wektora kierunkowego w zależności od kierunku jazdy
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camPos += camSpeed * camFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camPos -= camSpeed * camFront;

	//Dodawanie/Odejmowanie wektora prostopadłego do kierunku i grawitacji w zależności od kierunku jazdy
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

double yaw = -90.0f;
double pitch = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	const float sensitivity{ 0.1f };
	static double lastX{ 300 };
	static double lastY{ 300 };
	static bool firstMouse{ true };

	//ruch myszką -> zaktualizuj wartości xpos i ypos
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	//obliczanie różnicy pozycji myszy
	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	//kontrola czułości myszy
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	//aktualizacja kątów obrotu kamery
	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	//obliczanie składowych wektora kierunku patrzenia kamery
	glm::vec3 front(
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	);
	camFront = glm::normalize(front);
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);

	//renderowanie obiektów przezroczystych
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouseCallback);

	readTexture("./img/rock0.png", TEX_ROCK1);
	readTexture("./img/rock1.png", TEX_ROCK2);
	readTexture("./img/rock2.png", TEX_ROCK3);
	readTexture("./img/rock3.png", TEX_ROCK4);
	readTexture("./img/rock4.png", TEX_ROCK5);
	readTexture("./img/rock5.png", TEX_ROCK6);
	readTexture("./img/lisc.png", TEX_PLANT);
	readTexture("./models/TropicalFish01.png", TEX_FISH1);
	readTexture("./models/TropicalFish02.png", TEX_FISH2);
	readTexture("./models/TropicalFish03.png", TEX_FISH3);
	readTexture("./models/TropicalFish04.png", TEX_FISH4);
	readTexture("./models/TropicalFish05.png", TEX_FISH5);
	readTexture("./models/TropicalFish06.png", TEX_FISH6);
	readTexture("./models/TropicalFish13.png", TEX_FISH7);
	readTexture("./models/TropicalFish11.png", TEX_FISH8);
	readTexture("./models/TropicalFish09.png", TEX_FISH9);
	readTexture("./models/TropicalFish14.png", TEX_FISH10);
	readTexture("./models/sand1.png", TEX_BOTTOM);

	loadModel(std::string("models/Rock0.obj"), ROCK1);
	loadModel(std::string("models/Rock1.fbx"), ROCK2);
	loadModel(std::string("models/Rock2.fbx"), ROCK3);
	loadModel(std::string("models/Rock3.fbx"), ROCK4);
	loadModel(std::string("models/Rock4.fbx"), ROCK5);
	loadModel(std::string("models/Rock5.fbx"), ROCK6);
	loadModel(std::string("models/plants.obj"), PLANT);
	loadModel(std::string("models/plants1.obj"), PLANT);
	loadModel(std::string("models/TropicalFish01.obj"), FISH1);
	loadModel(std::string("models/TropicalFish02.obj"), FISH2);
	loadModel(std::string("models/TropicalFish03.obj"), FISH3);
	loadModel(std::string("models/TropicalFish04.obj"), FISH4);
	loadModel(std::string("models/TropicalFish05.obj"), FISH5);
	loadModel(std::string("models/TropicalFish06.obj"), FISH6);
	loadModel(std::string("models/TropicalFish13.obj"), FISH7);
	loadModel(std::string("models/TropicalFish11.obj"), FISH8);
	loadModel(std::string("models/TropicalFish09.obj"), FISH9);
	loadModel(std::string("models/TropicalFish14.obj"), FISH10);

	waterShader = new ShaderProgram("v_water.glsl", NULL, "f_water.glsl");
	phongShader = new ShaderProgram("v_phong.glsl", NULL, "f_phong.glsl");
	glassShader = new ShaderProgram("v_glass.glsl", NULL, "f_glass.glsl");
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
}

void drawGlass(glm::mat4& P, glm::mat4& V, glm::mat4& M) {
	glassShader->use(); //Aktywacja programu cieniującego

	//Przeslij parametry programu cieniującego do karty graficznej
	glUniformMatrix4fv(glassShader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(glassShader->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(glassShader->u("M"), 1, false, glm::value_ptr(M));

	//światło
	glUniform4fv(glassShader->u("li1"), 1, glm::value_ptr(l1));
	glUniform4fv(glassShader->u("li2"), 1, glm::value_ptr(l2));

	glEnableVertexAttribArray(glassShader->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(glassShader->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(glassShader->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(glassShader->a("normal"), 4, GL_FLOAT, false, 0, myCubeVertexNormals); //Wskaż tablicę z danymi dla atrybutu normal

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount); //Narysuj obiekt

	glDisableVertexAttribArray(glassShader->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(glassShader->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal
}

void drawBottom(glm::mat4& P, glm::mat4& V, glm::mat4& M) {
	float bottomTexCoords[] = {
		4.0f, 4.0f,	  0.0f, 0.0f,    0.0f, 4.0f,
		4.0f, 4.0f,   4.0f, 0.0f,    0.0f, 0.0f,
	};

	float bottomVertices[] = {
		8.0f,-14.0f, 6.0f,1.0f,
		8.0f, 14.0f,-6.0f,1.0f,
		8.0f,-14.0f,-6.0f,1.0f,

		8.0f,-14.0f, 6.0f,1.0f,
		8.0f, 14.0f, 6.0f,1.0f,
		8.0f, 14.0f,-6.0f,1.0f,
	};

	float bottomVertexNormals[] = {
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, -1.0f, 0.0f,

		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f,
	};
	int bottomVertexCount = 6;

	waterShader->use();//Aktywacja programu cieniującego

	//Przeslij parametry programu cieniującego do karty graficznej
	glUniformMatrix4fv(waterShader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(waterShader->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(waterShader->u("M"), 1, false, glm::value_ptr(M));

	//światło
	glUniform4fv(waterShader->u("li1"), 1, glm::value_ptr(l1));
	glUniform4fv(waterShader->u("li2"), 1, glm::value_ptr(l2));

	glEnableVertexAttribArray(waterShader->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(waterShader->a("vertex"), 4, GL_FLOAT, false, 0, bottomVertices); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(waterShader->a("texCoord"));
	glVertexAttribPointer(waterShader->a("texCoord"), 2, GL_FLOAT, false, 0, bottomTexCoords);

	glEnableVertexAttribArray(waterShader->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(waterShader->a("normal"), 4, GL_FLOAT, false, 0, bottomVertexNormals); //Wskaż tablicę z danymi dla atrybutu normal

	//powielanie tektury przez repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texs[TEX_BOTTOM]);
	glUniform1i(waterShader->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, bottomVertexCount); //Narysuj obiekt

	glDisableVertexAttribArray(waterShader->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(waterShader->a("texCoord"));
	glDisableVertexAttribArray(waterShader->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal
}

//obiekty
void drawModel(glm::mat4& P, glm::mat4& V, glm::mat4& M, int model_i, int txt) {
	phongShader->use();//Aktywacja programu cieniującego

	//Przeslij parametry programu cieniującego do karty graficznej
	glUniformMatrix4fv(phongShader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(phongShader->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(phongShader->u("M"), 1, false, glm::value_ptr(M));

	//światło
	glUniform4fv(phongShader->u("li1"), 1, glm::value_ptr(l1));
	glUniform4fv(phongShader->u("li2"), 1, glm::value_ptr(l2));

	glEnableVertexAttribArray(phongShader->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(phongShader->a("vertex"), 4, GL_FLOAT, false, 0, models[model_i].Vertices.data()); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(phongShader->a("texCoord"));
	glVertexAttribPointer(phongShader->a("texCoord"), 2, GL_FLOAT, false, 0, models[model_i].TexCoords.data());

	glEnableVertexAttribArray(phongShader->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(phongShader->a("normal"), 4, GL_FLOAT, false, 0, models[model_i].Normals.data()); //Wskaż tablicę z danymi dla atrybutu normal

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texs[txt]);
	glUniform1i(phongShader->u("tex"), 0);

	glDrawElements(GL_TRIANGLES, models[model_i].Indices.size(), GL_UNSIGNED_INT, models[model_i].Indices.data()); //Narysuj obiekt

	glDisableVertexAttribArray(phongShader->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(phongShader->a("texCoord"));
	glDisableVertexAttribArray(phongShader->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal
}

//rysowanie kamieni
void drawRocks(glm::mat4& P, glm::mat4& V) {
	glm::mat4 rocks = glm::mat4(1.0f);

	rocks = glm::translate(rocks, glm::vec3(0.0f, -7.85f, 0.0f));
	rocks = glm::scale(rocks, glm::vec3(0.125f, 0.125f, 0.125f));

	glm::mat4 rock1 = glm::translate(rocks, glm::vec3(0.0f, -2.0f, 0.0f));
	drawModel(P, V, rock1, ROCK1, TEX_ROCK1);

	glm::mat4 rock2 = glm::translate(rocks, glm::vec3(70.0f, -2.0f, -3.0f));
	rock2 = glm::scale(rock2, glm::vec3(0.04f, 0.04f, 0.04f));
	drawModel(P, V, rock2, ROCK2, TEX_ROCK2);

	glm::mat4 rock2_1 = glm::translate(rocks, glm::vec3(0.0f, -2.0f, 28.0f));
	rock2_1 = glm::scale(rock2_1, glm::vec3(0.01f, 0.01f, 0.01f));
	drawModel(P, V, rock2_1, ROCK2, TEX_ROCK2);

	glm::mat4 rock3 = glm::translate(rocks, glm::vec3(-60.0f, -2.0f, 2.0f));
	rock3 = glm::scale(rock3, glm::vec3(0.04f, 0.04f, 0.04f));
	drawModel(P, V, rock3, ROCK3, TEX_ROCK3);

	glm::mat4 rock3_1 = glm::translate(rocks, glm::vec3(50.0f, -2.0f, 28.0f));
	rock3_1 = glm::scale(rock3_1, glm::vec3(0.005f, 0.005f, 0.005f));
	drawModel(P, V, rock3_1, ROCK3, TEX_ROCK3);

	glm::mat4 rock4 = glm::translate(rocks, glm::vec3(-40.0f, 0.0f, -10.0f));
	rock4 = glm::scale(rock4, glm::vec3(0.02f, 0.02f, 0.02f));
	drawModel(P, V, rock4, ROCK4, TEX_ROCK4);

	glm::mat4 rock5 = glm::translate(rocks, glm::vec3(40.0f, 0.0f, 10.0f));
	rock5 = glm::scale(rock5, glm::vec3(0.05f, 0.05f, 0.05f));
	drawModel(P, V, rock5, ROCK5, TEX_ROCK5);

	glm::mat4 rock6 = glm::translate(rocks, glm::vec3(-80.0f, 0.0f, 20.0f));
	rock6 = glm::scale(rock6, glm::vec3(0.15f, 0.15f, 0.15f));
	drawModel(P, V, rock6, ROCK6, TEX_ROCK6);
}

void initAlgaes() {
	std::srand(static_cast<unsigned int>(std::time(0)));
	float posY{ -8.10f };
	float size{ 0.1f };
	for (int i{ 0 }; i < algaeCount; ++i) {
		float randPosX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 22.0f - 11.0f; //losowanie pozycji początkowej od -10 do 10
		float randPosZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 5.0f - 2.5f; //losowanie pozycji początkowej od -2.5 do 2.5
		float randAng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f; //losowanie pozycji początkowej od -5 do 5
		if (i % 5) {
			posY += 0.02f;
			size += 0.02f;
		}
		algaes[i].position = glm::vec3(randPosX, posY, randPosZ);
		algaes[i].size = size;
		algaes[i].angle = glm::radians(randAng);
	}
}

void drawAlgaes(glm::mat4& P, glm::mat4& V) {
	for (int i{ 0 }; i < algaeCount; ++i) {
		float timeRand = glfwGetTime();
		glm::mat4 algaeM = glm::mat4(1.0f);
		if (i % 2)
			algaes[i].angle += sin(timeRand * 0.8f) * 0.4f; //od -0.4 do 0.4
		else
			algaes[i].angle -= sin(timeRand * 0.8f) * 0.4f;
		algaeM = glm::translate(algaeM, algaes[i].position);
		algaeM = glm::rotate(algaeM, algaes[i].angle, glm::vec3(0.0f, 1.0f, 0.0f));
		algaeM = glm::scale(algaeM, glm::vec3(algaes[i].size, algaes[i].size, algaes[i].size));

		drawModel(P, V, algaeM, PLANT, TEX_PLANT);
	}
}

void initFishes() {
	std::srand(static_cast<unsigned int>(std::time(0)));
	for (int i{ 0 }; i < fishCount; ++i) {
		float randPosX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 20.0f - 10.0f; //losowanie pozycji początkowej od -10 do 10
		float randPosY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f - 5.0f; //losowanie pozycji początkowej od -5 do 5
		float randPosZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f - 5.0f; //losowanie pozycji początkowej od -5 do 5
		float randSize = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.009f + 0.001f; //losowanie pozycji początkowej od 0.001 do 0.01

		fishes[i].position = glm::vec3(randPosX, randPosY, randPosZ);
		fishes[i].movingRight = true;
		fishes[i].rotationAngle = 0.0f;
		fishes[i].speed = 0.1f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.05f)); //losowanie prędkości od 0.1 do 15
		fishes[i].size = glm::vec3(randSize, randSize, randSize);
	}
}

void updateFishState(Fish& fish) {
	if (fish.movingRight) {
		fish.position.x += fish.speed;
		if (fish.position.x >= 10.0f) {
			fish.movingRight = false;
			fish.rotationAngle = 3.14f;
		}
	}
	else {
		fish.position.x -= fish.speed;
		if (fish.position.x <= -10.0f) {
			fish.movingRight = true;
			fish.rotationAngle = 0.0f;
		}
	}
}

//rysowanie rybek
void drawFishes(glm::mat4& P, glm::mat4& V) {
	for (int i{ 0 }; i < fishCount; ++i) {
		glm::mat4 fishM = glm::mat4(1.0f);

		fishM = glm::translate(glm::mat4(1.0f), fishes[i].position);
		fishM = glm::rotate(fishM, 1.57f, glm::vec3(0.0f, 1.0f, 0.0f));
		fishM = glm::rotate(fishM, fishes[i].rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		fishM = glm::scale(fishM, fishes[i].size);

		drawModel(P, V, fishM, FISH1 + i, TEX_FISH1 + i);
	}
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 M = glm::mat4(1.0f);
	glm::mat4 V=glm::lookAt(
         camPos,
         camPos + camFront,
         camUp); //Wylicz macierz widoku
    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 1.0f, 50.0f); //Wylicz macierz rzutowania
	glm::mat4 M_rotate = glm::rotate(M, -PI / 2, glm::vec3(0.0f, 0.0f, 1.0f)); //Wylicz macierz modelu
	glm::mat4 M_scale = glm::scale(M_rotate, glm::vec3(8.0f, 14.0f, 6.0f)); //Wylicz macierz modelu

	//rysowanie poszczeglnych elementów
	drawBottom(P, V, M_rotate);
	for (int i{ 0 }; i < fishCount; ++i)
		updateFishState(fishes[i]);
	drawFishes(P, V);
	drawRocks(P, V);
	drawAlgaes(P, V);
	drawGlass(P, V, M_scale);

    glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni
}


int main(void)
{
	initFishes(); //zainicjalizuj obiekty ryb
	initAlgaes(); //zainicjalizuj obiekty alg
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno
	srand(time(0));

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 1000, "OpenGL", NULL, NULL);  //Utwórz okno 1000x1000 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
        glfwSetTime(0); //Zeruj timer
		drawScene(window); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
