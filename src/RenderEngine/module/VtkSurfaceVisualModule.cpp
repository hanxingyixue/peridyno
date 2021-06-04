#include "VtkSurfaceVisualModule.h"

// opengl
#include <glad/glad.h>
#include "RenderEngine.h"

// framework
#include "Topology/TriangleSet.h"
#include "Framework/Node.h"

#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLVertexBufferObjectCache.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkOpenGLIndexBufferObject.h>

#include <cuda_gl_interop.h>

using namespace dyno;

class dyno::SurfaceMapper : public vtkOpenGLPolyDataMapper
{
public:
	SurfaceMapper(SurfaceVisualModule* v): m_module(v)
	{
		// create psedo data
		vtkNew<vtkCylinderSource> cylinder;
		SetInputConnection(cylinder->GetOutputPort());
	}

	void UpdateBufferObjects(vtkRenderer *ren, vtkActor *act) override
	{
		if (!m_module->isInitialized())	return;

		auto node = m_module->getParent();

		if (node == NULL || !node->isVisible())	return;
		
		auto mesh = std::dynamic_pointer_cast<dyno::TriangleSet<dyno::DataType3f>>(node->getTopologyModule());
		auto faces = mesh->getTriangles();
		auto verts = mesh->getPoints();

		cudaError_t error;

		if (!m_initialized)
		{
			printf("Intialize\n");
			m_initialized = true;
								
			// vertex buffer
			vtkNew<vtkPoints>    tempVertData;
			tempVertData->SetNumberOfPoints(verts.size());

			vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
			vtkOpenGLVertexBufferObjectCache* cache = renWin->GetVBOCache();
			this->VBOs->CacheDataArray("vertexMC", tempVertData->GetData(), cache, VTK_FLOAT);
			this->VBOs->BuildAllVBOs(cache);
			m_vbo = this->VBOs->GetVBO("vertexMC");
						
			// index buffer
			this->Primitives[PrimitiveTris].IBO;
			std::vector<unsigned int> indexArray(faces->size() * 3);
			this->Primitives[PrimitiveTris].IBO->Upload(indexArray, vtkOpenGLIndexBufferObject::ElementArrayBuffer);
			this->Primitives[PrimitiveTris].IBO->IndexCount = indexArray.size();
			m_ibo = this->Primitives[PrimitiveTris].IBO;

			// create memory mapper for CUDA
			error = cudaGraphicsGLRegisterBuffer(&m_cudaVBO, m_vbo->GetHandle(), cudaGraphicsRegisterFlagsWriteDiscard);
			//printf("%s\n", cudaGetErrorName(error));
			error = cudaGraphicsGLRegisterBuffer(&m_cudaIBO, m_ibo->GetHandle(), cudaGraphicsRegisterFlagsWriteDiscard);

			// copy index memory, maybe only need once...
			{
				size_t size;
				void*  cudaPtr = 0;
				error = cudaGraphicsMapResources(1, &m_cudaIBO);
				error = cudaGraphicsResourceGetMappedPointer(&cudaPtr, &size, m_cudaIBO);
				error = cudaMemcpy(cudaPtr, faces->begin(), faces->size() * sizeof(unsigned int) * 3, cudaMemcpyDeviceToDevice);
				error = cudaGraphicsUnmapResources(1, &m_cudaIBO);
			}
		}

		// copy vertex memory
		{
			size_t size;
			void*  cudaPtr = 0;

			// upload vertex
			error = cudaGraphicsMapResources(1, &m_cudaVBO);
			//printf("1, %s\n", cudaGetErrorName(error));
			error = cudaGraphicsResourceGetMappedPointer(&cudaPtr, &size, m_cudaVBO);
			error = cudaMemcpy(cudaPtr, verts.begin(), verts.size() * sizeof(float) * 3, cudaMemcpyDeviceToDevice);
			error = cudaGraphicsUnmapResources(1, &m_cudaVBO);
		}
	}

private:
	SurfaceVisualModule* m_module;

	bool			 m_initialized = false;

	vtkOpenGLVertexBufferObject*	m_vbo;
	vtkOpenGLIndexBufferObject*		m_ibo;

	cudaGraphicsResource*			m_cudaVBO;
	cudaGraphicsResource*			m_cudaIBO;

};

IMPLEMENT_CLASS_COMMON(SurfaceVisualModule, 0)

SurfaceVisualModule::SurfaceVisualModule()
{
	this->setName("surface_renderer");
	this->m_mapper = new SurfaceMapper(this);
}

vtkActor* SurfaceVisualModule::createActor()
{
	vtkActor* actor = vtkActor::New();
	actor->SetMapper(m_mapper);
	return actor;
}

void SurfaceVisualModule::updateRenderingContext()
{

}