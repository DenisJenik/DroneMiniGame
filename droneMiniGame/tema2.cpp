#include "lab_m1/tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace m1;

Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
	srand(static_cast<unsigned>(time(nullptr)));

    camera = new implemented::Camera();
    camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
	camera->SetProjectionMatrix(glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f));

	minimapCamera = new implemented::Camera();
	minimapCamera->Set(glm::vec3(0, 10, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
	minimapCamera->SetProjectionMatrix(glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 200.0f));

    // Create a shader program for drawing face polygon with the color of the normal
    {
        Shader *shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    
    //culori folosite pentru cuburi
    glm::vec3 gray = glm::vec3(128.0f / 255.0f, 128.0f / 255.0f, 128 / 255.0f);
    glm::vec3 lightbrown = glm::vec3(200.f / 255.f, 177.f / 255.f, 152.f / 255.f);
    glm::vec3 housegray = glm::vec3(166.f / 255.f, 162.f / 255.f, 162.f / 255.f);
    glm::vec3 black = glm::vec3(0, 0, 0);
	glm::vec3 red = glm::vec3(1, 0, 0);
	glm::vec3 green = glm::vec3(0, 1, 0);
  
	meshes["terrain"] = CreateTerrainMesh(terrainSize, terrainSize, terrainCellSize); //teren
    
    meshes["cube"] = CreateBox("cube", gray); //simple cube gray
    meshes["cube2"] = CreateBox("cube2", black); //simple cube black
	meshes["cube3"] = CreateBox("cube3", housegray); //simple cube housegray
    meshes["package"] = CreateBox("package", lightbrown); //simple cube lightbrown

	meshes["cylinderu"] = CreateCylinder("cylinderu", 1.0f, 2.0f, 32); //trunchi copac
	meshes["cone"] = CreateCone("cone", 2.0f, 3.0f, 32); //coroana copac

    meshes["circle_red"] = CreateCircle("circle_red", 1.0f, 32, red); //cerc rosu
    meshes["circle_green"] = CreateCircle("circle_green", 1.0f, 32, green); //cerc verde

	meshes["arrowShape"] = CreateArrowShape("arrowShape", green); //sageata
   
    GenerateTrees(minTrees, maxTrees, 100.0f, 100.0f, 5.0f);
    GenerateCubes(minCubes, maxCubes, 100.0f, 100.0f, 10.0f);

    carryingPackage = false;
	packageActive = false;
    deliveryComplete = false;
    deliveryCircleActive = false;
	inDelivery = false;
	olddestination = -1;
	counter = 0;
}

Mesh* Tema2::CreateMesh(const char *name, const std::vector<VertexFormat> &vertices, const std::vector<unsigned int> &indices)
{
    unsigned int VAO = 0;
    // Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create the VBO and bind it
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // Create the IBO and bind it
    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // ========================================================================
    // This section demonstrates how the GPU vertex shader program
    // receives data.

    // If you look closely in the `Init()` and `Update()`
    // functions, you will see that we have three objects which we load
    // and use in three different ways:
    // - LoadMesh   + LabShader (this lab's shader)
    // - CreateMesh + VertexNormal (this shader is already implemented)
    // - CreateMesh + LabShader (this lab's shader)
    // To get an idea about how they're different from one another, do the
    // following experiments. What happens if you switch the color pipe and
    // normal pipe in this function (but not in the shader)? Now, what happens
    // if you do the same thing in the shader (but not in this function)?
    // Finally, what happens if you do the same thing in both places? Why?

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));
    // ========================================================================

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    CheckOpenGLError();

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    meshes[name]->vertices = vertices;
    meshes[name]->indices = indices;
    return meshes[name];
}

