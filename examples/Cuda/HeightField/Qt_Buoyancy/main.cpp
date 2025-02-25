/**
 * Copyright 2022 Xiaowei He
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <QtApp.h>

#include <SceneGraph.h>

#include <HeightField/Ocean.h>
#include <HeightField/OceanPatch.h>
#include <HeightField/Coupling.h>

#include <HeightField/Vessel.h>

#include <HeightField/initializeHeightField.h>

#include "Module/ComputeModule.h"

#include "Mapping/HeightFieldToTriangleSet.h"

#include <GLRenderEngine.h>
#include <GLSurfaceVisualModule.h>

using namespace std;
using namespace dyno;

/**
 * @brief An example to demonstrate the coupling between a boat and the ocean
 */

std::shared_ptr<SceneGraph> createScene()
{
	std::shared_ptr<SceneGraph> scn = std::make_shared<SceneGraph>();

	auto ocean = scn->addNode(std::make_shared<Ocean<DataType3f>>());

	auto patch = scn->addNode(std::make_shared<OceanPatch<DataType3f>>());
	patch->varWindType()->setValue(3);
	patch->connect(ocean->importOceanPatch());

	auto mapper = std::make_shared<HeightFieldToTriangleSet<DataType3f>>();

	ocean->stateHeightField()->connect(mapper->inHeightField());
	ocean->graphicsPipeline()->pushModule(mapper);


	auto sRender = std::make_shared<GLSurfaceVisualModule>();
	sRender->setColor(Color(0.0f, 0.2f, 1.0f));
	sRender->varUseVertexNormal()->setValue(true);
	sRender->varAlpha()->setValue(0.6);
	mapper->outTriangleSet()->connect(sRender->inTriangleSet());
	ocean->graphicsPipeline()->pushModule(sRender);

	auto boat = scn->addNode(std::make_shared<Vessel<DataType3f>>());
	boat->varDensity()->setValue(150.0f);
	boat->stateVelocity()->setValue(Vec3f(0, 0, 0));
// 	boat->varEnvelopeName()->setValue(getAssetPath() + "obj/boat_boundary.obj");
// 	boat->varMeshName()->setValue(getAssetPath() + "obj/boat_mesh.obj");

	auto coupling = scn->addNode(std::make_shared<Coupling<DataType3f>>());
	boat->connect(coupling->importVessels());
	ocean->connect(coupling->importOcean());
	
	return scn;
}

int main()
{
	HeightFieldLibrary::initStaticPlugin();

	QtApp app;

	app.setSceneGraph(createScene());

	app.initialize(1024, 768);
	
	//Set the distance unit for the camera, the fault unit is meter
	app.renderWindow()->getCamera()->setUnitScale(10.0);

	app.mainLoop();

	return 0;
}
