#pragma once

#include <vector>

#include "components/simple_scene.h"
#include "lab_m1/tema2/camera.h"


namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

        //creare meshuri
        Mesh *CreateMesh(const char *name, const std::vector<VertexFormat> &vertices, const std::vector<unsigned int> &indices);
        Mesh *CreateTerrainMesh(int m, int n, float cellSize);
        Mesh *Tema2::CreateCylinder(const std::string& name, float radius, float height, int segments);
        Mesh* Tema2::CreateCone(const std::string& name, float radius, float height, int segments);
        Mesh* Tema2::CreateBox(const std::string& name, const glm::vec3& color);
        Mesh* Tema2::CreateArrowShape(const std::string& name, const glm::vec3& color);
        Mesh* Tema2::CreateCircle(const std::string& name, float radius, int resolution, const glm::vec3& color);
        
		//generare copaci si cuburi
        void Tema2::GenerateTrees(int minTrees, int maxTrees, float terrainWidth, float terrainHeight, float minDistance);
        void Tema2::GenerateCubes(int minCubes, int maxCubes, float terrainWidth, float terrainHeight, float minDistance);
        bool Tema2::IsPositionValid(const glm::vec3& position, float minDistance);

        //coliziune
        bool CheckDroneObstacleCollision(const glm::vec3& obstacleMin, const glm::vec3& obstacleMax);
        bool CheckSphereCollision(const glm::vec3& sphere1Center, float sphere1Radius, const glm::vec3& sphere2Center, float sphere2Radius);
        void Tema2::HandleCollisions(float deltaTimeSeconds);
		
        //joc livrare        
        void Tema2::HandleDelivery();
        void Tema2::HandleDelivery2();

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, bool minimap = false);
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        Mesh* terrainMesh;
        float terrainSize = 100.0f;
        float terrainCellSize = 1.0f;
        
        struct TreeInfo {
            glm::vec3 position;
            float trunkHeight;
            float trunkRadius;
            float foliageHeight;
            float foliageRadius;
        };

        struct CubeInfo {
            glm::vec3 position;
            glm::vec3 scale;  
        };
     
        std::vector<TreeInfo> trees;
        std::vector<CubeInfo> cubes;

		int minTrees = 20; // numarul minim de copaci
		int maxTrees = 30; // numarul maxim de copaci
		int minCubes = 5; // numarul minim de cuburi
		int maxCubes = 8; // numarul maxim de cuburi

        //drona
        glm::vec3 dronePosition = glm::vec3(0, 1, 0); // pozitia dronei
		float droneRotationY = 0; // rotatia dronei pe OY
        float droneSpeed = 5.0f; // viteza dronei
		float droneRotationSpeed = 1.0f; // viteza de rotatie a dronei
        
        //joc livrare
        glm::vec3 packagePosition;  // pozitia pachetului
        glm::vec3 packageScale = glm::vec3(0.5f, 0.3f, 0.5f); // dimensiuni pachet
        bool carryingPackage; // indica daca drona transporta un pachet
        bool inDelivery; //status drona
        bool packageActive;  // indica daca exista un pachet care poate fi livrat

        glm::vec3 deliveryCircleScale = glm::vec3(1.0f, 0.1f, 1.0f);  // dimensiunea cercului
        bool deliveryCircleActive;  // indica daca cercul de livrare este activ
        bool deliveryComplete;  // indica dacă livrarea a fost finalizata
		
        glm::vec3 destinationPosition;  // pozitia destinatiei
        int packageBuildingIndex = -1;       // indexul cladirii pe care este pachetul
        int destinationBuildingIndex = -1;  // indexul cladirii pe care este cercul rosu
		int olddestination; // indexul cladirii precedente
        int counter; // scorul jocului
        
        //camera
        implemented::Camera* camera;
		implemented::Camera* minimapCamera;
    };
}   // namespace m1
