
#include <QMouseEvent>
#include <QLabel>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVBoxLayout>
#include <QScreen>
#include <QtMath>

#include <QCheckBox>
#include <QSlider>
#include <array>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Window.h"

namespace
{

//constexpr std::array<GLfloat, 21u> vertices = {
//	0.0f, 0.707f, 1.f, 0.f, 0.f, 0.0f, 0.0f,
//	-0.5f, -0.5f, 0.f, 1.f, 0.f, 0.5f, 1.0f,
//	0.5f, -0.5f, 0.f, 0.f, 1.f, 1.0f, 0.0f,
//};
//constexpr std::array<GLuint, 3u> indices = {0, 1, 2};

}// namespace

Window::Window() noexcept
{
	auto speed_slider = new QSlider();
	speed_slider->setRange(10, 200);
	speed_slider->setSingleStep(10);
	speed_slider->setOrientation(Qt::Vertical);

	auto speed_label = new QLabel("Speed 🚀");
	speed_label->setFont(QFont("Arial", 10));

	auto morphing_slider = new QSlider();
	morphing_slider->setRange(0, 100);
	morphing_slider->setSingleStep(1);
	morphing_slider->setOrientation(Qt::Vertical);

	auto morphing_label = new QLabel("Spherify 🔮");
	morphing_label->setFont(QFont("Arial", 10));

	auto directional_light_checkbox = new QCheckBox();
	directional_light_checkbox->setChecked(false);
	directional_light_checkbox->setText("directional 🌞");

	auto spot_light_checkbox = new QCheckBox();
	spot_light_checkbox->setChecked(false);
	spot_light_checkbox->setText("spot 💡");

	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto layout = new QVBoxLayout();
	layout->addWidget(fps, 2);
	layout->addWidget(speed_slider, 3);
	layout->addWidget(speed_label, 1);
	layout->addWidget(morphing_slider, 3);
	layout->addWidget(morphing_label, 1);
	layout->addWidget(directional_light_checkbox);		// TODO: add other checkboxes + control spaces
	layout->addWidget(spot_light_checkbox);

	setLayout(layout);

	timer_.start();

	connect(this, &Window::updateUI, [=] {
		fps->setText(formatFPS(ui_.fps));
	});
	connect(speed_slider, &QSlider::valueChanged, this, &Window::change_camera_speed);
	connect(morphing_slider, &QSlider::valueChanged, this, &Window::change_morphing_param);
	connect(directional_light_checkbox, &QCheckBox::stateChanged, this, &Window::change_directional_light);
	connect(spot_light_checkbox, &QCheckBox::stateChanged, this, &Window::change_spot_light);
}

Window::~Window()
{
	{
		// Free resources with context bounded.
		const auto guard = bindContext();
		texture_.reset();
		program_.reset();
	}
}

void Window::onInit()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/cube.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/cube.fs");
	program_->link();

	// Create VAO object
	vao_.create();
	vao_.bind();

	// Create VBO
//	vbo_.create();
//	vbo_.bind();
//	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
//	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	// ----------------------------------------------------------------

	loadModel("Models/vert_cube.glb");
	vbos = bindModel();

	// ---------------------------------------------

	// Create IBO
//	ibo_.create();
//	ibo_.bind();
//	ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
//	ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));

	texture_ = std::make_unique<QOpenGLTexture>(QImage(":/Textures/oxy.png"));
	texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	texture_->setWrapMode(QOpenGLTexture::WrapMode::Repeat);

	// Bind attributes
	program_->bind();

//	program_->enableAttributeArray(0);
//	program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, static_cast<int>(7 * sizeof(GLfloat)));
//
//	program_->enableAttributeArray(1);
//	program_->setAttributeBuffer(1, GL_FLOAT, static_cast<int>(2 * sizeof(GLfloat)), 3,
//								 static_cast<int>(7 * sizeof(GLfloat)));
//
//	program_->enableAttributeArray(2);
//	program_->setAttributeBuffer(2, GL_FLOAT, static_cast<int>(5 * sizeof(GLfloat)), 2,
//								 static_cast<int>(7 * sizeof(GLfloat)));
	// Bind attributes

	modelUniform_ = program_->uniformLocation("ModelMat");
	viewUniform_ = program_-> uniformLocation("ViewMat");
	projectionUniform_ = program_->uniformLocation("ProjMat");
	sunCoord_ = program_->uniformLocation("sun_coord");
	normalTrasform_ = program_->uniformLocation("normalMV");
	isDirectionalLightUniform_ = program_->uniformLocation("directional");
	isSpotLightUniform_ = program_->uniformLocation("spot");
	morphingParam_ = program_->uniformLocation("morphing_coef");
	spotPositionUniform_ = program_->uniformLocation("spot_position");
	spotDirection_ = program_->uniformLocation("spot_direction");
	// spotAngle_ = program_->uniformLocation("spot_angle");

	// Release all
	program_->release();

	vao_.release();

