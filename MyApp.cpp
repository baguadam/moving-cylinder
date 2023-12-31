#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"

#include <imgui.h>
#include <iostream>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	AssembleProgram(m_programID, "Vert_PosCol.vert", "Frag_PosCol.frag");

}

void CMyApp::CleanShaders()
{
	glDeleteProgram(m_programID);
}

void CMyApp::AddCircleCoordinates(MeshObject<VertexPosColor>& meshCPU, int coordinateZ) {
	meshCPU.vertexArray.push_back({ glm::vec3(0, 0, coordinateZ), glm::vec3(1, 0, coordinateZ / 2) });

	// Kör koordinátáinak kiszámítása egységkör alapján
	for (int i = 0; i < triangleCount; ++i) {
		double colorValue = static_cast<double>(i) / triangleCount;
		meshCPU.vertexArray.push_back(
			{ glm::vec3(cos((360 / triangleCount) * (i + 1) * M_PI / 180) / 2,
					    sin((360 / triangleCount) * (i + 1) * M_PI / 180) / 2,
					    coordinateZ),
			 glm::vec3(colorValue, coordinateZ / 2, colorValue) } // szükséges koordináták meghatározása az "egységkörben"
		);
	}
}

void CMyApp::AddWallIndexes(MeshObject<VertexPosColor>& meshCPU) {
	// hör háromszögeinek összekötése minden minden körháromszög, kivéve az utolsó esetén
	for (int i = 1; i < triangleCount; ++i) {
		meshCPU.indexArray.push_back(i); // alsó körön a "bal alsó" koordinána
		meshCPU.indexArray.push_back(triangleCount + i + 2); // felső körön a "jobb felső" koordináta
		meshCPU.indexArray.push_back(triangleCount + i + 1); // felső körön a "bal felső" koordináta

		meshCPU.indexArray.push_back(i); // alsó kör "bal alsó"
		meshCPU.indexArray.push_back(i + 1); // alsó kör "jobb alsó"
		meshCPU.indexArray.push_back(triangleCount + i + 2); // felső kör "jobb felső"
	}

	// utolsó háromszögek között az egyik háromszög kirajzolása: alsó utolsó koordinátája + felső első koordinátája + felső utolsó koordinátája
	meshCPU.indexArray.push_back(triangleCount); // alsó "bal alsó"
	meshCPU.indexArray.push_back(triangleCount + 2); // felső "jobb felső"
	meshCPU.indexArray.push_back(2 * triangleCount + 1); // felső "bal felső"

	// szintén utolsó háromszögek között a másik háromszög: alső utolsó + alsó első + felső első
	meshCPU.indexArray.push_back(triangleCount); // alsó "bal alsó"
	meshCPU.indexArray.push_back(1); // alsó "jobb alsó"
	meshCPU.indexArray.push_back(triangleCount + 2); // felső "bal felső"
}

void CMyApp::AddCircleIndexes(MeshObject<VertexPosColor>& meshCPU, int from, int to, bool isBaseCircle) {
	// Hozzáadjuk a szükséges indexeket a tömbhöz
	for (int i = from; i < to; ++i) {
		meshCPU.indexArray.push_back(from - 1);
		if (isBaseCircle) {
			meshCPU.indexArray.push_back(i + 1);
			meshCPU.indexArray.push_back(i);
		}
		else {
			meshCPU.indexArray.push_back(i);
			meshCPU.indexArray.push_back(i + 1);
		}
	}

	// utolsó háromszög kirajzolása, ez köti össze az origot, az utolsó pontot és a legelső pontot egymással
	meshCPU.indexArray.push_back(from - 1);
	if (isBaseCircle) {
		meshCPU.indexArray.push_back(from);
		meshCPU.indexArray.push_back(to);
	}
	else {
		meshCPU.indexArray.push_back(to);
		meshCPU.indexArray.push_back(from);
	}
}

