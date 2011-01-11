/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkLITTPlanStep_h
#define __vtkLITTPlanStep_h

#include "vtkKWWizardStep.h"
#include "vtkLITTPlan.h"
#include "vtkCommand.h"

#include "vtkObserverManager.h"
#include "vtkMRMLLITTPlanManagerNode.h"

class vtkLITTPlanGUI;
class vtkLITTPlanLogic;
class vtkMRMLScene;

class VTK_PROSTATENAV_EXPORT vtkLITTPlanStep : public vtkKWWizardStep
{
public:
  static vtkLITTPlanStep *New();
  vtkTypeRevisionMacro(vtkLITTPlanStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Register(vtkObject *o) { Superclass::Register(o); };
  virtual void UnRegister(vtkObjectBase *o) { Superclass::UnRegister(o); };

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkLITTPlanGUI);
  vtkGetObjectMacro(Logic, vtkLITTPlanLogic);

  virtual void SetGUI(vtkLITTPlanGUI*);
  virtual void SetLogic(vtkLITTPlanLogic*);
  virtual void TearDownGUI();

  vtkSetObjectMacro(LITTPlanManager, vtkMRMLLITTPlanManagerNode);
  vtkGetObjectMacro(LITTPlanManager, vtkMRMLLITTPlanManagerNode);

  vtkSetMacro(TotalSteps, int);
  vtkGetMacro(TotalSteps, int);
  vtkSetMacro(StepNumber, int);
  vtkGetMacro(StepNumber, int);

  void SetTitle(const char* title) {
    this->Title = title;
  }

  const char* GetTitle() {
    return this->Title.c_str();
  }

  void UpdateName();


  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInGUICallbackFlag (int flag) {
    this->InGUICallbackFlag = flag;
    }
  vtkGetMacro(InGUICallbackFlag, int);

  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkMRMLScene *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), (vtkObject*)mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLSceneEvents ( vtkMRMLScene *mrml, vtkIntArray *events )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &this->MRMLScene), mrml, events );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }


  void SetTitleBackgroundColor (double r, double g, double b) {
    this->TitleBackgroundColor[0] = r;
    this->TitleBackgroundColor[1] = g;
    this->TitleBackgroundColor[2] = b;
  };

  void GetTitleBackgroundColor (double* r, double* g, double* b) {
    *r = this->TitleBackgroundColor[0];
    *g = this->TitleBackgroundColor[1];
    *b = this->TitleBackgroundColor[2];
  };

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();
  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {};
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData) {};

  virtual void UpdateGUI() {};

  void ShowWorkspaceModel(bool show);
  bool IsWorkspaceModelShown();
  void ShowRobotModel(bool show);
  bool IsRobotModelShown();

protected:
  vtkLITTPlanStep();
  ~vtkLITTPlanStep();

  static void GUICallback(vtkObject *caller,
                          unsigned long eid, void *clientData, void *callData );

  static void MRMLCallback(vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData );

protected:
  
  double TitleBackgroundColor[3];

  int InGUICallbackFlag;
  int InMRMLCallbackFlag;

  vtkLITTPlanGUI   *GUI;
  vtkLITTPlanLogic *Logic;
  vtkMRMLScene        *MRMLScene;

  vtkCallbackCommand *GUICallbackCommand;
  vtkCallbackCommand *MRMLCallbackCommand;
  vtkObserverManager *MRMLObserverManager;

  vtkMRMLLITTPlanManagerNode* LITTPlanManager;

  //BTX
  std::string Title;
  //ETX
  
  int TotalSteps;     // Total number of steps in the wizard
  int StepNumber;     // Step number for this step.

private:
  vtkLITTPlanStep(const vtkLITTPlanStep&);
  void operator=(const vtkLITTPlanStep&);

};

#endif
