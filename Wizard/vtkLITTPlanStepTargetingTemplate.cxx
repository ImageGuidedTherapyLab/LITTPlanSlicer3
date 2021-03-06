/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkLITTPlanStepTargetingTemplate.h"

#include "vtkLITTPlanGUI.h"
#include "vtkLITTPlanLogic.h"

#include "vtkKWMatrixWidget.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlMath.h"

#include "vtkMRMLRobotNode.h"

////

#include "vtkLITTPlanTargetDescriptor.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkMRMLInteractionNode.h"

#include "vtkKWFrame.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrame.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntrySet.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWText.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLTransPerinealProstateTemplateNode.h"

const char TARGET_INDEX_ATTR[]="TARGET_IND";

#define DELETE_IF_NULL_WITH_SETPARENT_NULL(obj) \
  if (obj) \
    { \
    obj->SetParent(NULL); \
    obj->Delete(); \
    obj = NULL; \
    };

// Definition of target list columns
enum
{
  COL_NAME = 0,
  COL_X,
  COL_Y,
  COL_Z,
  COL_NEEDLE,
  COL_OR_W,
  COL_OR_X,
  COL_OR_Y,
  COL_OR_Z,
  COL_COUNT // all valid columns should be inserted above this line
};
static const char* COL_LABELS[COL_COUNT] = { "Name", "R", "A", "S", "Needle", "OrW", "OrX", "OrY", "OrZ" };
static const int COL_WIDTHS[COL_COUNT] = { 8, 6, 6, 6, 8, 6, 6, 6, 6 };


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLITTPlanStepTargetingTemplate);
vtkCxxRevisionMacro(vtkLITTPlanStepTargetingTemplate, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkLITTPlanStepTargetingTemplate::vtkLITTPlanStepTargetingTemplate()
{
  //this->SetName("Targeting");
  this->SetTitle("Targeting");
  this->SetDescription("Set target points.");

  this->MainFrame=NULL;
  
  // TargetPlanning frame
  this->TargetPlanningFrame=NULL;
  //this->LoadTargetingVolumeButton=NULL;
  this->VolumeSelectorWidget=NULL;
  this->TargetListSelectorWidget=NULL;
  this->TargetPlanningFrame=NULL;
  this->ShowWorkspaceButton=NULL;
  this->ShowNeedleButton=NULL;
  this->ShowTemplateButton=NULL;
  this->AddTargetsOnClickButton=NULL;
  //this->NeedleTypeMenuList=NULL; 

  // TargetList frame
  this->TargetListFrame=NULL;
  this->TargetList=NULL;
  this->DeleteButton=NULL;

  // TargetControl frame
  this->TargetControlFrame=NULL;
  this->NeedlePositionMatrix=NULL;
  //this->NeedleOrientationMatrix=NULL;
  this->MoveButton=NULL;
  this->StopButton=NULL;

  this->Message=NULL;

  this->TitleBackgroundColor[0] = 0.8;
  this->TitleBackgroundColor[1] = 0.8;
  this->TitleBackgroundColor[2] = 0.8;

  this->ProcessingCallback = false;

  this->TargetPlanListNode=NULL;

  this->OptionFrame = NULL;
}

//----------------------------------------------------------------------------
vtkLITTPlanStepTargetingTemplate::~vtkLITTPlanStepTargetingTemplate()
{
  RemoveGUIObservers();

  DELETE_IF_NULL_WITH_SETPARENT_NULL(MainFrame);
  
  // TargetPlanning
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetPlanningFrame);
  //DELETE_IF_NULL_WITH_SETPARENT_NULL(LoadTargetingVolumeButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(VolumeSelectorWidget);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetListSelectorWidget);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetPlanningFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(ShowWorkspaceButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(ShowNeedleButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(ShowTemplateButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(AddTargetsOnClickButton);
  //DELETE_IF_NULL_WITH_SETPARENT_NULL(NeedleTypeMenuList); 
  DELETE_IF_NULL_WITH_SETPARENT_NULL(OptionFrame);

  // TargetList frame
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetListFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetList);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(DeleteButton);

  // TargetControl frame
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetControlFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(NeedlePositionMatrix);
  //DELETE_IF_NULL_WITH_SETPARENT_NULL(NeedleOrientationMatrix);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(MoveButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(StopButton);

  DELETE_IF_NULL_WITH_SETPARENT_NULL(Message);
}

//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  this->ShowTargetPlanningFrame();
  this->ShowTargetListFrame();
  this->ShowTargetControlFrame();

  AddMRMLObservers();
  
  this->AddGUIObservers();

  EnableAddTargetsOnClickButton(this->AddTargetsOnClickButton->GetSelectedState()==1);
}

