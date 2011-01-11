/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkLITTPlanCalibrationStep_h
#define __vtkLITTPlanCalibrationStep_h

#include "vtkLITTPlanStep.h"

#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWFrame.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkKWMatrixWidgetWithLabel.h"

#include "vtkSlicerNodeSelectorWidget.h"

class VTK_PROSTATENAV_EXPORT vtkLITTPlanCalibrationStep : public vtkLITTPlanStep
{
public:
  static vtkLITTPlanCalibrationStep *New();
  vtkTypeRevisionMacro(vtkLITTPlanCalibrationStep,vtkLITTPlanStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers();

  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  

  void ShowZFrameModel(bool show);

  // Description:
  // If a file name is specified, the function will import an image from the file
  // to the MRML scene and call Z-Frame calibration code.
  void PerformZFrameCalibration(const char* filename);
  void PerformZFrameCalibration(vtkMRMLScalarVolumeNode* node, int s_index, int e_index);
  
protected:
  vtkLITTPlanCalibrationStep();
  ~vtkLITTPlanCalibrationStep();

  vtkKWFrame       *SelectImageFrame;
  //vtkKWLoadSaveButtonWithLabel *SelectImageButton;
  vtkSlicerNodeSelectorWidget *ZFrameImageSelectorWidget;
  vtkKWMatrixWidgetWithLabel* SliceRangeMatrix;
  vtkKWPushButton  *CalibrateButton;
  vtkKWCheckButton *ShowZFrameCheckButton;
  vtkKWCheckButton *ShowWorkspaceCheckButton;

  vtkKWFrame       *ZFrameSettingFrame;

  //vtkMRMLModelNode* ZFrameModelNode;
  //BTX
  std::string ZFrameModelNodeID;
  std::string ZFrameTransformNodeID;
  //ETX
  

private:
  vtkLITTPlanCalibrationStep(const vtkLITTPlanCalibrationStep&);
  void operator=(const vtkLITTPlanCalibrationStep&);
};

#endif
