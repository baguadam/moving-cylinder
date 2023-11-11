#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include "GLUtils.hpp"
#include "Camera.h"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f; // Program indulása óta eltelt idő
	float DeltaTimeInSec   = 0.0f; // Előző Update óta eltelt idő
};

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update( const SUpdateInfo& );
	void Render();
	void RenderGUI();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	void SetupDebugCallback();

	//
	// Adat változók
	//

	float m_ElapsedTimeInSec = 0.0f;

	// Kamera
	Camera m_camera;

	//
	// OpenGL-es dolgok
	//
	
	// uniform location lekérdezése
	GLint ul( const char* uniformName ) noexcept;

	// shaderekhez szükséges változók
	GLuint m_programID = 0; // shaderek programja

	// Shaderek inicializálása, és törtlése
	void InitShaders();
	void CleanShaders();

	// Geometriával kapcsolatos változók

	GLuint  vaoID = 0; // vertex array object erőforrás azonosító
	GLuint  vboID = 0; // vertex buffer object erőforrás azonosító
	GLuint  iboID = 0; // index buffer object erőforrás azonosító
	GLsizei count = 0; // mennyi indexet/vertexet kell rajzolnunk
	GLsizei triangleCount = 90;

	GLfloat zAxisSize = 1.0f;
	bool isSpacePushed = false;

	// Geometria inicializálása, és törtlése
	void InitGeometry();
	void CleanGeometry();

private:
	void AddCircleCoordinates(MeshObject<VertexPosColor>&, int);
	void AddCircleIndexes(MeshObject<VertexPosColor>&, int, int, bool);
	void AddWallIndexes(MeshObject<VertexPosColor>&);
};