//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ShowTargetPlanningFrame()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();

  if (!this->TargetPlanningFrame)
    {
    this->TargetPlanningFrame = vtkKWFrame::New();
    }
  if (!this->TargetPlanningFrame->IsCreated())
    {
    this->TargetPlanningFrame->SetParent(parent);
    this->TargetPlanningFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2",
               this->TargetPlanningFrame->GetWidgetName());
  
  //if (!this->LoadTargetingVolumeButton)
  //  {
  //   this->LoadTargetingVolumeButton = vtkKWPushButton::New();
  //  }
  //if (!this->LoadTargetingVolumeButton->IsCreated())
  //  {
  //  this->LoadTargetingVolumeButton->SetParent(this->TargetPlanningFrame);
  //  this->LoadTargetingVolumeButton->Create();
  //  this->LoadTargetingVolumeButton->SetBorderWidth(2);
  //  this->LoadTargetingVolumeButton->SetReliefToRaised();       
  //  this->LoadTargetingVolumeButton->SetHighlightThickness(2);
  //  this->LoadTargetingVolumeButton->SetBackgroundColor(0.85,0.85,0.85);
  //  this->LoadTargetingVolumeButton->SetActiveBackgroundColor(1,1,1);        
  //  this->LoadTargetingVolumeButton->SetText( "Load volume");
  //  this->LoadTargetingVolumeButton->SetBalloonHelpString("Click to load a volume. Need to additionally select the volume to make it the current targeting volume.");
  //  }

  if (!this->VolumeSelectorWidget)
    {
     this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->VolumeSelectorWidget->IsCreated())
    {
    this->VolumeSelectorWidget->SetParent(this->TargetPlanningFrame);
    this->VolumeSelectorWidget->Create();
    this->VolumeSelectorWidget->SetBorderWidth(2);  
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->VolumeSelectorWidget->SetMRMLScene(this->GetLogic()->GetApplicationLogic()->GetMRMLScene());
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->VolumeSelectorWidget->SetLabelText( "Targeting Volume: ");
    this->VolumeSelectorWidget->SetBalloonHelpString("Select the targeting volume from the current scene.");
    }

  if (!this->TargetListSelectorWidget)
    {
    this->TargetListSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->TargetListSelectorWidget->IsCreated())
    {
    this->TargetListSelectorWidget->SetParent(this->TargetPlanningFrame);
    this->TargetListSelectorWidget->Create();
    this->TargetListSelectorWidget->SetBorderWidth(2);  
    this->TargetListSelectorWidget->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
    this->TargetListSelectorWidget->SetMRMLScene(this->GetLogic()->GetApplicationLogic()->GetMRMLScene());
    this->TargetListSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TargetListSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->TargetListSelectorWidget->SetLabelText( "Target List: ");
    this->TargetListSelectorWidget->SetBalloonHelpString("Select the fiducial list from the current scene.");
    }

  //if (!this->NeedleTypeMenuList)
  //  {
  //  this->NeedleTypeMenuList = vtkKWMenuButtonWithLabel::New();
  //  }
  //if (!this->NeedleTypeMenuList->IsCreated())
  //  {
  //  this->NeedleTypeMenuList->SetParent(this->TargetPlanningFrame);
  //  this->NeedleTypeMenuList->Create();
  //  this->NeedleTypeMenuList->SetLabelText("Needle type: ");
  //  this->NeedleTypeMenuList->SetBalloonHelpString("Select the needle type");
  //  }

  this->Script("pack %s %s -side top -anchor nw -expand y -padx 2 -pady 2",
               this->VolumeSelectorWidget->GetWidgetName(),
               this->TargetListSelectorWidget->GetWidgetName());
               //this->NeedleTypeMenuList->GetWidgetName());

  if (!this->OptionFrame)
    {
    this->OptionFrame = vtkKWFrame::New();
    }
  if (!this->OptionFrame->IsCreated())
    {
    this->OptionFrame->SetParent(this->TargetPlanningFrame);
    this->OptionFrame->Create();
    }
  
  this->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 2",
               this->OptionFrame->GetWidgetName());

  if (!this->ShowWorkspaceButton)
  {
    this->ShowWorkspaceButton = vtkKWCheckButton::New();
  } 
  if (!this->ShowWorkspaceButton->IsCreated()) {
    this->ShowWorkspaceButton->SetParent(this->OptionFrame);
    this->ShowWorkspaceButton->Create();
    this->ShowWorkspaceButton->SelectedStateOff();
    this->ShowWorkspaceButton->SetText("Show workspace");
    this->ShowWorkspaceButton->SetBalloonHelpString("Show workspace of the robot");
  }

  if (!this->AddTargetsOnClickButton)
  {
    this->AddTargetsOnClickButton = vtkKWCheckButton::New();
  } 
  if (!this->AddTargetsOnClickButton->IsCreated()) {
    this->AddTargetsOnClickButton->SetParent(this->OptionFrame);
    this->AddTargetsOnClickButton->Create();
    this->AddTargetsOnClickButton->SelectedStateOff();
    this->AddTargetsOnClickButton->SetText("Add target by image click");
    this->AddTargetsOnClickButton->SetBalloonHelpString("Add a target if image is clicked, with the current needle");
  }

  if (!this->ShowNeedleButton)
  {
    this->ShowNeedleButton = vtkKWCheckButton::New();
  } 
  if (!this->ShowNeedleButton->IsCreated()) {
    this->ShowNeedleButton->SetParent(this->OptionFrame);
    this->ShowNeedleButton->Create();
    this->ShowNeedleButton->SelectedStateOff();
    this->ShowNeedleButton->SetText("Needle");
    this->ShowNeedleButton->SetBalloonHelpString("Show predicted needle path");
  }

  if (!this->ShowTemplateButton)
  {
    this->ShowTemplateButton = vtkKWCheckButton::New();
  } 
  if (!this->ShowTemplateButton->IsCreated()) {
    this->ShowTemplateButton->SetParent(this->OptionFrame);
    this->ShowTemplateButton->Create();
    this->ShowTemplateButton->SelectedStateOff();
    this->ShowTemplateButton->SetText("Template");
    this->ShowTemplateButton->SetBalloonHelpString("Show predicted needle path");
  }

  this->Script("pack %s %s %s %s -side left -expand y -padx 2 -pady 2",
               this->AddTargetsOnClickButton->GetWidgetName(),
               this->ShowWorkspaceButton->GetWidgetName(),
               this->ShowNeedleButton->GetWidgetName(),
               this->ShowTemplateButton->GetWidgetName());
  //this->Script("grid %s -row 0 -column 0 -columnspan 2 -padx 2 -pady 2 -sticky ew", this->LoadTargetingVolumeButton->GetWidgetName());
  //this->Script("grid %s -row 0 -column 0 -columnspan 2 -padx 2 -pady 2 -sticky ew", this->VolumeSelectorWidget->GetWidgetName());
  //this->Script("grid %s -row 1 -column 0 -columnspan 2 -padx 2 -pady 2 -sticky ew", this->NeedleTypeMenuList->GetWidgetName());
  //this->Script("grid %s -row 0 -column 0 -padx 2 -pady 2", this->AddTargetsOnClickButton->GetWidgetName());
  //this->Script("grid %s -row 0 -column 1 -padx 2 -pady 2", this->ShowWorkspaceButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ShowTargetListFrame()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();

  if (!this->TargetListFrame)
    {
    this->TargetListFrame = vtkKWFrame::New();
    }
  if (!this->TargetListFrame->IsCreated())
    {
    this->TargetListFrame->SetParent(parent);
    this->TargetListFrame->Create();
    }
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->TargetListFrame->GetWidgetName());

  if (!this->TargetList)
    {
    this->TargetList = vtkKWMultiColumnListWithScrollbars::New();
    this->TargetList->SetParent(this->TargetListFrame);
    this->TargetList->Create();
    this->TargetList->SetHeight(1);
    this->TargetList->GetWidget()->SetSelectionTypeToRow();
    this->TargetList->GetWidget()->SetSelectionBackgroundColor(1,0,0);
    this->TargetList->GetWidget()->MovableRowsOff();
    this->TargetList->GetWidget()->MovableColumnsOff();

    for (int col = 0; col < COL_COUNT; col ++)
      {
      this->TargetList->GetWidget()->AddColumn(COL_LABELS[col]);
      this->TargetList->GetWidget()->SetColumnWidth(col, COL_WIDTHS[col]);
      this->TargetList->GetWidget()->SetColumnAlignmentToLeft(col);
      //this->TargetList->GetWidget()->ColumnEditableOff(col);
      this->TargetList->GetWidget()->ColumnEditableOn(col);
      this->TargetList->GetWidget()->SetColumnEditWindowToSpinBox(col);
      }

    this->TargetList->GetWidget()->SetColumnEditWindowToCheckButton(0);

    }
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->TargetList->GetWidgetName());


  if(!this->DeleteButton)
    {
    this->DeleteButton = vtkKWPushButton::New();
    }
  if(!this->DeleteButton->IsCreated())
    {
    this->DeleteButton->SetParent(this->TargetListFrame);
    this->DeleteButton->SetText("Delete");
    this->DeleteButton->SetBalloonHelpString("Delete selected target");
    this->DeleteButton->Create();
    }    
  this->Script("pack %s -side top -anchor ne -padx 2 -pady 4", 
                    this->DeleteButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ShowTargetControlFrame()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();

  if (!this->TargetControlFrame)
    {
    this->TargetControlFrame = vtkKWFrame::New();
    }
  if (!this->TargetControlFrame->IsCreated())
    {
    this->TargetControlFrame->SetParent(parent);
    this->TargetControlFrame->Create();
    }
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->TargetControlFrame->GetWidgetName());

    if (!this->NeedlePositionMatrix)
    {
    this->NeedlePositionMatrix = vtkKWMatrixWidgetWithLabel::New();
    this->NeedlePositionMatrix->SetParent(this->TargetControlFrame);
    this->NeedlePositionMatrix->Create();
    this->NeedlePositionMatrix->SetLabelText("Position (X, Y, Z):");
    this->NeedlePositionMatrix->ExpandWidgetOff();
    this->NeedlePositionMatrix->GetLabel()->SetWidth(18);
    this->NeedlePositionMatrix->SetBalloonHelpString("Set the needle position");

    vtkKWMatrixWidget *matrix =  this->NeedlePositionMatrix->GetWidget();
    matrix->SetNumberOfColumns(3);
    matrix->SetNumberOfRows(1);
    matrix->SetElementWidth(12);
    matrix->SetRestrictElementValueToDouble();
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  //if (!this->NeedleOrientationMatrix)
  //  {
  //  this->NeedleOrientationMatrix = vtkKWMatrixWidgetWithLabel::New();
  //  this->NeedleOrientationMatrix->SetParent(this->TargetControlFrame);
  //  this->NeedleOrientationMatrix->Create();
  //  this->NeedleOrientationMatrix->SetLabelText("Orientation (W, X, Y, Z):");
  //  this->NeedleOrientationMatrix->ExpandWidgetOff();
  //  this->NeedleOrientationMatrix->GetLabel()->SetWidth(18);
  //  this->NeedleOrientationMatrix->SetBalloonHelpString("Set the needle orientation");
  //
  //  vtkKWMatrixWidget *matrix =  this->NeedleOrientationMatrix->GetWidget();
  //  matrix->SetNumberOfColumns(4);
  //  matrix->SetNumberOfRows(1);
  //  matrix->SetElementWidth(12);
  //  matrix->SetRestrictElementValueToDouble();
  //  matrix->SetElementChangedCommandTriggerToAnyChange();
  //  }

  //this->Script("pack %s %s -side top -anchor nw -expand n -padx 2 -pady 2",
  //             this->NeedlePositionMatrix->GetWidgetName(),
  //             this->NeedleOrientationMatrix->GetWidgetName());
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->NeedlePositionMatrix->GetWidgetName());

  if(!this->Message)
    {
    this->Message = vtkKWText::New();
    }
  if(!this->Message->IsCreated())
    {
    this->Message->SetParent(this->TargetControlFrame);
    this->Message->Create();
    this->Message->SetText("Select needle type, then click on image to add a target");      
    this->Message->SetBackgroundColor(0.7, 0.7, 0.95);
    this->Message->SetHeight(6);
    this->Message->SetWrapToWord();
    this->Message->ReadOnlyOn();
    this->Message->SetBorderWidth(2);
    this->Message->SetReliefToGroove();
    this->Message->SetFont("times 12 bold");
    this->Message->SetForegroundColor(0.0, 0.0, 0.0);

    }
  this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 6", 
                this->Message->GetWidgetName());

  if (!this->MoveButton)
    {
    this->MoveButton = vtkKWPushButton::New();
    this->MoveButton->SetParent (this->TargetControlFrame);
    this->MoveButton->Create();
    this->MoveButton->SetText("Move");
    this->MoveButton->SetBalloonHelpString("Move the robot to the position");
    }

  if (!this->StopButton)
    {
    this->StopButton = vtkKWPushButton::New();
    this->StopButton->SetParent (this->TargetControlFrame);
    this->StopButton->Create();
    this->StopButton->SetText("Stop");
    this->StopButton->SetBalloonHelpString("Stop the robot");
    }

  this->Script("pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->MoveButton->GetWidgetName(),
               this->StopButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{

  // -----------------------------------------------------------------
  // Move Button Pressed

  if (this->MoveButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    if (this->Logic && this->NeedlePositionMatrix /*&& this->NeedleOrientationMatrix*/)
      {
      float position[3];   // position parameters

      vtkKWMatrixWidget* matrix = this->NeedlePositionMatrix->GetWidget();
      position[0] = (float) matrix->GetElementValueAsDouble(0, 0);
      position[1] = (float) matrix->GetElementValueAsDouble(0, 1);
      position[2] = (float) matrix->GetElementValueAsDouble(0, 2);

      vtkMRMLNode* node = this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(this->GetLITTPlanManager()->GetRobotNode()->GetTargetTransformNodeID());
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);

      if (transformNode)
        {
        vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
        matrix->Identity();

        matrix->SetElement(0, 3, position[0]);
        matrix->SetElement(1, 3, position[1]);
        matrix->SetElement(2, 3, position[2]);

        vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();
        transformToParent->DeepCopy(matrix);

        // Send move to command 
        this->GetLITTPlanManager()->GetRobotNode()->MoveTo(transformNode->GetID());
        this->UpdateGUI();

        }
      }
    }

  // -----------------------------------------------------------------
  // Stop Button Pressed

  else if (this->StopButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {

    }

  /////////

  vtkMRMLLITTPlanManagerNode *mrmlNode = this->GetGUI()->GetLITTPlanManagerNode();

  if(!mrmlNode)
      return;

  if (this->DeleteButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
      vtkLITTPlanTargetDescriptor *targetDesc = mrmlNode->GetTargetDescriptorAtIndex(mrmlNode->GetCurrentTargetIndex());       
      if (this->TargetPlanListNode!=NULL && targetDesc!=NULL)
      {
        int fidIndex=this->TargetPlanListNode->GetFiducialIndex(targetDesc->GetFiducialID());
        if (fidIndex>=0)
        {
          this->TargetPlanListNode->RemoveFiducial(fidIndex);
          mrmlNode->SetCurrentTargetIndex(-1);
          UpdateTargetListGUI();
        }
        else
        {
          vtkErrorMacro("Cannot delete target, fiducial not found");
        }
      }
      else
      {
        vtkErrorMacro("Cannot delete target, fiducial or target descriptor is invalid");
      }
    }

  //// load targeting volume dialog button
  //if (this->LoadTargetingVolumeButton && this->LoadTargetingVolumeButton == vtkKWPushButton::SafeDownCast(caller) && (event == vtkKWPushButton::InvokedEvent))
  //  {
  //  this->GetApplication()->Script("::LoadVolume::ShowDialog");
  //  }

  // show workspace button
   if (this->ShowWorkspaceButton && this->ShowWorkspaceButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
      this->ShowWorkspaceModel(this->ShowWorkspaceButton->GetSelectedState() == 1);
    }

   if (this->ShowNeedleButton && this->ShowNeedleButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    this->ShowNeedle(this->ShowNeedleButton->GetSelectedState() == 1);
    }
   
   if (this->ShowTemplateButton && this->ShowTemplateButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    this->ShowTemplate(this->ShowTemplateButton->GetSelectedState() == 1);
    }

 // activate fiducial placement
 if (this->AddTargetsOnClickButton && this->AddTargetsOnClickButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
  {
    // Activate target fiducials in the Fiducial GUI
    if (this->GetLogic()==NULL)
    {
      vtkErrorMacro("Logic is invalid");
    }    
    else
    {
      EnableAddTargetsOnClickButton(this->AddTargetsOnClickButton->GetSelectedState()==1);
    }

  }

 //if (this->NeedleTypeMenuList && this->NeedleTypeMenuList->GetWidget()->GetMenu() == vtkKWMenu::SafeDownCast(caller) && (event == vtkKWMenu::MenuItemInvokedEvent))
 //  {
 //    mrmlNode->SetCurrentNeedleIndex(this->NeedleTypeMenuList->GetWidget()->GetMenu()->GetIndexOfSelectedItem());
 //  }
 //
  if (this->TargetListSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) &&
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLFiducialListNode *fid = vtkMRMLFiducialListNode::SafeDownCast(this->TargetListSelectorWidget->GetSelected());
    if (fid != NULL)
      {
      vtkMRMLLITTPlanManagerNode* manager=this->GetLITTPlanManager();
      manager->SetAndObserveTargetPlanListNodeID(fid->GetID());
      }
    }


  if (this->VolumeSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) &&
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
  {
    vtkMRMLScalarVolumeNode *volume = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    if (volume != NULL)
    {
      this->GetGUI()->GetLogic()->SelectVolumeInScene(volume, VOL_TARGETING);
      this->AddTargetsOnClickButton->SetSelectedState(1);
    }
  }
}


//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ProcessMRMLEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  vtkMRMLFiducialListNode *targetNode = vtkMRMLFiducialListNode::SafeDownCast(caller);
  if (targetNode!=NULL && targetNode == this->TargetPlanListNode )
  {
    switch (event)
    {
    case vtkCommand::ModifiedEvent:
    case vtkMRMLScene::NodeAddedEvent: // Node Added Event : when a fiducial is added to the list
    case vtkMRMLFiducialListNode::FiducialModifiedEvent:
    case vtkMRMLFiducialListNode::DisplayModifiedEvent:
      UpdateTargetListGUI();
      break;
    }
  }

  vtkMRMLLITTPlanManagerNode *managerNode = vtkMRMLLITTPlanManagerNode::SafeDownCast(caller);
  if (managerNode!=NULL && managerNode==GetLITTPlanManager())
    {
    switch (event)
      {
      case vtkMRMLLITTPlanManagerNode::CurrentTargetChangedEvent:
        this->GUI->BringTargetToViewIn2DViews(vtkLITTPlanGUI::BRING_MARKERS_TO_VIEW_KEEP_CURRENT_ORIENTATION);
        break;
      }
    }

}