Mesh* Tema2::CreateTerrainMesh(int m, int n, float cellSize) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    for (int z = 0; z <= n; ++z) {
        for (int x = 0; x <= m; ++x) {
            float posX = x * cellSize;
            float posZ = z * cellSize;
            vertices.emplace_back(glm::vec3(posX, 0, posZ), glm::vec3(0, 1, 0), glm::vec3(0.5f, 0.8f, 0.2f));
        }
    }

    for (int z = 0; z < n; ++z) {
        for (int x = 0; x < m; ++x) {
            int topLeft = z * (m + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (m + 1) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return CreateMesh("terrain", vertices, indices);
}

Mesh* Tema2::CreateBox(const std::string& name, const glm::vec3& color) {
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-1, -1,  1), glm::vec3(0, 1, 1), color),
        VertexFormat(glm::vec3(1, -1,  1), glm::vec3(1, 0, 1), color),
        VertexFormat(glm::vec3(-1,  1,  1), glm::vec3(1, 0, 0), color),
        VertexFormat(glm::vec3(1,  1,  1), glm::vec3(0, 1, 0), color),
        VertexFormat(glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1), color),
        VertexFormat(glm::vec3(1, -1, -1), glm::vec3(0, 1, 1), color),
        VertexFormat(glm::vec3(-1,  1, -1), glm::vec3(1, 1, 0), color),
        VertexFormat(glm::vec3(1,  1, -1), glm::vec3(0, 0, 1), color),
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,        1, 3, 2,
        2, 3, 7,        2, 7, 6,
        1, 7, 3,        1, 5, 7,
        6, 7, 4,        7, 5, 4,
        0, 4, 1,        1, 4, 5,
        2, 6, 4,        0, 2, 4,
    };

    return CreateMesh(name.c_str(), vertices, indices);
}

Mesh* Tema2::CreateCylinder(const std::string& name, float radius, float height, int segments) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 brownColor = glm::vec3(67.f/255.f, 40.f/255.f, 24.f/255.f);

    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), brownColor);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, 0, z), glm::vec3(0, -1, 0), brownColor);
    }

    for (int i = 1; i < segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    indices.push_back(0);
    indices.push_back(segments);
    indices.push_back(1);

    int offset = segments + 1;
    vertices.emplace_back(glm::vec3(0, height, 0), glm::vec3(0, 1, 0), brownColor);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, height, z), glm::vec3(0, 1, 0), brownColor);
    }

    for (int i = 1; i < segments; ++i) {
        indices.push_back(offset);
        indices.push_back(offset + i);
        indices.push_back(offset + i + 1);
    }
    indices.push_back(offset);
    indices.push_back(offset + segments);
    indices.push_back(offset + 1);

    for (int i = 1; i <= segments; ++i) {
        int next = (i % segments) + 1;
        indices.push_back(i);
        indices.push_back(next);
        indices.push_back(offset + i);

        indices.push_back(next);
        indices.push_back(offset + next);
        indices.push_back(offset + i);
    }

    return CreateMesh(name.c_str(), vertices, indices);
}

Mesh* Tema2::CreateCone(const std::string& name, float radius, float height, int segments) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 GreenColor = glm::vec3(56.f/255.f, 176.f/255.f, 0.0f);

    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), GreenColor);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, 0, z), glm::vec3(0, -1, 0), GreenColor);
    }

    for (int i = 1; i < segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    indices.push_back(0);
    indices.push_back(segments);
    indices.push_back(1);

    vertices.emplace_back(glm::vec3(0, height, 0), glm::vec3(0, 1, 0), GreenColor);

    int topIndex = segments + 1;
    for (int i = 1; i <= segments; ++i) {
        int next = (i % segments) + 1;
        indices.push_back(topIndex);
        indices.push_back(i);
        indices.push_back(next);
    }

    return CreateMesh(name.c_str(), vertices, indices);
}

Mesh* Tema2::CreateCircle(const std::string& name, float radius, int resolution, const glm::vec3& color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), color);

    for (int i = 0; i < resolution; i++) {
        float angle = 2.0f * glm::pi<float>() * i / resolution;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, 0, z), glm::vec3(0, 1, 0), color);

        if (i > 0) {
            indices.push_back(0);       
            indices.push_back(i);    
            indices.push_back(i - 1);
        }
    }

    indices.push_back(0);         
    indices.push_back(1);           
    indices.push_back(resolution - 1);

    return CreateMesh("name", vertices, indices);
}

