#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/Log.h"
#include "CinderImGui.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Material {
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;
};

struct Light {
	float position[3];
	float intensity; // assume the light is white
					 // attenuation parameters
	float constant;
	float linear;
	float quadratic;
};

class PlaygroundGLApp : public App {
public:
	void setup() override;
	void mouseWheel(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseMove(MouseEvent event) override;
	void resize() override { camera.setAspectRatio(getWindowAspectRatio()); }
	void update() override;
	void draw() override;

	gl::BatchRef model;
	gl::BatchRef wirePlane;
	
	CameraPersp camera;
	vec3 target = { 0, 0.5, 0 };

	glm::ivec2 deltaPos; // delta movement of mouse
	glm::ivec2 lastPos = { 0, 0 };

	// parameters in shader
	/// light
	Material material;
	Light light;

	void GuiWindow();
	void UpdateParametersInShader();
};

void PlaygroundGLApp::setup()
{
	// load shader
	auto phong_shader = gl::GlslProg::create(loadAsset("shaders/phong.vert"), loadAsset("shaders/phong.frag"));

	// setup camera, light and material
	camera.lookAt({ 0, 2, 3 }, target);
	camera.setFov(45);
	camera.setAspectRatio(getWindowAspectRatio());
	light = Light{
		{ 1.f,1.f,1.f },
		1.f,
		1.f,
		0.09f,
		0.032f
	};
	material = Material{
		{ 0.1f, 0.1f, 0.1f },
		{ 1.f,1.f,1.f },
		{ 0.5f,0.5f,0.5f },
		32.f };
	
	model = gl::Batch::create(geom::Teapot().subdivisions(20), phong_shader);
	wirePlane = gl::Batch::create(
		geom::WirePlane().size({ 10, 10 }).subdivisions({ 10,10 }),
		gl::getStockShader(gl::ShaderDef()));

	UpdateParametersInShader();

	// global gl settings
	gl::enableDepthRead();
	gl::enableDepthWrite();
	ui::initialize();
}

void PlaygroundGLApp::mouseMove(MouseEvent event)
{
	// update deltaPos
	deltaPos = event.getPos() - lastPos;
	lastPos = event.getPos();
}

void PlaygroundGLApp::mouseDrag(MouseEvent event)
{
	// update deltaPos
	deltaPos = event.getPos() - lastPos;
	lastPos = event.getPos();

	// moving camera around a crytal ball
	auto eye_point = camera.getEyePoint();

	eye_point = glm::rotateY(eye_point, -deltaPos.x*0.01f);
	auto up = glm::vec3{ 0,1,0 };
	auto right = glm::cross(up, eye_point);
	eye_point = glm::rotate(angleAxis(-deltaPos.y*0.003f, right), eye_point);

	camera.setEyePoint(eye_point);
	camera.lookAt(target);
	//console() << "Cemara Moved to " << eye_point << endl;
}

void PlaygroundGLApp::mouseWheel(MouseEvent event)
{
	// change camera FOV when scolling mousewheel to simulate zooming
	camera.setFov(math<float>::clamp(camera.getFov() - event.getWheelIncrement(), 10, 90));
	//console() << "FOV changed to " << camera.getFov() << endl;
}

void PlaygroundGLApp::GuiWindow()
{
	ui::ScopedWindow window{ "Parameters" };
	ui::Text("Frame Rate: %d", (int)getFrameRate());
	ui::Text("left click to move camera,\nscroll mousewheel to zoom in/out");
	if (ui::CollapsingHeader("Material"))
	{
		if (ui::ColorEdit3("Ambient", material.ambient))
		{
			model->getGlslProg()->uniform(
				"material.ambient",
				glm::vec3{ material.ambient[0], material.ambient[1], material.ambient[2] }
			);
		}
		if (ui::ColorEdit3("Diffuse", material.diffuse))
		{
			model->getGlslProg()->uniform(
				"material.diffuse",
				glm::vec3{ material.diffuse[0], material.diffuse[1], material.diffuse[2] }
			);
		}
		if (ui::ColorEdit3("Specular", material.specular))
		{
			model->getGlslProg()->uniform(
				"material.specular",
				glm::vec3{ material.specular[0], material.specular[1], material.specular[2] }
			);
		}
		if (ui::SliderFloat("Shininess", &material.shininess, 8, 64))
		{
			model->getGlslProg()->uniform(
				"material.shininess",
				material.shininess
			);
		}
	}
	if (ui::CollapsingHeader("Light"))
	{
		if (ui::DragFloat3("position", light.position, 0.1f))
		{
			model->getGlslProg()->uniform(
				"light.position",
				glm::vec3{ light.position[0], light.position[1], light.position[2] }
			);
		}
		if (ui::SliderFloat("intensity", &light.intensity, 0, 1))
		{
			model->getGlslProg()->uniform(
				"light.intensity",
				light.intensity
			);
		}
	}
}


void PlaygroundGLApp::update()
{
	GuiWindow();
}

void PlaygroundGLApp::draw()
{
	gl::clear();
	gl::setMatrices(camera);
	model->draw();
	wirePlane->draw();
	gl::color(Color{ light.intensity,light.intensity,light.intensity });
	gl::drawSphere(cinder::Sphere{ {light.position[0],light.position[1],light.position[2]}, 0.03f }, 100);
}

void PlaygroundGLApp::UpdateParametersInShader()
{
	model->getGlslProg()->uniform(
		"material.ambient",
		glm::vec3{ material.ambient[0], material.ambient[1], material.ambient[2] }
	);
	
	model->getGlslProg()->uniform(
		"material.diffuse",
		glm::vec3{ material.diffuse[0], material.diffuse[1], material.diffuse[2] }
	);
	
	model->getGlslProg()->uniform(
		"material.specular",
		glm::vec3{ material.specular[0], material.specular[1], material.specular[2] }
	);
	
	model->getGlslProg()->uniform(
		"material.shininess",
		material.shininess
	);
	model->getGlslProg()->uniform(
		"light.position",
		glm::vec3{ light.position[0], light.position[1], light.position[2] }
	);
	
	model->getGlslProg()->uniform(
		"light.intensity",
		light.intensity
	);
	model->getGlslProg()->uniform(
		"light.constant",
		light.constant
	);
	model->getGlslProg()->uniform(
		"light.linear",
		light.linear
	);
	model->getGlslProg()->uniform(
		"light.quadratic",
		light.quadratic
	);
}

CINDER_APP(
	PlaygroundGLApp,
	RendererGl{ RendererGl::Options().version(3,3).msaa(4) },
	[](App::Settings *settings) {
	settings->setTitle("Opengl");
	settings->setResizable(true);
	settings->setWindowSize({ 1280, 720 });
	settings->setConsoleWindowEnabled(false);
});