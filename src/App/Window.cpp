
#include <QMouseEvent>
#include <QLabel>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVBoxLayout>
#include <QScreen>
#include <QtMath>

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
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto layout = new QVBoxLayout();
	layout->addWidget(fps, 1);

	setLayout(layout);

	timer_.start();

	connect(this, &Window::updateUI, [=] {
		fps->setText(formatFPS(ui_.fps));
	});
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

	loadModel("Models/Cube.glb");
	vaoAndEbos = bindModel();

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

	mvpUniform_ = program_->uniformLocation("MVP");
//	sun_position_ = program_->uniformLocation("sun_position");
//	sun_color_ = program_->uniformLocation("sun_color");

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
	cameraPos_ = glm::vec3(0, -2, 3);
	cameraFront_ = glm::vec3(0, 0, -3);
	yawAngle_ = -90.;
	pitchAngle_ = 0.;
	first_touch = true;
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate MVP matrix
	model_ = glm::mat4(1.0f);
	view_ = glm::mat4(1.0f);
	projection_ = glm::mat4(1.0f);
//	const auto mvp = projection_ * view_ * model_;

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform value
//	program_->setUniformValue(mvpUniform_, mvp);

	// Activate texture unit and bind texture
	glActiveTexture(GL_TEXTURE0);
	texture_->bind();

	// Draw
	displayLoop();

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

		std::cout << yawAngle_ << " " << pitchAngle_ << std::endl;
//		if(pitchAngle_ > 89.0f)
//			pitchAngle_ = 89.0f;
//		if(pitchAngle_ < -89.0f)
//			pitchAngle_ = -89.0f;

		cameraFront_.x = glm::cos(glm::radians(yawAngle_)) * glm::cos(glm::radians(pitchAngle_));
		cameraFront_.y = glm::sin(glm::radians(pitchAngle_));
		cameraFront_.z = glm::sin(glm::radians(yawAngle_)) * glm::cos(glm::radians(pitchAngle_));

		cameraFront_ = glm::normalize(cameraFront_);

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

std::pair<GLuint, std::map<int, GLuint>> Window::bindModel() {
	std::map<int, GLuint> vbos;
	vao_.bind();

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(vbos, model.nodes[scene.nodes[i]]);
	}

	vao_.release();
	// cleanup vbos but do not delete index buffers yet
	for (auto it = vbos.cbegin(); it != vbos.cend();) {
		// ----------------------------------------
		if (it->first < 0)
			continue;
		// ---------------------------------------
		tinygltf::BufferView bufferView = model.bufferViews[it->first];
		if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {		// TODO: target was not set in bindMesh
			glDeleteBuffers(1, &vbos[it->first]);
			vbos.erase(it++);
		}
		else {
			++it;
		}
	}

	return {vao_.objectId(), vbos};
}

void Window::bindMesh(std::map<int, GLuint>& vbos, tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];
		if (bufferView.target == 0) {
//			std::cout << "WARN: bufferView.target is zero" << std::endl;
			continue;  // Unsupported bufferView.
					 /*
                   From spec2.0 readme:
                   https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
                            ... drawArrays function should be used with a count equal to
                   the count            property of any of the accessors referenced by the
                   attributes            property            (they are all equal for a given
                   primitive).
                 */
		}

		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

		GLuint vbo;
		glGenBuffers(1, &vbo);
		vbos[i] = vbo;
		glBindBuffer(bufferView.target, vbo);

		glBufferData(bufferView.target, bufferView.byteLength,
					 &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
	}

	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		for (auto &attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

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
				glVertexAttribPointer(vaa, size, accessor.componentType,
									  accessor.normalized ? GL_TRUE : GL_FALSE,
									  byteStride, BUFFER_OFFSET(accessor.byteOffset));
			} else
				std::cout << "vaa missing: " << attrib.first << std::endl;
		}
	}
}

void Window::drawMesh(tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vaoAndEbos.second.at(indexAccessor.bufferView));

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
	vao_.bind();

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(model.nodes[scene.nodes[i]]);
	}

	vao_.release();
}

void Window::displayLoop() {
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 model_rot = glm::mat4(1.0f);
//	glm::vec3 model_pos = glm::vec3(0, 0, -3);

	glClearColor(0.2, 0.2, 0.2, 1.0);		// background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	glm::mat4 trans = glm::translate(glm::mat4(1.0f), cameraFront_);  // reposition model
//	model_ = trans * model_;
	model_rot = glm::rotate(model_rot, glm::radians(0.8f), glm::vec3(0, 1, 0));  // rotate model on y axis
	model_ = model_ * model_rot;

	view_ = glm::lookAt(
		cameraPos_,                 // Camera in World Space
		cameraPos_ + cameraFront_,             		// and looks at the origin
		glm::vec3(0, 1, 0)  		// Head is up (set to 0,-1,0 to look upside-down)
	);

	// build a model-view-projection
	GLint w, h;
	h = this->size().height();
	w = this->size().width();
	projection_ = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.01f, 100.0f);

	const auto mvp = projection_ * view_ * model_;
	program_->setUniformValue(mvpUniform_, QMatrix4x4(glm::value_ptr(mvp)).transposed());

	drawModel();
}
