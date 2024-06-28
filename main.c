#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vecmath.h>

typedef struct root
{
	float posX, posY, posZ;
	float rotX, rotY, rotZ;
} root;

typedef struct camera
{
	root transform;
	float fovDeg;
	float nearPlane, farPlane;
} camera;

typedef struct vertex { struct color { float r, g, b; } color; vec3 pos; vec2 texcoord; } vertex;
typedef struct modelInfo { int vertexCount, faceCount; } modelInfo;
typedef struct mesh { root transform; vertex *vertices; vec3I *faces; modelInfo info; unsigned int VBO, VAO, EBO, texture, shader; } mesh;

unsigned int bindTextureFromPNG(const char* filePath, unsigned int shaderProgram)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		char infoLog[255];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		printf("%s", filePath);
		fprintf(stderr, "ERROR::SHADER::PROGRAM::TEXTURE_NOT_FOUND\n");
		exit(3);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);	

	return texture;
}

unsigned int bindTextureFromMTL(const char *mtlPath, unsigned int shaderProgram)
{
	FILE *fpMtl = fopen(mtlPath, "r");
	if(fpMtl == NULL)
	{
		char errorInfo[255] = "Material not found: ";
		strcat(errorInfo, mtlPath); strcat(errorInfo, "\n");
		printf("%s\n", errorInfo);
		exit(1);
	}

	char mtlBuffer[255];
	while(fgets(mtlBuffer, 255, fpMtl) != NULL) { if(strncmp(mtlBuffer, "map_Kd ", 7) == 0) break; }

	char texName[255];
	if(strncmp(mtlBuffer, "map_Kd ", 7) != 0)
	{
		char infoLog[255];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		printf("%s", mtlBuffer);
		fprintf(stderr, "ERROR::SHADER::PROGRAM::TEXTURE_NOT_FOUND\n");
		exit(2);
	}

	sscanf(mtlBuffer, "map_Kd %s", texName);
	//strcat(texPath, texName);
	fclose(fpMtl);

	return bindTextureFromPNG(texName, shaderProgram);
}

unsigned int importShaders(const char *vertexShaderPath, const char *fragmentShaderPath)
{
	FILE *pfVertexShader = fopen(vertexShaderPath, "r");
	if(pfVertexShader == NULL)
	{
		char errorInfo[255] = "Vertex Shader not found: ";
		strcat(errorInfo, vertexShaderPath); strcat(errorInfo, "\n");
		printf("%s\n", errorInfo);
		exit(-1);
	}

	FILE *pfFragmentShader = fopen(fragmentShaderPath, "r");
	if(pfFragmentShader == NULL)
	{
		char errorInfo[255] = "Vertex Shader not found: ";
		strcat(errorInfo, fragmentShaderPath); strcat(errorInfo, "\n");
		printf("%s\n", errorInfo);
		exit(-1);
	}

	fclose(pfVertexShader);
	fclose(pfFragmentShader);
}

