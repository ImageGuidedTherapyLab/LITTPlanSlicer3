/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkLITTPlanStepSetUpTemplate_h
#define __vtkLITTPlanStepSetUpTemplate_h

#include "vtkLITTPlanStep.h"
#include "vtkCommand.h"

class vtkKWLoadSaveButtonWithLabel;
class vtkKWFrame;
class vtkKWEntry;
class vtkKWCheckButton;
class vtkKWPushButton;
class vtkKWLabel;
class vtkSlicerNodeSelectorWidget;

class VTK_PROSTATENAV_EXPORT vtkLITTPlanStepSetUpTemplate :
  public vtkLITTPlanStep
{
public:
  static vtkLITTPlanStepSetUpTemplate *New();
  vtkTypeRevisionMacro(vtkLITTPlanStepSetUpTemplate,vtkLITTPlanStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  
  
protected:
  vtkLITTPlanStepSetUpTemplate();
  ~vtkLITTPlanStepSetUpTemplate();

  // GUI Widgets
 
  vtkSlicerNodeSelectorWidget* ScannerConnectorSelector;

  vtkKWFrame *ConnectorFrame;

private:
  vtkLITTPlanStepSetUpTemplate(const vtkLITTPlanStepSetUpTemplate&);
  void operator=(const vtkLITTPlanStepSetUpTemplate&);
};

#endif