//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::AddMRMLObservers()
{
  vtkMRMLLITTPlanManagerNode* manager=this->GetLITTPlanManager();
  if (manager==NULL)
  {
    return;
  }

  vtkMRMLFiducialListNode* plan = manager->GetTargetPlanListNode();
  if (plan)
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);

    // Set and observe target plan list
    //vtkObject *oldNode = this->TargetPlanListNode;
    this->MRMLObserverManager->SetAndObserveObjectEvents(vtkObjectPointer(&(this->TargetPlanListNode)),(plan),(events));
  }
 
  manager->AddObserver(vtkMRMLLITTPlanManagerNode::CurrentTargetChangedEvent, this->MRMLCallbackCommand);
}

//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::RemoveMRMLObservers()
{
  if (this->TargetPlanListNode!=NULL)
  {    
    this->MRMLObserverManager->SetAndObserveObjectEvents(vtkObjectPointer(&(this->TargetPlanListNode)), NULL, NULL);
  }
  vtkMRMLLITTPlanManagerNode* manager=this->GetLITTPlanManager();
  if (manager!=NULL)
  {
    manager->RemoveObservers(vtkMRMLLITTPlanManagerNode::CurrentTargetChangedEvent, this->MRMLCallbackCommand);    
  }
}


//---------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::OnMultiColumnListUpdate(int row, int col, char * str)
{

  vtkMRMLFiducialListNode* fidList = this->GetLITTPlanManager()->GetTargetPlanListNode();

  if (fidList == NULL)
    {
    return;
    }

  bool updated=false;

  // make sure that the row and column exists in the table
  if ((row >= 0) && (row < this->TargetList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->TargetList->GetWidget()->GetNumberOfColumns()))
    {
      
    // now update the requested value
    if (col == COL_NAME)
      {
      fidList->SetNthFiducialLabelText(row, str);
      updated=true;
      }
    else if (col >= COL_X && col <= COL_Z)
      {
      // get the current xyz
      float * xyz = fidList->GetNthFiducialXYZ(row);
      // now set the new one
      float newCoordinate = atof(str);
      if ( xyz )
        {
        if (col == COL_X)
          {
          fidList->SetNthFiducialXYZ(row, newCoordinate, xyz[1], xyz[2]);
          updated=true;
          }
        if (col == COL_Y)
          {
          fidList->SetNthFiducialXYZ(row, xyz[0], newCoordinate, xyz[2]);
          updated=true;
          }
        if (col == COL_Z)
          {
          fidList->SetNthFiducialXYZ(row, xyz[0], xyz[1], newCoordinate);
          updated=true;
          }
        }            
      }
    else if (col >= COL_OR_W  && col <= COL_OR_Z)
      {
      float * wxyz = fidList->GetNthFiducialOrientation(row);
      float newCoordinate = atof(str);
      if (col == COL_OR_W)
        {
        fidList->SetNthFiducialOrientation(row, newCoordinate, wxyz[1], wxyz[2], wxyz[3]);
        updated=true;
        }
      if (col == COL_OR_X)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], newCoordinate, wxyz[2], wxyz[3]);
        updated=true;
        }
      if (col == COL_OR_Y)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], newCoordinate, wxyz[3]);
        updated=true;
        }
      if (col == COL_OR_Z)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], wxyz[2], newCoordinate);
        updated=true;
        }
      }
    else
      {
      return;
      }
    }
  else
    {
    }
  if (updated)
  {
    this->GetLogic()->UpdateTargetListFromMRML();
  }
}