Mesh* Tema2::CreateArrowShape(const std::string& name, const glm::vec3& color) {

    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(-0.1f, 0, 0), glm::vec3(0, 1, 0), color),
        VertexFormat(glm::vec3(0.1f, 0, 0), glm::vec3(0, 1, 0), color), 
        VertexFormat(glm::vec3(0.1f, 0, -1.0f), glm::vec3(0, 1, 0), color),
        VertexFormat(glm::vec3(-0.1f, 0, -1.0f), glm::vec3(0, 1, 0), color),

        VertexFormat(glm::vec3(-0.2f, 0, -1.0f), glm::vec3(0, 1, 0), color),
        VertexFormat(glm::vec3(0.2f, 0, -1.0f), glm::vec3(0, 1, 0), color), 
        VertexFormat(glm::vec3(0.0f, 0, -1.5f), glm::vec3(0, 1, 0), color)  
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0, 
        4, 5, 6   
    };

    return CreateMesh("name", vertices, indices);
}


void Tema2::GenerateTrees(int minTrees, int maxTrees, float terrainWidth, float terrainHeight, float minDistance) {
    srand(static_cast<unsigned>(time(nullptr)));

	int numberOfTrees = rand() % (maxTrees - minTrees + 1) + minTrees; 

    for (int i = 0; i < numberOfTrees; ++i) {
        bool validPosition = false;
        glm::vec3 position;
        float trunkHeight, trunkRadius, foliageHeight, foliageRadius;

        while (!validPosition) {
            // pozitii aleatorii pe teren
            float posX = static_cast<float>(rand()) / RAND_MAX * terrainWidth - terrainWidth / 2.0f;
            float posZ = static_cast<float>(rand()) / RAND_MAX * terrainHeight - terrainHeight / 2.0f;
            position = glm::vec3(posX, 0, posZ);

            // dimensiuni aleatorii pentru copaci
            trunkHeight = 1.5f + static_cast<float>(rand()) / RAND_MAX * 2.0f; // intre 1.5 și 3.5
            trunkRadius = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.3f; //  0.2 și 0.5
            foliageHeight = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f; //  2.0 și 4.0
            foliageRadius = 1.0f + static_cast<float>(rand()) / RAND_MAX * 1.5f; //  1.0 și 2.5

            if (IsPositionValid(position, minDistance)) {
                validPosition = true;
            }
        }

        trees.push_back({ position, trunkHeight, trunkRadius, foliageHeight, foliageRadius });
    }
}

void Tema2::GenerateCubes(int minCubes, int maxCubes, float terrainWidth, float terrainHeight, float minDistance) {
    srand(static_cast<unsigned>(time(nullptr)));

    int numberOfCubes = rand() % (maxCubes - minCubes + 1) + minCubes;

    for (int i = 0; i < numberOfCubes; ++i) {
        bool validPosition = false;
        glm::vec3 position;
        glm::vec3 scale;

        while (!validPosition) {
            // pozitie aleatoare pe teren
            float posX = static_cast<float>(rand()) / RAND_MAX * terrainWidth - terrainWidth / 2.0f;
            float posZ = static_cast<float>(rand()) / RAND_MAX * terrainHeight - terrainHeight / 2.0f;
            position = glm::vec3(posX, 1.0f, posZ);

            // dimensiune aleatoare pentru cub
            float scaleX = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;  // intre 2.0 si 5.0
            float scaleY = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
            float scaleZ = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
            scale = glm::vec3(scaleX, scaleY, scaleZ);

            if (IsPositionValid(position, minDistance)) {
                validPosition = true;
            }
        }

        cubes.push_back({ position, scale });
    }
}

bool Tema2::IsPositionValid(const glm::vec3& position, float minDistance) {
 
    for (const auto& tree : trees) {
        if (glm::distance(position, tree.position) < minDistance) {
            return false;
        }
    }

    for (const auto& cube : cubes) {
        if (glm::distance(position, cube.position) < minDistance) {
            return false;
        }
    }

    return true;
}

