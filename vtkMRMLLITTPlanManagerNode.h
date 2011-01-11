/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLLITTPlanManagerNode_h
#define __vtkMRMLLITTPlanManagerNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkObject.h"
#include "vtkStdString.h"
#include "vtkLITTPlanWin32Header.h" 

#include "vtkMRMLFiducialListNode.h"

#include "vtkLITTPlanTargetDescriptor.h"

class vtkLITTPlanStep;
class vtkStringArray;
class vtkMRMLRobotNode;

enum VolumeType
{
  VOL_GENERIC, // any other than the specific volumes 
  VOL_TARGETING,
  VOL_VERIFICATION,
};

class VTK_PROSTATENAV_EXPORT vtkMRMLLITTPlanManagerNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

  //BTX
  
  // Events
  enum {
    CurrentTargetChangedEvent = 200900,
  };

  //ETX


 public:

  //----------------------------------------------------------------
  // Get and Set Macros
  //----------------------------------------------------------------

  static const char* GetLITTPlanModuleVersion();
  static int GetLITTPlanWorkingCopyRevision();

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLLITTPlanManagerNode *New();
  vtkTypeMacro(vtkMRMLLITTPlanManagerNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  void Init(const char* defaultNeedleListConfig);

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Updates other nodes in the scene depending on this node
  // or updates this node if it depends on other nodes
  virtual void UpdateScene(vtkMRMLScene *);

// Description:
  // Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  void UpdateReferences();

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "LITTPlanExam";};


  // Description:
  // Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  // Workflow wizard step management
  //----------------------------------------------------------------

  // Description:
  // Get number of wizard steps
  int GetNumberOfSteps();

  // Description:
  // Get number of wizard steps
  const char* GetStepName(int i);
  
  // Description:
  // Switch step. Returns 0 if it is failed.
  int SwitchStep(int newStep);

  // Description:
  // Get current workflow step.
  int GetCurrentStep();

  vtkSetReferenceStringMacro(TargetingVolumeNodeRef);
  vtkGetStringMacro(TargetingVolumeNodeRef);

  vtkSetReferenceStringMacro(VerificationVolumeNodeRef);
  vtkGetStringMacro(VerificationVolumeNodeRef);

  //----------------------------------------------------------------
  // Needle Management
  //----------------------------------------------------------------

  //BTX
  // Description:
  // Set/Get the number of needle types and TargetingFiducialsLists, because there could be more than one lists, depending on needle type
  int GetNumberOfNeedles(){ return this->NeedlesVector.size();};
  //ETX

  // Description:
  // Set/Get the current needle index (if <0 then there is no active needle)
  vtkGetMacro(CurrentNeedleIndex,int);
  vtkSetMacro(CurrentNeedleIndex,int);

  //BTX
  bool SetNeedle(unsigned int needleIndex, NeedleDescriptorStruct needleDesc);
  // returns false if needle info was not found
  bool GetNeedle(unsigned int needleIndex, NeedleDescriptorStruct &needleDesc);
  NeedleDescriptorStruct* GetNeedle(vtkLITTPlanTargetDescriptor *targetDesc);
  
  //ETX

  //----------------------------------------------------------------
  // Target Management
  //----------------------------------------------------------------
  
  bool AddTargetToFiducialList(double targetRAS[3], unsigned int fiducialListIndex, unsigned int targetNr, int & fiducialIndex);  
  bool GetTargetFromFiducialList(int fiducialListIndex, int fiducialIndex, double &r, double &a, double &s);  
  void SetFiducialColor(int fiducialIndex, bool selected); // :TODO: rename it to SetFiducialSelected

  //BTX
  // Description:
  // Get Targeting Fiducials Lists names (used in the wizard steps)
  std::string GetTargetingFiducialsListDescription(unsigned int index)
    {
    if (index < this->NeedlesVector.size())
      return this->NeedlesVector[index].mDescription;
    else
      return "";
    }; 
  //ETX
  
  unsigned int AddTargetDescriptor(vtkLITTPlanTargetDescriptor *target);
  int RemoveTargetDescriptorAtIndex(unsigned int index); // return with 0 if failed
  vtkLITTPlanTargetDescriptor *GetTargetDescriptorAtIndex(unsigned int index);
  int GetTotalNumberOfTargets() { return this->TargetDescriptorsVector.size();};
  vtkGetMacro(CurrentTargetIndex, int);
  int SetCurrentTargetIndex(int index);

  // Description:
  // Set and start observing target plan list
  //void SetAndObserveTargetPlanList(vtkMRMLFiducialListNode* ptr);
  vtkGetStringMacro(TargetPlanListNodeID);
  vtkMRMLFiducialListNode* GetTargetPlanListNode();
  void SetAndObserveTargetPlanListNodeID(const char *targetPlanListNodeID);

  vtkGetStringMacro(RobotNodeID);
  vtkMRMLRobotNode* GetRobotNode();
  void SetAndObserveRobotNodeID(const char *robotNodeID);

 protected:

  bool ReadNeedleListFromConfigXml(const char* needleListConfigStr);

  vtkStdString GetWorkflowStepsString();
  bool SetWorkflowStepsFromString(const vtkStdString& workflowStepsString);

  //BTX
  bool GetAttNameSection(const std::string& attName, const std::string& groupName, unsigned int &sectionInd, std::string &sectionName);
  //ETX

  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLLITTPlanManagerNode();
  ~vtkMRMLLITTPlanManagerNode();
  vtkMRMLLITTPlanManagerNode(const vtkMRMLLITTPlanManagerNode&);
  void operator=(const vtkMRMLLITTPlanManagerNode&);


 protected:
  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------

  // List of workflow steps (wizard pages)
  vtkStringArray *StepList;
  
  int CurrentStep;

  //BTX
  std::vector<NeedleDescriptorStruct> NeedlesVector;
  int CurrentNeedleIndex;

  std::vector<vtkLITTPlanTargetDescriptor*> TargetDescriptorsVector;
  int CurrentTargetIndex; // if <0 then no current target is selected

  //ETX

  vtkSetReferenceStringMacro(TargetPlanListNodeID);
  char *TargetPlanListNodeID;
  vtkMRMLFiducialListNode* TargetPlanListNode;

  vtkSetReferenceStringMacro(RobotNodeID);
  char *RobotNodeID;
  vtkMRMLRobotNode* RobotNode;

  char *TargetingVolumeNodeRef;
  char *VerificationVolumeNodeRef;

  bool Initialized;
};

#endif