//---------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::OnMultiColumnListSelectionChanged()
{

  vtkMRMLFiducialListNode* fidList = this->GetLITTPlanManager()->GetTargetPlanListNode();

  if (fidList == NULL)
    {
    return;
    }

  if (this->MRMLScene)
    {
    this->MRMLScene->SaveStateForUndo();
    }

  int numRows = this->TargetList->GetWidget()->GetNumberOfSelectedRows();
  if (numRows == 1)
    {   
    
    int rowIndex = this->TargetList->GetWidget()->GetIndexOfFirstSelectedRow();    
    int targetIndex=this->TargetList->GetWidget()->GetRowAttributeAsInt(rowIndex, TARGET_INDEX_ATTR);
    vtkLITTPlanTargetDescriptor* targetDesc=this->GetLITTPlanManager()->GetTargetDescriptorAtIndex(targetIndex);    

    if (targetDesc==NULL)
    {
      vtkErrorMacro("Target descriptor not found");
      return;
    }
    // Copy the values to inputs
    vtkKWMatrixWidget* matrix = this->NeedlePositionMatrix->GetWidget();
    double* xyz=targetDesc->GetRASLocation();
    matrix->SetElementValueAsDouble(0, 0, xyz[0]);
    matrix->SetElementValueAsDouble(0, 1, xyz[1]);
    matrix->SetElementValueAsDouble(0, 2, xyz[2]);

    //matrix = this->NeedleOrientationMatrix->GetWidget();
    //double* wxyz=targetDesc->GetRASOrientation();
    //matrix->SetElementValueAsDouble(0, 0, wxyz[0]);
    //matrix->SetElementValueAsDouble(0, 1, wxyz[1]);
    //matrix->SetElementValueAsDouble(0, 2, wxyz[2]);
    //matrix->SetElementValueAsDouble(0, 3, wxyz[3]);
          
    this->GetLITTPlanManager()->SetCurrentTargetIndex(targetIndex);
    }
}

