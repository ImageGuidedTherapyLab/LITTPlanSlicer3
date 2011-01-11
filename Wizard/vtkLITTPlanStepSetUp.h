/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkLITTPlanStepSetUp_h
#define __vtkLITTPlanStepSetUp_h

#include "vtkLITTPlanStep.h"
#include "vtkCommand.h"

class vtkKWLoadSaveButtonWithLabel;
class vtkKWFrame;
class vtkKWEntry;
class vtkKWCheckButton;
class vtkKWPushButton;
class vtkKWLabel;
class vtkSlicerNodeSelectorWidget;
class vtkMRMLTransPerinealProstateRobotNode;

class VTK_PROSTATENAV_EXPORT vtkLITTPlanStepSetUp :
  public vtkLITTPlanStep
{
public:
  static vtkLITTPlanStepSetUp *New();
  vtkTypeRevisionMacro(vtkLITTPlanStepSetUp,vtkLITTPlanStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  

protected:
  vtkLITTPlanStepSetUp();
  ~vtkLITTPlanStepSetUp();

  vtkMRMLTransPerinealProstateRobotNode* GetRobotNode();

  // GUI Widgets
 
  vtkSlicerNodeSelectorWidget* RobotConnectorSelector;
  vtkSlicerNodeSelectorWidget* ScannerConnectorSelector;

  vtkKWFrame *ConnectorFrame;

private:
  vtkLITTPlanStepSetUp(const vtkLITTPlanStepSetUp&);
  void operator=(const vtkLITTPlanStepSetUp&);
};

#endif