//	ibo_.release();
//	vbo_.release();

	// Еnable depth test and face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set initial parameters
	cameraPos_ = glm::vec3(0, 2, 7);
	cameraFront_ = glm::vec3(0, 0, -4);
	cameraUp_ = glm::vec3(0, 1, 0);

	// free movement parameters
	yawAngle_ = -90.;
	pitchAngle_ = -13.6;
	cameraSpeed_ = 0.01;
	calculate_camera_front();

	// light parameters
	is_directional = false;
	is_spot = false;
	spotPosition = glm::vec3(0.0, 5.0, 3.0);

	// morphing parameters
	morphing_param = 100;
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform value
//	program_->setUniformValue(mvpUniform_, mvp);

	// Activate texture unit and bind texture
	glActiveTexture(GL_TEXTURE0);
	texture_->bind();

	// Draw
	display();

	// Release VAO and shader program
	texture_->release();
	vao_.release();
	program_->release();

	++frameCount_;

	// Request redraw if animated
	if (animated_)
	{
		update();
	}
}

void Window::onResize(const size_t width, const size_t height)
{
//	// Configure viewport
//	glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
//
//	// Configure matrix
//	const auto aspect = static_cast<float>(width) / static_cast<float>(height);
//	const auto zNear = 0.1f;
//	const auto zFar = 100.0f;
//	const auto fov = 60.0f;
//	projection_.setToIdentity();
//	projection_.perspective(fov, aspect, zNear, zFar);
}

void Window::change_morphing_param(int state) {
	morphing_param = 100 - state;
	update();
}

void Window::change_camera_speed(int s) {
	cameraSpeed_ = s / static_cast<float>(1000);
	update();
}

void Window::change_directional_light([[maybe_unused]] int state) {
	is_directional = !is_directional;
	update();
}

void Window::change_spot_light([[maybe_unused]] int state) {
	is_spot = !is_spot;
	update();
}

void Window::keyPressEvent(QKeyEvent * got_event) {
	auto key = got_event->key();

	if (key == Qt::Key_W) {
//		cameraPos_.z -= cameraSpeed_;
		cameraPos_ += cameraSpeed_ * cameraFront_; 		// cameraFront --- от камеры к объекту => приближаемся
	} else if (key == Qt::Key_S) {
		cameraPos_ -= cameraSpeed_ * cameraFront_;
	} else if (key == Qt::Key_A) {
		auto norm = glm::normalize(glm::cross(cameraFront_, cameraUp_));
		cameraPos_ -= cameraSpeed_ * norm;
	} else if (key == Qt::Key_D) {
		auto norm = glm::normalize(glm::cross(cameraFront_, cameraUp_));
		cameraPos_ += cameraSpeed_ * norm;
	} else if (key == Qt::Key_V) {
		cameraPos_.y += cameraSpeed_;
	} else if (key == Qt::Key_C) {
		cameraPos_.y -= cameraSpeed_;
	} else {
		return;
	}

	update();
}

void Window::mousePressEvent(QMouseEvent * got_event) {
	mouseStartPos_ = got_event->pos();
	dragged_ = true;
}

void Window::mouseMoveEvent(QMouseEvent * got_event) {
	if (dragged_) {

		auto x_diff = got_event->pos().x() - mouseStartPos_.x();
		auto y_diff = mouseStartPos_.y() - got_event->pos().y();

		yawAngle_ += x_diff * 0.1f;
		pitchAngle_ += y_diff * 0.1f;

		calculate_camera_front();

		mouseStartPos_ = got_event->pos();

		update();
	}
}

void Window::mouseReleaseEvent([[maybe_unused]] QMouseEvent * got_event) {
	dragged_ = false;
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{ std::move(callback) }
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}
	};
}

void Window::calculate_camera_front() {
	cameraFront_.x = glm::cos(glm::radians(yawAngle_)) * glm::cos(glm::radians(pitchAngle_));
	cameraFront_.y = glm::sin(glm::radians(pitchAngle_));
	cameraFront_.z = glm::sin(glm::radians(yawAngle_)) * glm::cos(glm::radians(pitchAngle_));
	cameraFront_ = glm::normalize(cameraFront_);
}