//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::UpdateTargetListGUI()
{
  vtkMRMLFiducialListNode* activeFiducialListNode=NULL;
  if (this->GetLITTPlanManager()!=NULL)
  {
    activeFiducialListNode=this->GetLITTPlanManager()->GetTargetPlanListNode();
  }

  if (activeFiducialListNode == NULL)    //clear out the list box
    {
    if (this->TargetList)
      {
      if (this->TargetList->GetWidget()->GetNumberOfRows() != 0)
        {
        this->TargetList->GetWidget()->DeleteAllRows();
        }
      }
    return;
    }
  
  // create new target points, if necessary
  this->GetLogic()->UpdateTargetListFromMRML();

  vtkMRMLLITTPlanManagerNode *manager = this->GetGUI()->GetLITTPlanManagerNode();
  if (!manager)
  {
    return;
  }

  //int numPoints = activeFiducialListNode->GetNumberOfFiducials();
  int numPoints = manager->GetTotalNumberOfTargets();

  bool deleteFlag = true;

  if (this->TargetList && numPoints != this->TargetList->GetWidget()->GetNumberOfRows())
    {
    // clear out the multi column list box and fill it in with the
    // new list
    this->TargetList->GetWidget()->DeleteAllRows();
    }
  else
    {
    deleteFlag = false;
    }
        
  double *xyz;
  double *wxyz;

  for (int row = 0; row < numPoints; row++)
    {      
    int targetIndex=row;
    vtkLITTPlanTargetDescriptor* target = manager->GetTargetDescriptorAtIndex(targetIndex);

    if (deleteFlag)
      {
      // add a row for this point
      this->TargetList->GetWidget()->AddRow();
      }
    this->TargetList->GetWidget()->SetRowAttributeAsInt(row, TARGET_INDEX_ATTR, targetIndex);

    xyz=target->GetRASLocation();
    wxyz=target->GetRASOrientation();

    if (xyz == NULL)
      {
      vtkErrorMacro ("UpdateTargetListGUI: ERROR: got null xyz for point " << row << endl);
      }

    if (target->GetName().compare(this->TargetList->GetWidget()->GetCellText(row,COL_NAME)) != 0)
        {
          this->TargetList->GetWidget()->SetCellText(row,COL_NAME,target->GetName().c_str());
        }               

    // selected
    vtkKWMultiColumnList* columnList = this->TargetList->GetWidget();
    if (xyz != NULL)
      {
      for (int i = 0; i < 3; i ++) // for position (x, y, z)
        {
        if (deleteFlag || columnList->GetCellTextAsDouble(row,COL_X+i) != xyz[i])
          {
          columnList->SetCellTextAsDouble(row,COL_X+i,xyz[i]);
          }
        }
      }
    if (wxyz != NULL)
      {
      for (int i = 0; i < 4; i ++) // for orientation (w, x, y, z)
        {
        if (deleteFlag || columnList->GetCellTextAsDouble(row, COL_OR_W+i) != wxyz[i])
          {
          columnList->SetCellTextAsDouble(row,COL_OR_W+i,wxyz[i]);
          }
        }
      }

    //if (target->GetNeedleTypeString().compare(this->TargetList->GetWidget()->GetCellText(row,COL_NEEDLE)) != 0)
    //{
    //  this->TargetList->GetWidget()->SetCellText(row,COL_NEEDLE,target->GetNeedleTypeString().c_str());
    //}
    //
    }  

}