void CMyApp::InitGeometry()
{
	MeshObject<VertexPosColor> meshCPU;

	AddCircleCoordinates(meshCPU, 0); // alapkör koordinátái
	AddCircleCoordinates(meshCPU, 2); // borítókör koordinátái

	AddCircleIndexes(meshCPU, 1, triangleCount, true); // első index, ami nem az origo, utolsó index, alapkör-e 
	AddCircleIndexes(meshCPU, triangleCount + 2, 2 * triangleCount + 1, false); // hasonlóan első index, ami nem az origó, ez a háromszögek száma + 2 a két origo miatt

	AddWallIndexes(meshCPU);

	// 1 db VAO foglalasa
	glGenVertexArrays(1, &vaoID);
	// a frissen generált VAO beallitasa aktívnak
	glBindVertexArray(vaoID);

	// hozzunk létre egy új VBO erőforrás nevet
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID); // tegyük "aktívvá" a létrehozott VBO-t

	// töltsük fel adatokkal az aktív VBO-t
	glBufferData(GL_ARRAY_BUFFER,	// az aktív VBO-ba töltsünk adatokat
		meshCPU.vertexArray.size() * sizeof(VertexPosColor),		// ennyi bájt nagyságban
		meshCPU.vertexArray.data(),	// erről a rendszermemóriabeli címről olvasva
		GL_STATIC_DRAW);	// úgy, hogy a VBO-nkba nem tervezünk ezután írni és minden kirajzoláskor felhasnzáljuk a benne lévő adatokat

	glEnableVertexAttribArray(0); // ez lesz majd a pozíció
	glVertexAttribPointer(
		0,						  // a VB-ben található adatok közül a 0. "indexű" attribútumait állítjuk be
		3,						  // komponens szam
		GL_FLOAT,				  // adatok tipusa
		GL_FALSE,				  // normalizalt legyen-e
		sizeof(VertexPosColor),   // stride (0=egymas utan)
		reinterpret_cast<const void*>(offsetof(VertexPosColor, position)) // a 0. indexű attribútum hol kezdődik a sizeof(Vertex)-nyi területen belül
	);

	glEnableVertexAttribArray(1); // ez lesz majd a pozíció
	glVertexAttribPointer(
		1,						  // a VB-ben található adatok közül a 1. "indexű" attribútumait állítjuk be
		3,						  // komponens szam
		GL_FLOAT,				  // adatok tipusa
		GL_FALSE,				  // normalizalt legyen-e
		sizeof(VertexPosColor),   // stride (0=egymas utan)
		reinterpret_cast<const void*>(offsetof(VertexPosColor, color)) // a 1. indexű attribútum hol kezdődik a sizeof(Vertex)-nyi területen belül
	);

	// index puffer létrehozása
	glGenBuffers(1, &iboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		meshCPU.indexArray.size() * sizeof(GLuint),
		meshCPU.indexArray.data(),
		GL_STATIC_DRAW);

	count = static_cast<GLsizei>(meshCPU.indexArray.size());

	glBindVertexArray(0); // Kapcsoljuk ki a VAO-t!
}

void CMyApp::CleanGeometry()
{
	glDeleteBuffers(1, &vboID);
	glDeleteBuffers(1, &iboID);
	glDeleteVertexArrays(1, &vaoID);
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 0.0, 5.0),// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up


	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
}

void CMyApp::Update(const SUpdateInfo& updateInfo)
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
	m_camera.Update(updateInfo.DeltaTimeInSec);
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - VAO beállítása
	glBindVertexArray(vaoID);

	// - shader bekapcsolasa
	glUseProgram(m_programID);
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj())); //ha egyszer leküldöm, ő már nem változik

	// az origó középpontű, 5 egység élhosszúságú kocka csúcsai
	glm::vec3 cubeVertices[] = {
		glm::vec3( 2.5f, -2.5f, -2.5f),
		glm::vec3(-2.5f, -2.5f, -2.5f),
		glm::vec3(-2.5f,  2.5f, -2.5f),
		glm::vec3( 2.5f,  2.5f, -2.5f),
		glm::vec3( 2.5f, -2.5f,  2.5f),
		glm::vec3(-2.5f, -2.5f,  2.5f),
		glm::vec3( 2.5f,  2.5f,  2.5f),
		glm::vec3(-2.5f,  2.5f,  2.5f),
	};

	float x = std::sinf(2.0f * glm::pi<float>() * 1.0f/6.0f * m_ElapsedTimeInSec) * 10;
	float z = 0.01f * std::pow(x, 3) + 0.05f * std::pow(x, 2);
	glm::vec3 moveVector(x, 0, z);

	if (isSpacePushed) {
		zAxisSize = 1.0f + (std::sinf(2.0f * glm::pi<float>() * 1.0f/10.0f * m_ElapsedTimeInSec) * 0.75f + 0.25f);
	}

	// nyolc alakzat transzformálása és kirajzolása a megfelelő helyre
	for (auto vertex : cubeVertices) {
		glm::mat4 matWorld = glm::translate(vertex + moveVector) * glm::scale(glm::vec3(1.0f, 1.0f, zAxisSize));
		glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	// shader kikapcsolasa
	glUseProgram(0);

	// VAO kikapcsolása
	glBindVertexArray(0);
}

void CMyApp::RenderGUI()
{
	// ImGui::ShowDemoWindow();
}

GLint CMyApp::ul(const char* uniformName) noexcept
{
	GLuint programID = 0;

	// Kérdezzük le az aktuális programot!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
	glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&programID));
	// A program és a uniform név ismeretében kérdezzük le a location-t!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
	return glGetUniformLocation(programID, uniformName);
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{
	if (key.repeat == 0) // Először lett megnyomva
	{
		if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL)
		{
			CleanShaders();
			InitShaders();
		}
		if (key.keysym.sym == SDLK_F1)
		{
			GLint polygonModeFrontAndBack[2] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv(GL_POLYGON_MODE, polygonModeFrontAndBack); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL ? GL_FILL : GL_LINE); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode(GL_FRONT_AND_BACK, polygonMode); // Állítsuk be az újat!
		}
		if (key.keysym.sym == SDLK_SPACE) {
			isSpacePushed = !isSpacePushed;
		}
	}
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_camera.MouseWheel(wheel);
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.Resize(_w, _h);
}

