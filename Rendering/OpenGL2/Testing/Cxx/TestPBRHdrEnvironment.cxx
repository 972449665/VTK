/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestPBRHdrEnvironment.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This test covers the PBR Interpolation shading
// It renders spheres with different materials using a skybox as image based lighting

#include "vtkActor.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkImageReader2.h"
#include "vtkImageReader2Factory.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkNew.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLSkybox.h"
#include "vtkOpenGLTexture.h"
#include "vtkPBRIrradianceTexture.h"
#include "vtkPBRPrefilterTexture.h"
#include "vtkProperty.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRendererCollection.h"
#include "vtkSphereSource.h"
#include "vtkTestUtilities.h"
#include "vtkTexture.h"

//----------------------------------------------------------------------------
int TestPBRHdrEnvironment(int argc, char* argv[])
{
  if (argc <= 1)
  {
    cout << "Usage: " << argv[0] << " <hdr file>" << endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkOpenGLRenderer> renderer;

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(600, 600);
  renWin->AddRenderer(renderer);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  vtkNew<vtkOpenGLSkybox> skybox;

  vtkSmartPointer<vtkPBRIrradianceTexture> irradiance = renderer->GetEnvMapIrradiance();
  irradiance->SetIrradianceStep(0.3);
  vtkSmartPointer<vtkPBRPrefilterTexture> prefilter = renderer->GetEnvMapPrefiltered();
  prefilter->SetPrefilterSamples(64);
  // This size needs to be chosen according to the size of the input texture
  prefilter->SetPrefilterSize(1024);

  auto reader =
    vtkSmartPointer<vtkImageReader2>::Take(vtkImageReader2Factory::CreateImageReader2(argv[1]));
  if (!reader)
  {
    cout << "Error reading file " << argv[1] << std::endl;
    return EXIT_FAILURE;
  }

  reader->SetFileName(argv[1]);
  vtkNew<vtkTexture> texture;
  texture->SetColorModeToDirectScalars();
  texture->MipmapOn();
  texture->InterpolateOn();
  texture->SetInputConnection(reader->GetOutputPort());

  // HDRI OpenGL
  renderer->UseImageBasedLightingOn();
  renderer->SetEnvironmentTexture(texture);

  // Skybox OpenGL
  skybox->SetFloorRight(0.0, 0.0, 1.0);
  skybox->SetProjection(vtkSkybox::Sphere);
  skybox->SetTexture(texture);

  renderer->AddActor(skybox);

  vtkNew<vtkSphereSource> sphere;
  sphere->SetThetaResolution(75);
  sphere->SetPhiResolution(75);

  vtkNew<vtkPolyDataMapper> pdSphere;
  pdSphere->SetInputConnection(sphere->GetOutputPort());

  for (int i = 0; i < 6; i++)
  {
    vtkNew<vtkActor> actorSphere;
    actorSphere->SetPosition(i, 0.0, 0.0);
    actorSphere->SetMapper(pdSphere);
    actorSphere->GetProperty()->SetInterpolationToPBR();
    actorSphere->GetProperty()->SetMetallic(1.0);
    actorSphere->GetProperty()->SetRoughness(i / 5.0);
    renderer->AddActor(actorSphere);
  }

  for (int i = 0; i < 6; i++)
  {
    vtkNew<vtkActor> actorSphere;
    actorSphere->SetPosition(i, 2.0, 0.0);
    actorSphere->SetMapper(pdSphere);
    actorSphere->GetProperty()->SetInterpolationToPBR();
    actorSphere->GetProperty()->SetMetallic(0.0);
    actorSphere->GetProperty()->SetRoughness(i / 5.0);
    renderer->AddActor(actorSphere);
  }

  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
  {
    iren->Start();
  }

  return !retVal;
}