//-----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::AddGUIObservers()
{
  this->RemoveGUIObservers();

  //if (this->LoadTargetingVolumeButton)
  //  {
  //  this->LoadTargetingVolumeButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand); 
  //  }
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);  
    }
  if (this->TargetListSelectorWidget)
    {
    this->TargetListSelectorWidget->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);  
    }

  if (this->ShowWorkspaceButton)
    {
      this->ShowWorkspaceButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ShowNeedleButton)
    {
      this->ShowNeedleButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ShowTemplateButton)
    {
      this->ShowTemplateButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AddTargetsOnClickButton)
    {
      this->AddTargetsOnClickButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }  
  //if (this->NeedleTypeMenuList)
  //  {
  //  this->NeedleTypeMenuList->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
//  }
    if (this->DeleteButton)
    {
    this->DeleteButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MoveButton)
    {
    this->MoveButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->StopButton)
    {
    this->StopButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->TargetList)
    {
    this->TargetList->GetWidget()->SetCellUpdatedCommand(this, "OnMultiColumnListUpdate");
    this->TargetList->GetWidget()->SetSelectionChangedCommand(this, "OnMultiColumnListSelectionChanged");
    }
}
//-----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::RemoveGUIObservers()
{
  //if (this->LoadTargetingVolumeButton)
  //  {
  //  this->LoadTargetingVolumeButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand); 
  //  }
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->RemoveObserver ((vtkCommand *)this->GUICallbackCommand);  
    }
  if (this->TargetListSelectorWidget)
    {
    this->TargetListSelectorWidget->RemoveObserver ((vtkCommand *)this->GUICallbackCommand);  
    }
  if (this->ShowWorkspaceButton)
    {
    this->ShowWorkspaceButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->ShowNeedleButton)
    {
    this->ShowNeedleButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->ShowTemplateButton)
    {
    this->ShowTemplateButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->AddTargetsOnClickButton)
    {
      this->AddTargetsOnClickButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }  
  //if (this->NeedleTypeMenuList)
  //  {
  //    this->NeedleTypeMenuList->GetWidget()->GetMenu()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  //  }
  if (this->DeleteButton)
    {
    this->DeleteButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MoveButton)
    {
    this->MoveButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->StopButton)
    {
    this->StopButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->TargetList)
    {
    this->TargetList->GetWidget()->SetCellUpdatedCommand(this, "");
    this->TargetList->GetWidget()->SetSelectionChangedCommand(this, "");
    }
}