void initMesh(mesh *outputMesh)
{
	(*outputMesh).vertices  = NULL;
	(*outputMesh).faces     = NULL;

	(*outputMesh).transform = (root){ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
}

void importObj(mesh *outputMesh, const char *filename, unsigned int shaderProgram)
{
	FILE *pFile;
	char mystring [100];
	char *pch;
	int i=0;
	int j=0;

	initMesh(outputMesh);

	(*outputMesh).shader = shaderProgram;

	pFile = fopen (filename, "r");
	if (pFile == NULL) perror ("Error opening file");

	while(fgets(mystring , 100 , pFile) != NULL)
	{
		if(strncmp(mystring, "mtllib ", 7) == 0)
		{
			char mtlName[255]; char mtlPath[255] = "./assets/models/";
			sscanf(mystring, "mtllib %s", mtlName);
			strcat(mtlPath, mtlName);

			(*outputMesh).texture = bindTextureFromMTL(mtlPath, (*outputMesh).shader);
		}
	}
	rewind(pFile);

	//count the vertex number
	while(fgets (mystring , 100 , pFile) != NULL)
	{
		if(strcmp(strtok (mystring," "),"v")==0) i++;
	}

	//total of vertex
	(*outputMesh).info.vertexCount = i;
	(*outputMesh).vertices = malloc(i * sizeof(vertex));
	rewind(pFile);
	i=0;

	//count the faces number
	while(fgets (mystring , 100 , pFile) != NULL)
	{
		if(strcmp(strtok (mystring," "),"f")==0) i++;
	}

	//total of faces
	(*outputMesh).info.faceCount = i;
	(*outputMesh).faces = malloc(i * sizeof(vertex));

	rewind(pFile);
	i=0;j=0;

	//parsing vertex
	while(fgets (mystring , 100 , pFile) != NULL)
	{
		if(strcmp(strtok (mystring," "),"v")==0)
		{
			pch = strtok (NULL, " ");
			if(pch!=NULL) (*outputMesh).vertices[i].pos.x=atof(pch);

			pch = strtok (NULL, " ");
			if(pch!=NULL) (*outputMesh).vertices[i].pos.y=atof(pch);

			pch = strtok (NULL, " ");
			if(pch!=NULL) (*outputMesh).vertices[i].pos.z=atof(pch);

			(*outputMesh).vertices[i].color.r = 1.0f;
			(*outputMesh).vertices[i].color.g = 1.0f;
			(*outputMesh).vertices[i].color.b = 1.0f;

			(*outputMesh).vertices[i].texcoord.x = 0.0f;
			(*outputMesh).vertices[i].texcoord.y = 0.0f;

			i++;
		}	
	}

	rewind(pFile);
	i=0;

	//parsing faces
	while(fgets (mystring , 100 , pFile) != NULL)
	{
		if(strcmp(strtok (mystring," "),"f")==0){
			pch = strtok(NULL, " ");
			if(pch!=NULL) (*outputMesh).faces[j].x = atoi(pch);

			pch = strtok(NULL, " ");
			if(pch!=NULL) (*outputMesh).faces[j].y = atoi(pch);

			pch = strtok(NULL, " ");
			if(pch!=NULL) (*outputMesh).faces[j].z = atoi(pch);

			i=0;
			j++;
		}	
	}

	fclose (pFile);

	// create buffers/arrays
	glGenVertexArrays(1, &(*outputMesh).VAO);
	glGenBuffers(1, &(*outputMesh).VBO);
	glGenBuffers(1, &(*outputMesh).EBO);

	glBindVertexArray((*outputMesh).VAO);
	glBindBuffer(GL_ARRAY_BUFFER, (*outputMesh).VBO);
	glBufferData(GL_ARRAY_BUFFER, (*outputMesh).info.vertexCount * sizeof(vertex), &(*outputMesh).vertices[0], GL_STATIC_DRAW);  

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*outputMesh).EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (*outputMesh).info.faceCount * sizeof(vec3I), &(*outputMesh).faces[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(mesh, vertices) + offsetof(vertex, pos)));

	// color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(mesh, vertices) + offsetof(vertex, color)));

	// vertex texture coords
	glEnableVertexAttribArray(2);	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(mesh, vertices) + offsetof(vertex, texcoord)));

	glBindVertexArray(0);
}

void drawMesh(mesh *objModel, camera *player)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, objModel->texture);

	float model[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	rotate4x4matrix(model, objModel->transform.rotX, objModel->transform.rotY, objModel->transform.rotZ);
	translate4x4matrix(model, objModel->transform.posX, objModel->transform.posY, objModel->transform.posZ);

	float camera[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f 
	};

	rotate4x4matrix(camera, -player->transform.rotX, -player->transform.rotY, 0.0f);
	translate4x4matrix(camera, -player->transform.posX, -player->transform.posY, -player->transform.posZ);

	float projection[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f 
	};

	project4x4matrix(projection, player->fovDeg, player->nearPlane, player->farPlane);

	// Bind vertex array object
	glBindVertexArray(objModel->VAO);

	// Bind texture
	glBindTexture(GL_TEXTURE_2D, objModel->texture);

	// Draw mesh
	glDrawElements(GL_TRIANGLES, objModel->info.faceCount * 3, GL_UNSIGNED_INT, 0);

	// Unbind vertex array object
	glBindVertexArray(0);
}

