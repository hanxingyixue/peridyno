#include "GLPointVisualModule.h"
#include "GLRenderEngine.h"

#include <Utility.h>

// opengl
#include <glad/glad.h>

#ifdef CUDA_BACKEND
// cuda
#include <cuda_gl_interop.h>
#endif

#ifdef VK_BACKEND

#endif // VK_BACKEND


namespace dyno
{
	IMPLEMENT_CLASS(GLPointVisualModule)

	GLPointVisualModule::GLPointVisualModule()
	{
		mNumPoints = 0;
		this->setName("point_renderer");
		this->inColor()->tagOptional(true);

		this->varPointSize()->setRange(0.001f, 1.0f);


		this->varForceUpdate()->setValue(true);
	}

	GLPointVisualModule::~GLPointVisualModule()
	{
// 		points.clear();
// 		colors.clear();
// 
// 		mPosition.release();
// 		mColor.release();
	}

	void GLPointVisualModule::setColorMapMode(ColorMapMode mode)
	{
		this->varColorMode()->getDataPtr()->setCurrentKey(mode);
	}

	bool GLPointVisualModule::initializeGL()
	{
		uint vecSize = sizeof(Vec3f) / sizeof(float);

		mPosition.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
		mColor.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

		mVertexArray.create();
		mVertexArray.bindVertexBuffer(&mPosition, 0, vecSize, GL_FLOAT, 0, 0, 0);
		mVertexArray.bindVertexBuffer(&mColor, 1, vecSize, GL_FLOAT, 0, 0, 0);

		mShaderProgram = gl::ShaderFactory::createShaderProgram("point.vert", "point.frag");

		gl::glCheckError();

		return true;
	}

	void GLPointVisualModule::destroyGL()
	{
		if (isGLInitialized)
		{
			mShaderProgram->release();
			delete mShaderProgram;

			mPosition.release();
			mColor.release();
			mVertexArray.release();

			isGLInitialized = false;
		}
	}

	void GLPointVisualModule::updateGL()
	{
		updateMutex.lock();

		uint vecSize = sizeof(Vec3f) / sizeof(float);

		mPosition.mapGL();
		mVertexArray.bindVertexBuffer(&mPosition, 0, vecSize, GL_FLOAT, 0, 0, 0);

		if (this->varColorMode()->getDataPtr()->currentKey() == ColorMapMode::PER_VERTEX_SHADER
			&& !this->inColor()->isEmpty())
		{
			mColor.mapGL();
			mVertexArray.bindVertexBuffer(&mColor, 1, vecSize, GL_FLOAT, 0, 0, 0);
		}
		else
		{
			mVertexArray.bind();
			glDisableVertexAttribArray(1);
			mVertexArray.unbind();
		}

		updateMutex.unlock();
	}

	void GLPointVisualModule::updateGraphicsContext()
	{
		updateMutex.lock();
		// update data
		auto pPointSet = this->inPointSet()->getDataPtr();
		auto points = pPointSet->getPoints();

		mNumPoints = points.size();

		if (mNumPoints > 0)
		{
			mPosition.load(points);

			if (this->varColorMode()->getDataPtr()->currentKey() == ColorMapMode::PER_VERTEX_SHADER
				&& !this->inColor()->isEmpty())
			{
				auto colors = this->inColor()->getData();
				mColor.load(colors);
			}

			GLVisualModule::updateGraphicsContext();
		}

		updateMutex.unlock();
	}

	void GLPointVisualModule::paintGL(GLRenderPass pass)
	{
		if (mNumPoints == 0)
			return;

		mShaderProgram->use();
		mShaderProgram->setFloat("uPointSize", this->varPointSize()->getData());

		unsigned int subroutine;
		if (pass == GLRenderPass::COLOR)
		{
			mShaderProgram->setFloat("uMetallic", this->varMetallic()->getData());
			mShaderProgram->setFloat("uRoughness", this->varRoughness()->getData());
			mShaderProgram->setFloat("uAlpha", this->varAlpha()->getData());

			subroutine = 0;
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutine);
		}
		else if (pass == GLRenderPass::SHADOW)
		{
			subroutine = 1;
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutine);
		}
		else if (pass == GLRenderPass::TRANSPARENCY)
		{
			printf("WARNING: GLPointVisualModule does not support transparency!\n");
			return;
		}
		else
		{
			printf("Unknown render pass!\n");
			return;
		}

		// per-object color color
		auto color = this->varBaseColor()->getData();
		glVertexAttrib3f(1, color.r, color.g, color.b);

		mVertexArray.bind();
		glDrawArrays(GL_POINTS, 0, mNumPoints);
		gl::glCheckError();
	}
}