//--------------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::UpdateGUI()
{
  vtkMRMLLITTPlanManagerNode *mrmlNode = this->GetGUI()->GetLITTPlanManagerNode();

  if (!mrmlNode)
  {
    return;
  }

  const char* volNodeID = mrmlNode->GetTargetingVolumeNodeRef();
  vtkMRMLScalarVolumeNode *volNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(volNodeID));
  if (volNode && this->VolumeSelectorWidget)
  {
    this->VolumeSelectorWidget->UpdateMenu();
    this->VolumeSelectorWidget->SetSelected( volNode );
  }

  const char* targetNodeID = mrmlNode->GetTargetPlanListNodeID();
  vtkMRMLFiducialListNode *targetNode=vtkMRMLFiducialListNode::SafeDownCast(this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(targetNodeID));
  if (targetNode && this->TargetListSelectorWidget)
    {
    this->TargetListSelectorWidget->UpdateMenu();
    this->TargetListSelectorWidget->SetSelected(targetNode);
    }

  // Display information about the currently selected target descriptor    
  if (this->Message)
  {
  // NOTE: This part will be rewritten with TargetDescriptor.
  //       Currently TargetDescriptor does not support template index...
  /*
    vtkMRMLRobotNode* robot=NULL;
    if (this->GetLITTPlanManager()!=NULL)
    {
      robot=this->GetLITTPlanManager()->GetRobotNode();
    }
    vtkLITTPlanTargetDescriptor *targetDesc = mrmlNode->GetTargetDescriptorAtIndex(mrmlNode->GetCurrentTargetIndex()); 

    if (robot!=NULL && targetDesc!=NULL)
    {
      std::string info=robot->GetTargetInfoText(targetDesc);
      this->Message->SetText(info.c_str());
    }
    else
    {
      // no target info available for the current robot with the current target
      this->Message->SetText("");
    }
  */
    if (this->GetLITTPlanManager()!=NULL && this->Message)
      {
      vtkMRMLTransPerinealProstateTemplateNode* robot;
      robot = vtkMRMLTransPerinealProstateTemplateNode::SafeDownCast(this->GetLITTPlanManager()->GetRobotNode());
      if (robot)
        {
        this->Message->SetText(robot->GetScreenMessage());
        }
      }
  }

  UpdateTargetListGUI();

  //if (this->NeedleTypeMenuList)
  //  {
  //  this->NeedleTypeMenuList->GetWidget()->GetMenu()->DeleteAllItems();
  //  for (int i = 0; i < mrmlNode->GetNumberOfNeedles(); i++)
  //    {
  //    std::ostrstream needleTitle;
  //    needleTitle << mrmlNode->GetNeedleType(i) << " ("
  //      <<mrmlNode->GetNeedleOvershoot(i)<<"mm overshoot, "
  //      <<mrmlNode->GetNeedleLength(i)<<"mm length"
  //      << ")" << std::ends;      
  //    this->NeedleTypeMenuList->GetWidget()->GetMenu()->AddRadioButton(needleTitle.str());
  //    needleTitle.rdbuf()->freeze();
  //    needleTitle.clear();
  //    }
  //  int needleIndex=mrmlNode->GetCurrentNeedleIndex();
  //  this->NeedleTypeMenuList->GetWidget()->GetMenu()->SelectItem(needleIndex);
  //  }
}

