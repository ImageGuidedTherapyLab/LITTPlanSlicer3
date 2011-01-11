/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkLITTPlanManualControlStep_h
#define __vtkLITTPlanManualControlStep_h

#include "vtkLITTPlanStep.h"

class vtkKWFrame;
class vtkKWScaleWithEntry;

class VTK_PROSTATENAV_EXPORT vtkLITTPlanManualControlStep : public vtkLITTPlanStep
{
public:
  static vtkLITTPlanManualControlStep *New();
  vtkTypeRevisionMacro(vtkLITTPlanManualControlStep,vtkLITTPlanStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkLITTPlanManualControlStep();
  ~vtkLITTPlanManualControlStep();

  // GUI Widgets
  vtkKWFrame *MainFrame;
  vtkKWFrame *ControlFrame;

  vtkKWScaleWithEntry* PRScale;
  vtkKWScaleWithEntry* PAScale;
  vtkKWScaleWithEntry* PSScale;
  vtkKWScaleWithEntry* NRScale;
  vtkKWScaleWithEntry* NAScale;
  vtkKWScaleWithEntry* NSScale;

private:
  vtkLITTPlanManualControlStep(const vtkLITTPlanManualControlStep&);
  void operator=(const vtkLITTPlanManualControlStep&);
};

#endif
