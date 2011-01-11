/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkLITTPlanTargetingStep_h
#define __vtkLITTPlanTargetingStep_h

#include "vtkLITTPlanStep.h"

class vtkKWEntry;
class vtkKWEntrySet;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWLabel;
class vtkSlicerNodeSelectorWidget;
class vtkKWMatrixWidgetWithLabel;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWMultiColumnList;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;
class vtkKWSpinBoxWithLabel;
class vtkKWText;
class vtkImageData;
class vtkMRMLScalarVolumeNode;
class vtkMRMLSelectionNode;
class vtkMRMLFiducialListNode;
class vtkKWCheckButton;

class VTK_PROSTATENAV_EXPORT vtkLITTPlanTargetingStep : public vtkLITTPlanStep
{
public:
  static vtkLITTPlanTargetingStep *New();
  vtkTypeRevisionMacro(vtkLITTPlanTargetingStep,vtkLITTPlanStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void TearDownGUI();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  virtual void UpdateGUI();

  void AddMRMLObservers();
  void RemoveMRMLObservers();

  void OnMultiColumnListUpdate(int row, int col, char * str);
  void OnMultiColumnListSelection();
  void UpdateTargetListGUI();

  void SetShowTargetOrientation(int show);

protected:
  vtkLITTPlanTargetingStep();
  ~vtkLITTPlanTargetingStep();


  void ShowTargetPlanningFrame();
  void ShowTargetListFrame();
  void ShowTargetControlFrame();

  void EnableAddTargetsOnClickButton(bool enable);

  unsigned int PopulateListWithTargetDetails(unsigned int targetDescIndex);

  void AddGUIObservers();
  void RemoveGUIObservers();  
  
  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
    TargetNumberColumn = 0,
    NeedleTypeColumn = 1,
    RASLocationColumn = 2,
    ReachableColumn = 3,
    RotationColumn = 4,
    NeedleAngleColumn = 5,    
    DepthColumn = 6,
    NumberOfColumns = 7,
    };
    
  //ETX

  bool ProcessingCallback;

  vtkKWFrame *MainFrame;
  
  // TargetPlanning
  vtkKWFrame *TargetPlanningFrame;
  vtkKWPushButton* LoadTargetingVolumeButton;
  vtkSlicerNodeSelectorWidget* VolumeSelectorWidget;
  vtkKWFrame *LoadVolumeDialogFrame;
  vtkKWCheckButton *ShowWorkspaceButton;  
  vtkKWCheckButton *ShowRobotButton;  
  vtkKWCheckButton *AddTargetsOnClickButton;
  vtkKWMenuButtonWithLabel *NeedleTypeMenuList;  
  vtkMRMLFiducialListNode *TargetPlanListNode;

  // TargetList frame
  vtkKWFrame *TargetListFrame;
  vtkKWMultiColumnListWithScrollbars* TargetList;
  vtkKWPushButton *DeleteButton;

  // TargetControl frame
  vtkKWFrame *TargetControlFrame;
  vtkKWMatrixWidgetWithLabel* NeedlePositionMatrix;
  vtkKWMatrixWidgetWithLabel* NeedleOrientationMatrix;
  vtkKWPushButton *MoveButton;
  vtkKWPushButton *StopButton;

  vtkKWText *Message;

  vtkKWSpinBoxWithLabel *PerfusionValueBox;
  vtkKWSpinBoxWithLabel *ThermalConductivityValueBox;
  vtkKWSpinBoxWithLabel *OpticalAbsorptionValueBox;
  vtkKWSpinBoxWithLabel *OpticalScatteringValueBox;

  int ShowTargetOrientation;

private:
  vtkLITTPlanTargetingStep(const vtkLITTPlanTargetingStep&);
  void operator=(const vtkLITTPlanTargetingStep&);  
};

#endif