//---------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ShowNeedle(bool show)
{
  vtkMRMLTransPerinealProstateTemplateNode* robotNode = 
    vtkMRMLTransPerinealProstateTemplateNode::SafeDownCast(this->GetLITTPlanManager()->GetRobotNode());

  if (robotNode == NULL)
    return;

  vtkMRMLModelNode* modelNode = robotNode->GetActiveNeedleModelNode();

  if (modelNode == NULL)
    return;

  vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();

  if (displayNode == NULL)
    return;

  // Show the predicted needle path
  displayNode->SetVisibility(show);

}

//---------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::ShowTemplate(bool show)
{
  vtkMRMLTransPerinealProstateTemplateNode* robotNode = 
    vtkMRMLTransPerinealProstateTemplateNode::SafeDownCast(this->GetLITTPlanManager()->GetRobotNode());

  if (robotNode == NULL)
    return;

  vtkMRMLModelNode* modelNode = robotNode->GetTemplateModelNode();

  if (modelNode == NULL)
    return;

  vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();

  if (displayNode == NULL)
    return;

  // Show the predicted needle path
  displayNode->SetVisibility(show);

}


//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::HideUserInterface()
{  
  RemoveMRMLObservers(); // HideUserInterface deletes the reference to the scene, so RemoveMRMLObservers shall be done before calling HideUserInterface
  RemoveGUIObservers();  
  Superclass::HideUserInterface();
}

//----------------------------------------------------------------------------
void vtkLITTPlanStepTargetingTemplate::EnableAddTargetsOnClickButton(bool enable)
{
  if (this->GetLITTPlanManager()==NULL)
  {
    return;
  }
  vtkMRMLFiducialListNode* fidNode = this->GetLITTPlanManager()->GetTargetPlanListNode();
  GetLogic()->SetCurrentFiducialList(fidNode);
  GetLogic()->SetMouseInteractionMode( (enable) ? 
    vtkMRMLInteractionNode::Place : 
    vtkMRMLInteractionNode::ViewTransform
    );
}