void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0.54f, 0.81f, 0.94f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}   

void Tema2::Update(float deltaTimeSeconds) {

    HandleCollisions(deltaTimeSeconds);
	HandleDelivery();

    //terrain
    glm::mat4 terrainModel = glm::mat4(1);
    terrainModel = glm::translate(terrainModel, glm::vec3(-(terrainSize/ 2.f), 0, -(terrainSize / 2.f))); //centrare
    RenderSimpleMesh(meshes["terrain"], shaders["LabShader"], terrainModel);

	//arrow
    glm::mat4 arrowModel = glm::mat4(1);
    if(dronePosition.y - glm::vec3(0, 1, 0).y < 1){
        arrowModel = glm::translate(arrowModel, glm::vec3(dronePosition.x, 1, dronePosition.z));
    }
    else {
        arrowModel = glm::translate(arrowModel, dronePosition - glm::vec3(0, 1, 0));
    }
    glm::vec3 targetPosition = carryingPackage ? destinationPosition : packagePosition; 
    glm::vec3 direction = glm::normalize(dronePosition - targetPosition);
    float angle = atan2(direction.x, direction.z);
    arrowModel = glm::rotate(arrowModel, angle, glm::vec3(0, 1, 0));
    RenderSimpleMesh(meshes["arrowShape"], shaders["VertexNormal"], arrowModel);

    //buildings
    for (const auto& cube : cubes) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, cube.position);
        modelMatrix = glm::scale(modelMatrix, cube.scale);  // Dimensiunea cubului
        RenderSimpleMesh(meshes["cube3"], shaders["VertexNormal"], modelMatrix);
    }
    
    //trees
    for (const auto& tree : trees) {
    
        glm::mat4 trunkModel = glm::mat4(1);
        trunkModel = glm::translate(trunkModel, tree.position);
        trunkModel = glm::scale(trunkModel, glm::vec3(tree.trunkRadius, tree.trunkHeight, tree.trunkRadius));
        RenderSimpleMesh(meshes["cylinderu"], shaders["VertexNormal"], trunkModel);

        glm::mat4 foliageModel1 = glm::mat4(1);
        foliageModel1 = glm::translate(glm::mat4(1), tree.position + glm::vec3(0, tree.trunkHeight, 0));
        foliageModel1 = glm::scale(foliageModel1, glm::vec3(tree.foliageRadius, tree.foliageHeight, tree.foliageRadius));
        RenderSimpleMesh(meshes["cone"], shaders["VertexNormal"], foliageModel1);

        glm::mat4 foliageModel2 = glm::mat4(1);
        foliageModel2 = glm::translate(glm::mat4(1), tree.position + glm::vec3(0, tree.trunkHeight + tree.foliageHeight, 0));
        foliageModel2 = glm::scale(foliageModel2, glm::vec3(tree.foliageRadius * 0.8f, tree.foliageHeight * 0.8f, tree.foliageRadius * 0.8f));
        RenderSimpleMesh(meshes["cone"], shaders["VertexNormal"], foliageModel2);
    }
	    

    //drone
    glm::mat4 droneBase = glm::mat4(1);
    droneBase = glm::translate(droneBase, dronePosition);
	droneBase = glm::rotate(droneBase, droneRotationY, glm::vec3(0, 1, 0));

	//drone dimensions
    glm::vec3 armScale = glm::vec3(1.f, 0.1f, 0.1f); 
    glm::vec3 cubeScale = glm::vec3(0.1f, 0.1f, 0.1f); 
    glm::vec3 propellerScale = glm::vec3(3.0f, 0.1f, 0.5f);

    glm::mat4 mainArm = glm::translate(droneBase, glm::vec3(0, 1, 0));
	mainArm = glm::rotate(mainArm, RADIANS(45.0f), glm::vec3(0, 1, 0));
    mainArm = glm::scale(mainArm, armScale);
    RenderSimpleMesh(meshes["cube"], shaders["VertexNormal"], mainArm);

    glm::mat4 secondaryArm = glm::translate(droneBase, glm::vec3(0, 1, 0));
    secondaryArm = glm::rotate(secondaryArm, RADIANS(135.0f), glm::vec3(0, 1, 0));
    secondaryArm = glm::scale(secondaryArm, armScale);
    RenderSimpleMesh(meshes["cube"], shaders["VertexNormal"], secondaryArm);
    
	//propellers support
    float cubeOffsetDistance = sqrt(2) * (armScale.x / 2.0f - cubeScale.x / 2.0f);
    glm::vec3 cubeOffsets[4] = {
        glm::vec3(-cubeOffsetDistance, 1.1f, cubeOffsetDistance),
        glm::vec3(cubeOffsetDistance, 1.1f, cubeOffsetDistance),  
        glm::vec3(-cubeOffsetDistance, 1.1f, -cubeOffsetDistance), 
        glm::vec3(cubeOffsetDistance, 1.1f, -cubeOffsetDistance)  
    };

	//propellers
    for (int i = 0; i < 4; i++) {
        glm::mat4 cubeModel = glm::translate(droneBase, cubeOffsets[i]);
		cubeModel = glm::rotate(cubeModel, RADIANS(45.0f), glm::vec3(0, 1, 0));
        cubeModel = glm::scale(cubeModel, cubeScale);
        RenderSimpleMesh(meshes["cube"], shaders["VertexNormal"], cubeModel);

        glm::mat4 propellerModel = glm::translate(cubeModel, glm::vec3(0, 1.0f + propellerScale.y, 0));
        float angle = static_cast<float>(Engine::GetElapsedTime() * 5.0);   
        propellerModel = glm::rotate(propellerModel, angle, glm::vec3(0, 1, 0));
        propellerModel = glm::scale(propellerModel, propellerScale);

        RenderSimpleMesh(meshes["cube2"], shaders["VertexNormal"], propellerModel);
    }

    //minimap
    glViewport(0, 0, 256, 256);
    RenderSimpleMesh(meshes["terrain"], shaders["LabShader"], terrainModel, true);
    
    for (const auto& cube : cubes) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, cube.position);
        modelMatrix = glm::scale(modelMatrix, cube.scale);
        RenderSimpleMesh(meshes["cube3"], shaders["VertexNormal"], modelMatrix, true);
    }

    for (const auto& tree : trees) {
     
        glm::mat4 trunkModel = glm::mat4(1);
        trunkModel = glm::translate(trunkModel, tree.position);
        trunkModel = glm::scale(trunkModel, glm::vec3(tree.trunkRadius, tree.trunkHeight, tree.trunkRadius));
        RenderSimpleMesh(meshes["cylinderu"], shaders["VertexNormal"], trunkModel, true);

        glm::mat4 foliageModel1 = glm::mat4(1);
        foliageModel1 = glm::translate(glm::mat4(1), tree.position + glm::vec3(0, tree.trunkHeight, 0));
        foliageModel1 = glm::scale(foliageModel1, glm::vec3(tree.foliageRadius, tree.foliageHeight, tree.foliageRadius));
        RenderSimpleMesh(meshes["cone"], shaders["VertexNormal"], foliageModel1, true);

        glm::mat4 foliageModel2 = glm::mat4(1);
        foliageModel2 = glm::translate(glm::mat4(1), tree.position + glm::vec3(0, tree.trunkHeight + tree.foliageHeight, 0));
        foliageModel2 = glm::scale(foliageModel2, glm::vec3(tree.foliageRadius * 0.8f, tree.foliageHeight * 0.8f, tree.foliageRadius * 0.8f));
        RenderSimpleMesh(meshes["cone"], shaders["VertexNormal"], foliageModel2, true);
    }

    RenderSimpleMesh(meshes["cube"], shaders["VertexNormal"], mainArm, true);
    RenderSimpleMesh(meshes["cube"], shaders["VertexNormal"], secondaryArm, true);
	HandleDelivery2();
}

