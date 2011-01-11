/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

// .NAME vtkLITTPlanLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 

#ifndef __vtkLITTPlanLogic_h
#define __vtkLITTPlanLogic_h

#include "vtkLITTPlanWin32Header.h"

#include "vtkKWTkUtilities.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLSliceNode.h"

#include "vtkMRMLLITTPlanManagerNode.h"

class vtkLITTPlanGUI;

class VTK_PROSTATENAV_EXPORT vtkLITTPlanLogic : public vtkSlicerModuleLogic 
{
  
 public:

   //BTX
   enum 
   {  // Events
     LocatorUpdateEvent      = 50000,
     StatusUpdateEvent       = 50001,
   };
   //ETX

  
  static vtkLITTPlanLogic *New();
  
  vtkTypeRevisionMacro(vtkLITTPlanLogic,vtkObject);
  
  void SetGUI(vtkLITTPlanGUI* gui) { this->GUI = gui; };
  vtkLITTPlanGUI* GetGUI()         { return this->GUI; };

  void PrintSelf(ostream&, vtkIndent);  
  
  int  Enter();
  void TimerHandler();
  
  int  RobotStop();
  int  RobotMoveTo(float px, float py, float pz,
                   float nx, float ny, float nz,
                   float tx, float ty, float tz);
  int  RobotMoveTo(float position[3], float orientation[4]);

  int  RobotMoveTo();
  
  int  ScanStart();
  int  ScanPause();
  int  ScanStop();
  
  //BTX
  //Image* ReadCalibrationImage(const char* filename, int* width, int* height,
  //                            std::vector<float>& position, std::vector<float>& orientation);

  bool AddTargetToNeedle(std::string needleType, float* rasLocation, unsigned int & targetDescIndex);

  // Description:
  // Add volume to MRML scene and return the MRML node.
  // If volumeType is specified, then the volume is also selected as the current Calibration
  // targeting or verification volume.
  vtkMRMLScalarVolumeNode *AddVolumeToScene(const char *fileName, VolumeType volumeType=VOL_GENERIC);

  // Description:
  // Set a specific role for a loaded volume.
  int SelectVolumeInScene(vtkMRMLScalarVolumeNode* volumeNode, VolumeType volumeType);

  // Description:
  // Show/hide robot workspace. Returns with 0 in case of failure.
  int ShowWorkspaceModel(bool show);
  bool IsWorkspaceModelShown();

  // Description:
  // Show/hide robot. Returns with 0 in case of failure.
  int ShowRobotModel(bool show);
  bool IsRobotModelShown();

  // Description:
  // Switch mouse interaction mode to activate target placement
  // by clicking on the image
  // vtkMRMLInteractionNode::Place = place fiducials
  // vtkMRMLInteractionNode::ViewTransform = rotate scene
  // Return value: zero if an error occurred
  int SetMouseInteractionMode(int mode);

  // Description:
  // Select the current fidicual list in the Fiducial module
  // If the user clicks on the image in Place interaction mode, then fiducials will be added to the current fiducial list.
  int SetCurrentFiducialList(vtkMRMLFiducialListNode* fidNode);

  //ETX

  void UpdateTargetListFromMRML();

  // Description:
  // Set Slicers's 2D view orientations from the image orientation.
  void SetSliceViewFromVolume(vtkMRMLVolumeNode *volumeNode);
  
 protected:

  //BTX
  std::string GetFoRStrFromVolumeNodeID(const char* volNodeID);
  //ETX

  // Description:
  // Link targets to fiducials (when no FiducialIDs are available), based on fiducial position and label
  void LinkTargetsToFiducials();

  // Description:
  // Helper method for loading a volume via the Volume module.
  vtkMRMLScalarVolumeNode *AddArchetypeVolume(const char* fileName, const char *volumeName);  

  void UpdateAll();

  vtkLITTPlanLogic();
  ~vtkLITTPlanLogic();
  vtkLITTPlanLogic(const vtkLITTPlanLogic&);
  void operator=(const vtkLITTPlanLogic&);
  
  static void DataCallback(vtkObject*, unsigned long, void *, void *);

  /*
  void UpdateAll();
  void UpdateSliceDisplay();
  void UpdateLocator();
  */

  vtkCallbackCommand *DataCallbackCommand;
  
 private:
  
  int GetTargetIndexFromFiducialID(const char* fiducialID);

  int CreateCoverageVolume();
  void DeleteCoverageVolume();
  int UpdateCoverageVolumeImage();

  vtkMRMLRobotNode* GetRobotNode();

  vtkLITTPlanGUI* GUI;

  /*
  bool  Connected;
  bool  RealtimeImageUpdate;
  */

  int   TimerOn;

};

#endif


  
