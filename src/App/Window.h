#pragma once

#include <Base/GLWidget.hpp>

#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include <functional>
#include <memory>
// ------------------------------
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tinygltf/tiny_gltf.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class Window final : public fgl::GLWidget
{
	Q_OBJECT
public:
	Window() noexcept;
	~Window() override;

public: // fgl::GLWidget
	void onInit() override;
	void onRender() override;
	void onResize(size_t width, size_t height) override;
	void mousePressEvent(QMouseEvent* got_event) override;
	void mouseMoveEvent(QMouseEvent* got_event) override;
	void mouseReleaseEvent(QMouseEvent* got_event) override;
	void keyPressEvent(QKeyEvent* got_event) override;

private:
	class PerfomanceMetricsGuard final
	{
	public:
		explicit PerfomanceMetricsGuard(std::function<void()> callback);
		~PerfomanceMetricsGuard();

		PerfomanceMetricsGuard(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard(PerfomanceMetricsGuard &&) = delete;

		PerfomanceMetricsGuard & operator=(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard & operator=(PerfomanceMetricsGuard &&) = delete;

	private:
		std::function<void()> callback_;
	};

private:
	[[nodiscard]] PerfomanceMetricsGuard captureMetrics();

signals:
	void updateUI();

private:
	// uniforms
	GLint modelUniform_ = -1;
	GLint viewProjUniform_ = -1;
	GLint sunCoord_ = -1;
	GLint normalTrasform_ = -1;
	GLint directionalLightUniform_ = -1;

	// buffers
	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;

	// mvp parameters
	glm::mat4 model_;
	glm::mat4 view_;
	glm::mat4 projection_;

	std::unique_ptr<QOpenGLTexture> texture_;
	std::unique_ptr<QOpenGLShaderProgram> program_;

	QElapsedTimer timer_;
	size_t frameCount_ = 0;

	struct {
		size_t fps = 0;
	} ui_;

	bool animated_ = false;

	// mouse control params
	bool dragged_ = false;
	QPoint mouseStartPos_;

	// widget reacting
	void change_camera_speed(int s);
	void change_directional_light(int state);

	// camera coord parameters
	glm::vec3 cameraPos_;
	glm::vec3 cameraFront_;
	glm::vec3 cameraUp_;

	// free movements params
	float cameraSpeed_;
	float yawAngle_;
	float pitchAngle_;

	// light params
	bool directional;

	// model managing
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	std::map<int, GLuint> vbos;

	void display();
	void drawModel();
	void drawModelNodes(tinygltf::Node &node);
	void drawMesh(tinygltf::Mesh &mesh);
	void bindMesh(std::map<int, GLuint>& vbos, tinygltf::Mesh &mesh);
	std::map<int, GLuint> bindModel();
	void bindModelNodes(std::map<int, GLuint>& vbos, tinygltf::Node &node);
	bool loadModel(const char *filename);
};