bool Tema2::CheckDroneObstacleCollision(const glm::vec3& obstacleMin, const glm::vec3& obstacleMax) {
    float x = max(obstacleMin.x, min(dronePosition.x, obstacleMax.x));
    float y = max(obstacleMin.y, min(dronePosition.y, obstacleMax.y));
    float z = max(obstacleMin.z, min(dronePosition.z, obstacleMax.z));

    float distance = sqrt((x - dronePosition.x) * (x - dronePosition.x) +
        (y - (dronePosition.y)) * (y - (dronePosition.y)) +
        (z - dronePosition.z) * (z - dronePosition.z));

    return distance < 0.5f;
}

bool Tema2::CheckSphereCollision(const glm::vec3& sphere1Center, float sphere1Radius, const glm::vec3& sphere2Center, float sphere2Radius) {
    float distanceSquared = glm::distance2(sphere1Center, sphere2Center);
    float radiusSum = sphere1Radius + sphere2Radius;
    return distanceSquared <= radiusSum * radiusSum;
}

void Tema2::HandleCollisions(float deltaTimeSeconds) {
	//drone - terrain
    float mapSize = terrainSize * terrainCellSize / 2.0f;

    if (dronePosition.x < -mapSize) {
        dronePosition.x = -mapSize;
    }
    if (dronePosition.x > mapSize) {
        dronePosition.x = mapSize;  
    }
    if (dronePosition.z < -mapSize) {
        dronePosition.z = -mapSize; 
    }
    if (dronePosition.z > mapSize) {
        dronePosition.z = mapSize;  
    }
    if (dronePosition.y < 0.0f) {
		dronePosition.y = 0.0f;
    }

	//drone - buildings
    for (const auto& cube : cubes) {
        glm::vec3 cubeMin = cube.position - cube.scale; 
        glm::vec3 cubeMax = cube.position + cube.scale;

        if (CheckDroneObstacleCollision(cubeMin, cubeMax)) {
            glm::vec3 direction = glm::normalize(dronePosition - cube.position); 
            dronePosition += direction * deltaTimeSeconds * droneSpeed;  
        }
    }

	//dronă - trees
    for (const auto& tree : trees) {
        glm::vec3 trunkMin = tree.position - glm::vec3(tree.trunkRadius, 0, tree.trunkRadius);
        glm::vec3 trunkMax = tree.position + glm::vec3(tree.trunkRadius, tree.trunkHeight, tree.trunkRadius);

        if (CheckDroneObstacleCollision(trunkMin, trunkMax)) {
            glm::vec3 direction = glm::normalize(dronePosition - tree.position);
            dronePosition += direction * deltaTimeSeconds * droneSpeed;
        }

        glm::vec3 crownCenter = tree.position + glm::vec3(0, tree.trunkHeight + tree.foliageHeight * 0.5f, 0);

        float crownRadius = glm::sqrt(glm::pow(tree.foliageRadius, 2) + glm::pow(tree.foliageHeight * 0.5f, 2));

        if (CheckSphereCollision(dronePosition, 0.5f, crownCenter, crownRadius * 1.5f)) {
            glm::vec3 direction = glm::normalize(dronePosition - crownCenter);
            dronePosition += direction * deltaTimeSeconds * droneSpeed;
        }
    }
}

