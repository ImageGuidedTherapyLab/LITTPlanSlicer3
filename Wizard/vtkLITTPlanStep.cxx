/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkLITTPlanStep.h"
#include "vtkLITTPlanGUI.h"
#include "vtkLITTPlanLogic.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkObserverManager.h"
#include "vtkMRMLFiducialListNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLITTPlanStep);
vtkCxxRevisionMacro(vtkLITTPlanStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkLITTPlanStep,GUI,vtkLITTPlanGUI);
vtkCxxSetObjectMacro(vtkLITTPlanStep,Logic,vtkLITTPlanLogic);

//----------------------------------------------------------------------------
vtkLITTPlanStep::vtkLITTPlanStep()
{

  std::cerr << "vtkLITTPlanStep::vtkLITTPlanStep() start" << std::endl;
  this->GUI = NULL;
  this->Logic = NULL;
  this->MRMLScene = NULL;
  this->LITTPlanManager = NULL;

  this->GUICallbackCommand = vtkCallbackCommand::New();
  this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->GUICallbackCommand->SetCallback(&vtkLITTPlanStep::GUICallback);

  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkLITTPlanStep::MRMLCallback);
  this->MRMLCallbackCommand = this->MRMLObserverManager->GetCallbackCommand();

  this->TitleBackgroundColor[0] = 0.8;
  this->TitleBackgroundColor[1] = 0.8;
  this->TitleBackgroundColor[2] = 1.0;

  this->InGUICallbackFlag = 0;
  this->InMRMLCallbackFlag = 0;

  std::cerr << "vtkLITTPlanStep::vtkLITTPlanStep() end" << std::endl;
}

//----------------------------------------------------------------------------
vtkLITTPlanStep::~vtkLITTPlanStep()
{
  this->SetAndObserveMRMLScene ( NULL );

  if (this->MRMLObserverManager)
    {
    this->MRMLObserverManager->RemoveAllObservers();
    this->MRMLObserverManager->Delete();
    }    

  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->Delete ( );
    this->GUICallbackCommand = NULL;
    }

  this->SetGUI(NULL);
  this->SetLogic(NULL);

}

//----------------------------------------------------------------------------
void vtkLITTPlanStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  this->SetAndObserveMRMLScene(NULL);

  if (this->GetGUI())
    {
    this->GetGUI()->GetWizardWidget()->ClearPage();
    }
}

//----------------------------------------------------------------------------
void vtkLITTPlanStep::Validate()
{
  this->Superclass::Validate();

  vtkKWWizardWorkflow *wizardWorkflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  wizardWorkflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
  wizardWorkflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkLITTPlanStep::CanGoToSelf()
{
  return this->Superclass::CanGoToSelf() || 1;
}

//----------------------------------------------------------------------------
void vtkLITTPlanStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkLITTPlanStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  
  if (!this->MRMLScene)
    {
    this->SetAndObserveMRMLScene (this->GetGUI()->GetMRMLScene());
    }

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  wizardWidget->GetCancelButton()->SetEnabled(0);
  wizardWidget->SetTitleAreaBackgroundColor(this->TitleBackgroundColor[0],
                                            this->TitleBackgroundColor[1],
                                            this->TitleBackgroundColor[2]);

}


//----------------------------------------------------------------------------
void vtkLITTPlanStep::GUICallback( vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData )
{

  vtkLITTPlanStep *self = reinterpret_cast<vtkLITTPlanStep *>(clientData);
  
  if (self->GetInGUICallbackFlag())
    {
    }

  vtkDebugWithObjectMacro(self, "In vtkLITTPlanStep GUICallback");
  
  self->SetInGUICallbackFlag(1);
  self->ProcessGUIEvents(caller, eid, callData);
  self->SetInGUICallbackFlag(0);
  
}


//----------------------------------------------------------------------------
void vtkLITTPlanStep::MRMLCallback(vtkObject *caller, 
                                    unsigned long eid, void *clientData, void *callData)
{

  vtkLITTPlanStep *self = reinterpret_cast<vtkLITTPlanStep *>(clientData);
  
  if (self->GetInMRMLCallbackFlag())
    {
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkLITTPlanStep MRMLCallback");
  
  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
}


//----------------------------------------------------------------------------
void vtkLITTPlanStep::UpdateName()
{
  std::stringstream ss;
  ss << this->StepNumber << " / " << this->TotalSteps << ". " << this->Title;
  this->SetName(ss.str().c_str());
  this->Modified();
}

void vtkLITTPlanStep::TearDownGUI()
{
  // Override in child classes  
}

//----------------------------------------------------------------------------
void vtkLITTPlanStep::ShowWorkspaceModel(bool show)
{
  vtkLITTPlanLogic *logic=this->GetGUI()->GetLogic();
  if (!logic)
  {
    vtkErrorMacro("Invalid logic object");
    return;
  }
  logic->ShowWorkspaceModel(show);
}

//----------------------------------------------------------------------------
bool vtkLITTPlanStep::IsWorkspaceModelShown()
{
  vtkLITTPlanLogic *logic=this->GetGUI()->GetLogic();
  if (!logic)
  {
    vtkErrorMacro("Invalid logic object");
    return false;
  }
  return logic->IsWorkspaceModelShown();
}

//----------------------------------------------------------------------------
void vtkLITTPlanStep::ShowRobotModel(bool show)
{
  vtkLITTPlanLogic *logic=this->GetGUI()->GetLogic();
  if (!logic)
  {
    vtkErrorMacro("Invalid logic object");
    return;
  }
  logic->ShowRobotModel(show);
}

//----------------------------------------------------------------------------
bool vtkLITTPlanStep::IsRobotModelShown()
{
  vtkLITTPlanLogic *logic=this->GetGUI()->GetLogic();
  if (!logic)
  {
    vtkErrorMacro("Invalid logic object");
    return false;
  }
  return logic->IsRobotModelShown();
}