void processInput(GLFWwindow *window, camera *player)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	float speed    = 0.1f;
	float rotSpeed = 1.0f;

	float moveVec[4] = { -glfwGetKey(window, GLFW_KEY_W) + glfwGetKey(window, GLFW_KEY_S), 0.0f, -glfwGetKey(window, GLFW_KEY_A) + glfwGetKey(window, GLFW_KEY_D), 1.0f };
	rotateVec4(moveVec, 0.0f, (*player).transform.rotY, 0.0f);

	(*player).transform.posZ += moveVec[0] * speed;
	(*player).transform.posX += moveVec[2] * speed;

	if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) (*player).transform.rotX +=  rotSpeed;
	if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) (*player).transform.rotX += -rotSpeed;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) (*player).transform.rotY += -rotSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) (*player).transform.rotY +=  rotSpeed;
}

int main()
{
	unsigned int SCR_WIDTH = 800;
	unsigned int SCR_HEIGHT = 600;	

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to initialize GLAD");
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);

	const char *vertexShaderSource =
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"layout (location = 2) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"out vec3 ourColor;\n"
		"uniform mat4 model;\n"
		"uniform mat4 camera;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"\tgl_Position = projection * camera * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"\tTexCoord = vec2(aTexCoord.x, 10.f - aTexCoord.y);\n"
		"\tourColor = vec3(aColor.x, aColor.y, aColor.z);\n"
		"}\n\0";
	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D texture1;\n"
		"void main()\n"
		"{\n"
		"\tFragColor = texture(texture1, TexCoord) * vec4(ourColor, 1.0);\n"
		"}\n\0";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
		glfwTerminate();
		return -1;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("%s", infoLog);
		fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
		glfwTerminate();
		return -1;
	}

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	float vertices[] = {
		// positions         // colors           // texture coords
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("./assets/textures/missing texture.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		fprintf(stderr, "ERROR::SHADER::PROGRAM::TEXTURE_NOT_FOUND\n");
		glfwTerminate();
		return -1;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);	

	// uncomment this call to draw in wireframe polygons.
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(shaderProgram);

	float rotDeg = 90.0f;

	mesh monke;
	//initMesh(&monke);
	importObj(&monke, "./assets/models/monke.obj", shaderProgram);

	camera player = {
		.fovDeg = 90.0f,
		.nearPlane = 0.05f,
		.farPlane = 100.0f
	};

	while (!glfwWindowShouldClose(window))
	{
		processInput(window, &player);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawMesh(&monke, &player);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		float model[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		rotDeg += 1.0f;
		//float rotRad = rotDeg*3.141592f/180.0f;
		rotate4x4matrix(model, rotDeg, rotDeg, rotDeg);
		translate4x4matrix(model, 0.0f, 0.0f, -2.0f);

		float camera[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f 
		};

		translate4x4matrix(camera, -player.transform.posX, -player.transform.posY, -player.transform.posZ);
		rotate4x4matrix(camera, -player.transform.rotX, -player.transform.rotY, 0.0f);

		float projection[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f 
		};

		project4x4matrix(projection, player.fovDeg, player.nearPlane, player.farPlane);

		//float vec4rot[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		//multVec4mat(scale, vec4rot, vec4rot);

		//float rotx = vec4rot[0];
		//float roty = vec4rot[1];
		//float rotz = vec4rot[2];
		//printf("x-%lf y-%lf z-%lf\n", rotx, roty, rotz);

		glUseProgram(shaderProgram);
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, model);

		glUseProgram(shaderProgram);
		GLint cameraLoc = glGetUniformLocation(shaderProgram, "camera");
		glUniformMatrix4fv(cameraLoc, 1, GL_TRUE, camera);

		glUseProgram(shaderProgram);
		GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glDeleteVertexArrays(1, &monke.VAO);
	glDeleteBuffers(1, &monke.VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	free(monke.vertices);
	free(monke.faces);

	stbi_image_free(data);

	glfwTerminate();
	return 0;
}