void Tema2::HandleDelivery() {
    // se alege aleatoriu locatia pachetului
    if (!carryingPackage && !packageActive) {
        packageBuildingIndex = rand() % cubes.size();
        while (packageBuildingIndex == olddestination) {
            packageBuildingIndex = rand() % cubes.size();
        }
        packagePosition = cubes[packageBuildingIndex].position + glm::vec3(0, cubes[packageBuildingIndex].scale.y, 0);
        packageActive = true;
        deliveryCircleActive = false;
    }
    //pachet randat pe cladire
    if (!carryingPackage && packageActive) {
        glm::mat4 packageModel = glm::mat4(1);
        packageModel = glm::translate(packageModel, packagePosition);
        packageModel = glm::scale(packageModel, packageScale);
        RenderSimpleMesh(meshes["package"], shaders["VertexNormal"], packageModel);
    }

	// se alege aleatoriu locatia de livrare
    if (carryingPackage && !deliveryCircleActive) {
        destinationBuildingIndex = rand() % cubes.size();
        while (destinationBuildingIndex == packageBuildingIndex && destinationBuildingIndex == olddestination) {
            destinationBuildingIndex = rand() % cubes.size();
			olddestination = destinationBuildingIndex;
        }

        destinationPosition = cubes[destinationBuildingIndex].position + glm::vec3(0, cubes[destinationBuildingIndex].scale.y + deliveryCircleScale.y, 0);
        deliveryCircleActive = true;
    }
    // livrare
    if (carryingPackage) {
        packagePosition = dronePosition + glm::vec3(0, 1.f - (packageScale.y + 0.1f), 0);

        glm::mat4 packageModel = glm::mat4(1);
        packageModel = glm::translate(packageModel, packagePosition);
        packageModel = glm::scale(packageModel, packageScale);
        RenderSimpleMesh(meshes["package"], shaders["VertexNormal"], packageModel);

        glm::mat4 circleModel = glm::mat4(1);
        circleModel = glm::translate(circleModel, destinationPosition);
        circleModel = glm::scale(circleModel, deliveryCircleScale);
        RenderSimpleMesh(meshes["circle_red"], shaders["VertexNormal"], circleModel);
    }

    // pachet livrat, cercul ramane verde
    if (!carryingPackage && deliveryComplete) {
        glm::mat4 greencircleModel = glm::mat4(1);
        greencircleModel = glm::translate(greencircleModel, destinationPosition);
        greencircleModel = glm::scale(greencircleModel, deliveryCircleScale);
        RenderSimpleMesh(meshes["circle_green"], shaders["VertexNormal"], greencircleModel);
    }
}