bool Window::loadModel(const char *filename) {
	bool res = loader.LoadBinaryFromFile(&this->model, &err, &warn, filename);
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "ERR: " << err << std::endl;
	}

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;
	else
		std::cout << "Loaded glTF: " << filename << std::endl;

	return res;
}

void Window::bindModelNodes(std::map<int, GLuint>& vbos,
					tinygltf::Node &node) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(vbos, model.meshes[node.mesh]);
	}

	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(vbos, model.nodes[node.children[i]]);
	}
}

std::map<int, GLuint> Window::bindModel() {
	std::map<int, GLuint> vbos;

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(vbos, model.nodes[scene.nodes[i]]);
	}

	return vbos;
}

void Window::bindMesh(std::map<int, GLuint>& vbos, tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];
		if (bufferView.target == 0) {
			continue;
		}

		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

		GLuint vbo;
		glGenBuffers(1, &vbo);
		vbos[i] = vbo;
		glBindBuffer(bufferView.target, vbo);

		glBufferData(bufferView.target, bufferView.byteLength,
					 &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

		// TODO: unbind buffer?
	}

	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
//		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		for (auto &attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);
			// TODO: unbind buffer?

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0) vaa = 0;
			if (attrib.first.compare("NORMAL") == 0) vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
			// --------------------------------------------------------
//			if (attrib.first.compare("TANGENT") == 0) continue;
			// --------------------------------------------------------
			if (vaa > -1) {
				glEnableVertexAttribArray(vaa);
				// 1: номер параметра (номер регистра видеокарты)
				// 2: сколько параметров поступит (у треуг., напр, 3)
				// 3: какого типа будут эти параметры
				// 4: нужно ли normalize применить ко входу (?)
				// 5: где от текущей позиции будет в буффере след значение (значение сдвига)
				// 6: сдвиг от начала буфера: где первое значение
				glVertexAttribPointer(vaa, size, accessor.componentType,
									  accessor.normalized ? GL_TRUE : GL_FALSE,
									  byteStride, BUFFER_OFFSET(accessor.byteOffset));
			} else
				std::cout << "vaa missing: " << attrib.first << std::endl;
		}
	}
	// TODO: bind textures: compile to gltf, add, compile back to glb
}

void Window::drawMesh(tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

		glDrawElements(primitive.mode, indexAccessor.count,
					   indexAccessor.componentType,
					   BUFFER_OFFSET(indexAccessor.byteOffset));
	}
}

// recursively draw node and children nodes of model
void Window::drawModelNodes(tinygltf::Node &node) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(model.meshes[node.mesh]);
	}

	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(model.nodes[node.children[i]]);
	}
}

void Window::drawModel() {

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(model.nodes[scene.nodes[i]]);
	}
}

void Window::display() {
	model_ = glm::mat4(1.0f);
	view_ = glm::mat4(1.0f);
	projection_ = glm::mat4(1.0f);

	glClearColor(0.3, 0.3, 0.3, 1.0);		// background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// calculate model, view, projection separately
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0));
	model_ = rot * model_;
	// glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -4));  // reposition model
	// model_ = trans * model_;

	view_ = glm::lookAt(
		cameraPos_,                 		// camera in world space
		cameraPos_ + cameraFront_,          // target in world space
		cameraUp_  							// y-axis in the world space
	);

	GLint w, h;
	h = this->size().height();
	w = this->size().width();
	projection_ = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.01f, 100.0f);

	// calculate uniforms
	const auto normal_mv = glm::transpose(glm::inverse(view_ * model_));
	auto spot_direction = glm::vec3(0, 1, -2) - spotPosition;

	// set uniforms
	program_->setUniformValue(modelUniform_, QMatrix4x4(glm::value_ptr(model_)).transposed());
	program_->setUniformValue(viewUniform_, QMatrix4x4(glm::value_ptr(view_)).transposed());
	program_->setUniformValue(projectionUniform_, QMatrix4x4(glm::value_ptr(projection_)).transposed());
	program_->setUniformValue(normalTrasform_, QMatrix4x4(glm::value_ptr(normal_mv)).transposed());
	program_->setUniformValue(sunCoord_, QVector3D(3.0, 5.0, 1.0));
	program_->setUniformValue(isDirectionalLightUniform_, is_directional);
	program_->setUniformValue(isSpotLightUniform_, is_spot);
	program_->setUniformValue(morphingParam_, morphing_param);
	program_->setUniformValue(spotPositionUniform_, QVector3D(spotPosition.x, spotPosition.y, spotPosition.z));
	program_->setUniformValue(spotDirection_, QVector3D(spot_direction.x, spot_direction.y, spot_direction.z));
	// program_->setUniformValue(spotAngle_, 20.0);

	drawModel();
}