void Tema2::HandleDelivery2() {
    // se alege aleatoriu locatia pachetului
    if (!carryingPackage && !packageActive) {
        packageBuildingIndex = rand() % cubes.size();
        while (packageBuildingIndex == olddestination) {
            packageBuildingIndex = rand() % cubes.size();
        }
        packagePosition = cubes[packageBuildingIndex].position + glm::vec3(0, cubes[packageBuildingIndex].scale.y, 0);
        packageActive = true;
        deliveryCircleActive = false;
    }
    //pachet randat pe cladire
    if (!carryingPackage && packageActive) {
        glm::mat4 packageModel = glm::mat4(1);
        packageModel = glm::translate(packageModel, packagePosition);
        packageModel = glm::scale(packageModel, packageScale);
        RenderSimpleMesh(meshes["package"], shaders["VertexNormal"], packageModel, true);
    }

    // se alege aleatoriu locatia de livrare
    if (carryingPackage && !deliveryCircleActive) {
        destinationBuildingIndex = rand() % cubes.size();
        while (destinationBuildingIndex == packageBuildingIndex && destinationBuildingIndex == olddestination) {
            destinationBuildingIndex = rand() % cubes.size();
            olddestination = destinationBuildingIndex;
        }

        destinationPosition = cubes[destinationBuildingIndex].position + glm::vec3(0, cubes[destinationBuildingIndex].scale.y + deliveryCircleScale.y, 0);
        deliveryCircleActive = true;
    }
    // livrare
    if (carryingPackage) {
        packagePosition = dronePosition + glm::vec3(0, 1.f - (packageScale.y + 0.1f), 0);

        glm::mat4 packageModel = glm::mat4(1);
        packageModel = glm::translate(packageModel, packagePosition);
        packageModel = glm::scale(packageModel, packageScale);
        RenderSimpleMesh(meshes["package"], shaders["VertexNormal"], packageModel, true);

        glm::mat4 circleModel = glm::mat4(1);
        circleModel = glm::translate(circleModel, destinationPosition);
        circleModel = glm::scale(circleModel, deliveryCircleScale);
        RenderSimpleMesh(meshes["circle_red"], shaders["VertexNormal"], circleModel, true);
    }

    // pachet livrat, cercul ramane verde
    if (!carryingPackage && deliveryComplete) {
        glm::mat4 greencircleModel = glm::mat4(1);
        greencircleModel = glm::translate(greencircleModel, destinationPosition);
        greencircleModel = glm::scale(greencircleModel, deliveryCircleScale);
        RenderSimpleMesh(meshes["circle_green"], shaders["VertexNormal"], greencircleModel, true);
    }
}

void Tema2::FrameEnd()
{
   // DrawCoordinateSystem();
}

void Tema2::RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 & modelMatrix, bool minimap)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

  
    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Get shader location for uniform mat4 "Model"
    GLint modelLocation = glGetUniformLocation(shader->GetProgramID(), "Model");

    // Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Get shader location for uniform mat4 "View"
    GLint viewLocation = glGetUniformLocation(shader->GetProgramID(), "View");

    // Set shader uniform "View" to viewMatrix
    if (!minimap) {
        glm::mat4 viewMatrix = camera->GetViewMatrix();
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	}
	else {
		glm::mat4 viewMatrix = minimapCamera->GetViewMatrix();
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	}
    
    // Get shader location for uniform mat4 "Projection"
    GLint projLocation = glGetUniformLocation(shader->GetProgramID(), "Projection");

    // Set shader uniform "Projection" to projectionMatrix
	if (!minimap) {
		glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
		glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	}
    else {
        glm::mat4 projectionMatrix = minimapCamera->GetProjectionMatrix();
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    }

    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    glm::mat4 forwardMatrix = glm::rotate(glm::mat4(1.0f), droneRotationY, glm::vec3(0, 1, 0));

    glm::vec3 forwardDirection = glm::vec3(forwardMatrix[2]);
    glm::vec3 rightDirection = glm::vec3(forwardMatrix[0]);

	// forward
    if (window->KeyHold(GLFW_KEY_W)) {
        dronePosition -= forwardDirection * droneSpeed * deltaTime;
    }

	// backward
    if (window->KeyHold(GLFW_KEY_S)) {
        dronePosition += forwardDirection * droneSpeed * deltaTime;
    }

	// left
    if (window->KeyHold(GLFW_KEY_A)) {
        dronePosition -= rightDirection * droneSpeed * deltaTime;
    }

	// right
    if (window->KeyHold(GLFW_KEY_D)) {
        dronePosition += rightDirection * droneSpeed * deltaTime;
    }

    // up
    if (window->KeyHold(GLFW_KEY_UP)) {
        dronePosition.y += droneSpeed * deltaTime; 
    }

    // down
    if (window->KeyHold(GLFW_KEY_DOWN)) {
        dronePosition.y -= droneSpeed * deltaTime;
    }

	// rotate left
    if (window->KeyHold(GLFW_KEY_LEFT)) {
        droneRotationY += droneRotationSpeed * deltaTime;
    }

	// rotate right
    if (window->KeyHold(GLFW_KEY_RIGHT)) {
        droneRotationY -= droneRotationSpeed * deltaTime;
    }

    glm::vec3 cameraPosition = dronePosition - 5.f * glm::vec3(-forwardDirection.x, -0.5f, -forwardDirection.z);
    camera->Set(cameraPosition, dronePosition, glm::vec3(0, 1, 0));

	glm::vec3 minimapCameraPosition = glm::vec3(dronePosition.x, 100, dronePosition.z);
	minimapCamera->Set(minimapCameraPosition, dronePosition, glm::vec3(1, 0, 0));
}

void Tema2::OnKeyPress(int key, int mods) {
    if (key == GLFW_KEY_J && !carryingPackage) {
        if (glm::distance(dronePosition, packagePosition) < 1.0f) {
            carryingPackage = true;
            packageActive = false;
        }
    }

    if (key == GLFW_KEY_K && carryingPackage) {
        if (glm::distance(dronePosition, destinationPosition) < 1.0f) {
            carryingPackage = false;
            deliveryComplete = true;
            deliveryCircleActive = false;
			counter++;
			cout << "Pachet livrat! Ai scorul: " << counter << endl;
        }
    }
}

void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event

}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// Add mouse scroll event
}

void Tema2::OnWindowResize(int width, int height)
{
}
