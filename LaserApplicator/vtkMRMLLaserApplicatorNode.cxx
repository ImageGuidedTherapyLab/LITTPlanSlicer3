/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLLaserApplicatorNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLLaserApplicatorNode.h"

#include "vtkObjectFactory.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLScene.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "vtkMRMLModelNode.h"
#include "vtkCylinderSource.h"
#include "vtkTubeFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkAppendPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkSmartPointer.h"
#include "vtkMath.h"
#include "vtkContourFilter.h"
#include "vtkPointData.h"
#include "vtkAppendPolyData.h"
#include "vtkSphereSource.h"
#include "vtkGlyph3D.h"
#include "vtkPoints.h"
#include "vtkPointSource.h"
#include "vtkSTLReader.h"

/* copied from Applications/CLI/PolyDataToLabelmap.cxx */
#include "itkImage.h"
#include "itkOrientedImage.h"
//#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
//
//#include "vtkXMLPolyDataReader.h"
#include "vtkPolyDataPointSampler.h"
//#include "vtkDebugLeaks.h"
//
//#include "itkPluginUtilities.h"
//#include "PolyDataToLabelmapCLP.h"

#include "itkVTKImageToImageFilter.h"
#include "vtkLITTPlanTargetDescriptor.h"

//static const char MARKER_LABEL_NAMES[CALIB_MARKER_COUNT][9]={ "    Tip ", 
//                                                              "   Entry",
//                                                              "NotUsed ",
//                                                              "NotUsed "};
static const char MARKER_LABEL_NAMES[CALIB_MARKER_COUNT][9]={ "    Tip ", 
                                                              "   Entry" };

typedef itk::OrientedImage< unsigned char, 3 > LabelImageType;

//------------------------------------------------------------------------------
vtkMRMLLaserApplicatorNode* vtkMRMLLaserApplicatorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLLaserApplicatorNode");
  if(ret)
    {
      return (vtkMRMLLaserApplicatorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLLaserApplicatorNode;
}

//----------------------------------------------------------------------------
vtkMRMLLaserApplicatorNode* vtkMRMLLaserApplicatorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLLaserApplicatorNode");
  if(ret)
    {
      return (vtkMRMLLaserApplicatorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLLaserApplicatorNode;
}

//----------------------------------------------------------------------------
vtkMRMLLaserApplicatorNode::vtkMRMLLaserApplicatorNode()
{
  // Calibration inputs

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    this->MarkerSegmentationThreshold[i]=9.0;
  } 

  this->MarkerDimensionsMm[0]=8.0;
  this->MarkerDimensionsMm[1]=5.0;
  this->MarkerDimensionsMm[2]=5.0;

  this->MarkerRadiusMm=3.5;
  this->RobotInitialAngle=0.0;

  this->EnableAutomaticMarkerCenterpointAdjustment=true;  

  // Robot model

  this->ModelAxes=vtkPolyData::New();
  this->ModelProbe=vtkPolyData::New();
  this->ModelNeedle=vtkPolyData::New();
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {    
    this->ModelMarkers[i]=vtkPolyData::New();
  } 

  this->ModelAxesVisible=false;
    
  this->RobotModelNodeRef=NULL;

  this->WorkspaceModelNodeRef=NULL;

  CalibrationStatusDescription=NULL;

  this->CalibrationPointListNodeID = NULL;
  this->CalibrationPointListNode = NULL;

  this->CalibrationVolumeNodeID = NULL;
  this->CalibrationVolumeNode = NULL;

  // Calibration output
  ResetCalibrationData();
}

//----------------------------------------------------------------------------
vtkMRMLLaserApplicatorNode::~vtkMRMLLaserApplicatorNode()
{
  SetRobotModelNodeRef(NULL);
  SetWorkspaceModelNodeRef(NULL);
    
  if (GetCalibrationPointListNode()!=NULL)
  {
    GetCalibrationPointListNode()->SetAndObserveTransformNodeID(NULL);  
  }
  SetAndObserveCalibrationPointListNodeID( NULL );

  SetAndObserveCalibrationVolumeNodeID( NULL );

  SetCalibrationStatusDescription(NULL);

  if (this->ModelAxes!=NULL)
  {
    this->ModelAxes->Delete();
    this->ModelAxes=NULL;
  }
  if (this->ModelProbe!=NULL)
  {
    this->ModelProbe->Delete();
    this->ModelProbe=NULL;
  }
  if (this->ModelNeedle!=NULL)
  {
    this->ModelNeedle->Delete();
    this->ModelNeedle=NULL;
  }
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    if (this->ModelMarkers[i]!=NULL)
    {
      this->ModelMarkers[i]->Delete();
      this->ModelMarkers[i]=NULL;
    }
  }

}

void vtkMRMLLaserApplicatorNode::RemoveChildNodes()
{
  Superclass::RemoveChildNodes();

  vtkMRMLModelNode *modelNode=GetRobotModelNode();    
  if (modelNode!=NULL && this->GetScene()!=NULL)
  {
    vtkMRMLDisplayNode *dispNode=modelNode->GetDisplayNode();
    modelNode->SetAndObserveDisplayNodeID(NULL);
    this->GetScene()->RemoveNode(modelNode);
    this->GetScene()->RemoveNode(dispNode);
  }

  modelNode=GetWorkspaceModelNode();    
  if (modelNode!=NULL && this->GetScene()!=NULL)
  {
    vtkMRMLDisplayNode *dispNode=modelNode->GetDisplayNode();
    modelNode->SetAndObserveDisplayNodeID(NULL);
    this->GetScene()->RemoveNode(modelNode);
    this->GetScene()->RemoveNode(dispNode);
  }
  
  vtkMRMLFiducialListNode *fidNode=GetCalibrationPointListNode();
  if (fidNode!=NULL)
  {
    fidNode->SetAndObserveTransformNodeID(NULL);  
  }
  if (fidNode!=NULL && this->GetScene()!=NULL)
  {
    this->GetScene()->RemoveNode(fidNode);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);  

  // Calibration inputs

  if ( this->CalibrationVolumeNodeID )
  {
    of << indent << " CalibrationVolumeRef=\"" << this->CalibrationVolumeNodeID << "\"";
  }

  if (this->CalibrationPointListNodeID != NULL) 
  {
    of << indent << " CalibrationPointListNodeRef=\"" << this->CalibrationPointListNodeID << "\"";
  }
  
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    of << indent << " MarkerSegmentationThreshold" << i << "=\"" << this->MarkerSegmentationThreshold[i] << "\" ";
  } 

  of << indent << " MarkerDimensionsMm=\"" 
      << this->MarkerDimensionsMm[0] << " " 
      << this->MarkerDimensionsMm[1] << " " 
      << this->MarkerDimensionsMm[2] << "\" "; 

  of << indent << " MarkerRadiusMm=\"" << this->MarkerRadiusMm << "\" "; 
  of << indent << " RobotInitialAngle=\"" << this->RobotInitialAngle << "\" "; 
  
  of << indent << " EnableAutomaticMarkerCenterpointAdjustment=\"" << (this->EnableAutomaticMarkerCenterpointAdjustment?1:0) << "\" "; 

  // Calibration outputs

  of << indent << " Cal_CalibrationValid=\"" << (this->CalibrationData.CalibrationValid?1:0) << "\" "; 
  of << indent << " Cal_AxesDistance=\"" << this->CalibrationData.AxesDistance << "\" "; 
  of << indent << " Cal_RobotRegistrationAngleDegrees=\"" << this->CalibrationData.RobotRegistrationAngleDegrees << "\" "; 
  of << indent << " Cal_AxesAngleDegrees=\"" << this->CalibrationData.AxesAngleDegrees << "\" "; 
  of << indent << " Cal_I1=\"" << this->CalibrationData.I1[0] << " " << this->CalibrationData.I1[1] << " " << this->CalibrationData.I1[2] << "\" "; 
  of << indent << " Cal_I2=\"" << this->CalibrationData.I2[0] << " " << this->CalibrationData.I2[1] << " " << this->CalibrationData.I2[2] << "\" "; 
  of << indent << " Cal_v1=\"" << this->CalibrationData.v1[0] << " " << this->CalibrationData.v1[1] << " " << this->CalibrationData.v1[2] << "\" "; 
  of << indent << " Cal_v2=\"" << this->CalibrationData.v2[0] << " " << this->CalibrationData.v2[1] << " " << this->CalibrationData.v2[2] << "\" "; 
  of << indent << " Cal_FoR=\"" << this->CalibrationData.FoR << "\" "; 

  // Robot model
  
  of << indent << " ModelAxesVisible=\"" << (this->ModelAxesVisible?1:0) << "\" "; 

  if ( this->RobotModelNodeRef )
  {
    of << indent << " RobotModelNodeRef=\"" << this->RobotModelNodeRef << "\"";
  }
  if ( this->WorkspaceModelNodeRef )
  {
    of << indent << " WorkspaceModelNodeRef=\"" << this->WorkspaceModelNodeRef << "\"";
  }
}


//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
  {
    attName = *(atts++);
    attValue = *(atts++);

    // Calibration inputs
    if (!strcmp(attName, "CalibrationVolumeRef"))
    {
      this->SetAndObserveCalibrationVolumeNodeID(NULL);
      this->SetCalibrationVolumeNodeID(attValue);
    }
    if (!strcmp(attName, "CalibrationPointListNodeRef")) 
    {
      this->SetAndObserveCalibrationPointListNodeID(NULL);
      this->SetCalibrationPointListNodeID(attValue);
    }
    for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {
      {
        std::ostrstream strvalue;
        strvalue << "MarkerSegmentationThreshold" << i << std::ends;        
        if (!strcmp(attName, strvalue.str()))
        {
          std::stringstream ss;
          ss << attValue;
          ss >> this->MarkerSegmentationThreshold[i];
        }
        strvalue.rdbuf()->freeze(0);
      }
    }
    if (!strcmp(attName, "MarkerDimensionsMm"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MarkerDimensionsMm[0];
      ss >> this->MarkerDimensionsMm[1];
      ss >> this->MarkerDimensionsMm[2];
    }
    else if (!strcmp(attName, "MarkerRadiusMm"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MarkerRadiusMm;
    }
    else if (!strcmp(attName, "RobotInitialAngle"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->RobotInitialAngle;
    }
    else if (!strcmp(attName, "EnableAutomaticMarkerCenterpointAdjustment"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->EnableAutomaticMarkerCenterpointAdjustment;
    }

    // Calibration outputs
    if (!strcmp(attName, "Cal_CalibrationValid"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.CalibrationValid;
    }
    else if (!strcmp(attName, "Cal_AxesDistance"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.AxesDistance;
    }
    else if (!strcmp(attName, "Cal_AxesAngleDegrees"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.AxesAngleDegrees;
    }
    else if (!strcmp(attName, "Cal_CalibrationValid"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.CalibrationValid;
    }
    else if (!strcmp(attName, "Cal_I1"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.I1[0];
      ss >> this->CalibrationData.I1[1];
      ss >> this->CalibrationData.I1[2];
    }
    else if (!strcmp(attName, "Cal_I2"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.I2[0];
      ss >> this->CalibrationData.I2[1];
      ss >> this->CalibrationData.I2[2];
    }
    else if (!strcmp(attName, "Cal_v1"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.v1[0];
      ss >> this->CalibrationData.v1[1];
      ss >> this->CalibrationData.v1[2];
    }
    else if (!strcmp(attName, "Cal_v2"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CalibrationData.v2[0];
      ss >> this->CalibrationData.v2[1];
      ss >> this->CalibrationData.v2[2];
    }
    else if (!strcmp(attName, "Cal_FoR"))
    {      
      this->CalibrationData.FoR=attValue;     
    }

    // Robot model
    if (!strcmp(attName, "ModelAxesVisible"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ModelAxesVisible;
    }
    else if (!strcmp(attName, "RobotModelNodeRef"))
    {
      this->SetRobotModelNodeRef(attValue);
    }
    else if (!strcmp(attName, "WorkspaceModelNodeRef"))
    {
      this->SetWorkspaceModelNodeRef(attValue);
    }

  }
}
//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  
  // Calibration inputs

  os << indent << "CalibrationVolumeNodeID:   " << 
   (this->CalibrationVolumeNodeID ? this->CalibrationVolumeNodeID : "(none)") << "\n";
  
 os << indent << "CalibrationPointListNodeID: " <<
    (this->CalibrationPointListNodeID? this->CalibrationPointListNodeID: "(none)") << "\n";

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    os << indent << " MarkerSegmentationThreshold" << i << ": " << (this->MarkerSegmentationThreshold[i]?1:0) << "\n";
  } 

  os << indent << " MarkerDimensionsMm:" 
      << this->MarkerDimensionsMm[0] << " " 
      << this->MarkerDimensionsMm[1] << " " 
      << this->MarkerDimensionsMm[2] << "\n"; 

  os << indent << " MarkerRadiusMm: " << this->MarkerRadiusMm << "\n"; 
  os << indent << " RobotInitialAngle: " << this->RobotInitialAngle << "\n"; 
  
  os << indent << " EnableAutomaticMarkerCenterpointAdjustment: " << (this->EnableAutomaticMarkerCenterpointAdjustment?1:0) << "\n"; 

  // Calibration outputs

  os << indent << " Cal_CalibrationValid: " << (this->CalibrationData.CalibrationValid?1:0) << "\n"; 
  os << indent << " Cal_AxesDistance: " << this->CalibrationData.AxesDistance << "\n"; 
  os << indent << " Cal_RobotRegistrationAngleDegrees: " << this->CalibrationData.RobotRegistrationAngleDegrees << "\n"; 
  os << indent << " Cal_AxesAngleDegrees: " << this->CalibrationData.AxesAngleDegrees << "\n"; 
  os << indent << " Cal_I1: " << this->CalibrationData.I1[0] << " " << this->CalibrationData.I1[1] << " " << this->CalibrationData.I1[2] << "\n"; 
  os << indent << " Cal_I2: " << this->CalibrationData.I2[0] << " " << this->CalibrationData.I2[1] << " " << this->CalibrationData.I2[2] << "\n"; 
  os << indent << " Cal_v1: " << this->CalibrationData.v1[0] << " " << this->CalibrationData.v1[1] << " " << this->CalibrationData.v1[2] << "\n"; 
  os << indent << " Cal_v2: " << this->CalibrationData.v2[0] << " " << this->CalibrationData.v2[1] << " " << this->CalibrationData.v2[2] << "\n"; 
  os << indent << " Cal_FoR: " << this->CalibrationData.FoR << "\n"; 

  // Robot model
  
  os << indent << " ModelAxesVisible: " << (this->ModelAxesVisible?1:0) << "\n"; 

  os << indent << "RobotModelNodeRef:   " << 
   (this->RobotModelNodeRef ? this->RobotModelNodeRef : "(none)") << "\n";

  os << indent << "WorkspaceModelNodeRef:   " << 
   (this->WorkspaceModelNodeRef ? this->WorkspaceModelNodeRef : "(none)") << "\n";

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLLaserApplicatorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLLaserApplicatorNode *node = (vtkMRMLLaserApplicatorNode *) anode;

  // Calibration input

  this->SetAndObserveCalibrationVolumeNodeID( NULL );
  this->SetCalibrationVolumeNodeID( node->CalibrationVolumeNodeID );

  this->SetAndObserveCalibrationPointListNodeID( NULL );
  this->SetCalibrationPointListNodeID( node->CalibrationPointListNodeID );

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    this->MarkerSegmentationThreshold[i]=node->MarkerSegmentationThreshold[i];
  } 

  this->MarkerDimensionsMm[0]=node->MarkerDimensionsMm[0];
  this->MarkerDimensionsMm[1]=node->MarkerDimensionsMm[1];
  this->MarkerDimensionsMm[2]=node->MarkerDimensionsMm[2];

  this->MarkerRadiusMm=node->MarkerRadiusMm;
  this->RobotInitialAngle=node->RobotInitialAngle;

  this->EnableAutomaticMarkerCenterpointAdjustment=node->EnableAutomaticMarkerCenterpointAdjustment;

  // Calibration output

  this->CalibrationData=node->CalibrationData;

  // Robot model

  this->ModelAxes->DeepCopy(node->ModelAxes);
  this->ModelProbe->DeepCopy(node->ModelProbe);
  this->ModelNeedle->DeepCopy(node->ModelProbe);
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {    
    this->ModelMarkers[i]->DeepCopy(node->ModelMarkers[i]);
  } 

  this->ModelAxesVisible=node->ModelAxesVisible;
    
  this->SetRobotModelNodeRef(node->RobotModelNodeRef);  

  this->SetWorkspaceModelNodeRef(node->WorkspaceModelNodeRef);  
}

//----------------------------------------------------------------------------
int vtkMRMLLaserApplicatorNode::Init(vtkSlicerApplication* app, const char* moduleShareDir)
{ 
  this->Superclass::Init(app, moduleShareDir);

  // This part should be moved to Robot Display Node.
  // Robot model
  if (GetRobotModelNode()==NULL)
  {
    const char* nodeID = AddModelNode("LaserApplicatorModel", 0.5, 0.5, 1.0);
    this->SetRobotModelNodeRef(nodeID);
  }
  // Workspace model
  if (GetWorkspaceModelNode()==NULL)
  {
    const char* nodeID = AddModelNode("LaserApplicatorDamageModel", 0.2, 0.8, 0.2);
    this->SetWorkspaceModelNodeRef(nodeID);
  }

  if (GetCalibrationPointListNode()==NULL && this->Scene!=NULL)
  {
    // the fiducial node hasn't been created yet
    vtkSmartPointer<vtkMRMLFiducialListNode> newCalibPointListNode=vtkSmartPointer<vtkMRMLFiducialListNode>::New();
    newCalibPointListNode->SetName("CalibrationMarkers");
    this->Scene->AddNode(newCalibPointListNode); // don't update scene until ref set in robot node
    SetAndObserveCalibrationPointListNodeID(newCalibPointListNode->GetID());
  }

  return 1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLLaserApplicatorNode::AddModelNode(const char* nodeName, double colorR, double colorG, double colorB)
{  
  vtkSmartPointer<vtkMRMLModelDisplayNode> displayNode  = vtkSmartPointer<vtkMRMLModelDisplayNode>::New(); 
  displayNode->SetScene(this->Scene);
  displayNode->SetColor(colorR,colorG,colorB);
  displayNode->SetOpacity(0.5);
  displayNode->VisibilityOff();
  displayNode->SliceIntersectionVisibilityOff();
  this->Scene->AddNode(displayNode);

  vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
  modelNode->SetName(nodeName);
  modelNode->SetScene(this->Scene);
  modelNode->SetHideFromEditors(0);
  modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  this->Scene->AddNode(modelNode);

  return modelNode->GetID(); // model is added to the scene, so the GetID string remains valid
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLLaserApplicatorNode::GetRobotModelNode()
{
  if (this->GetScene() && this->RobotModelNodeRef != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->RobotModelNodeRef));
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLLaserApplicatorNode::GetWorkspaceModelNode()
{
  if (this->GetScene() && this->WorkspaceModelNodeRef != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->WorkspaceModelNodeRef));
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Fiducial marker positions changed
  if (caller == (vtkObject*)this->CalibrationPointListNode)
  {
    switch (event)
    {
    case vtkCommand::ModifiedEvent:
    case vtkMRMLScene::NodeAddedEvent: // when a fiducial is added to the list
    case vtkMRMLFiducialListNode::FiducialModifiedEvent:
    case vtkMRMLFiducialListNode::DisplayModifiedEvent:
    case vtkMRMLScene::NodeRemovedEvent:
      UpdateCalibration();
      break;
    }
  }

  if (caller == (vtkObject*)this->CalibrationVolumeNode)
  {
    switch (event)
    {
    case vtkCommand::ModifiedEvent:
      UpdateCalibration();
      break;
    }
  }

  return;
}

bool vtkMRMLLaserApplicatorNode::GetNeedleDirectionAtTarget(vtkLITTPlanTargetDescriptor *targetDesc, NeedleDescriptorStruct *needle, double* needleDirection)
{
  // :TODO: update this for arbitrary target and calib volume transform  
  if (needleDirection==NULL)
  {
    // error computing targeting params
    vtkErrorMacro("GetNeedleDirectionAtTarget needleDirection pointer in invalid");
    return false;
  }
  TRLaserAblationTargetingParams targetingParams;
  bool valid=vtkLaserApplicatorFiducialCalibrationAlgo::FindTargetingParams(targetDesc, this->CalibrationData, needle, &targetingParams);
  if (!valid || !targetingParams.TargetingParametersValid)
  {
    // error computing targeting params
    return false;
  }
  
  double targetRAS[3];
  targetDesc->GetRASLocation(targetRAS);

  needleDirection[0] = targetRAS[0] - targetingParams.HingePosition[0];
  needleDirection[1] = targetRAS[1] - targetingParams.HingePosition[1];
  needleDirection[2] = targetRAS[2] - targetingParams.HingePosition[2];
  vtkMath::Normalize(needleDirection);

  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLLaserApplicatorNode::GetTargetInfoText(vtkLITTPlanTargetDescriptor *targetDesc, NeedleDescriptorStruct *needle)
{
  TRLaserAblationTargetingParams targetingParams;  
  bool validTargeting=vtkLaserApplicatorFiducialCalibrationAlgo::FindTargetingParams(targetDesc, this->CalibrationData, needle, &targetingParams);

  std::ostrstream os;    

  // Put most important information in the first section
  if (validTargeting && targetingParams.TargetingParametersValid)
  {
    if (this->CalibrationData.FoR.compare(targetDesc->GetTargetingVolumeFoR())!=0)
    {
      os << "Warning: frame of reference id mismatch"<<std::endl;
    }
    if (targetingParams.IsOutsideReach)
    {
      os << "Warning: the target is not reachable"<<std::endl;
    }    
    os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(1);
    os << "Device rotation (deg): ";
    // Display a + sign explicitly for positive rotation angles
    if (targetingParams.AxisRotation>0)
    {
      os << "+";
    }
    os << targetingParams.AxisRotation<<std::endl;    
    os << "Needle angle (deg): "<<targetingParams.NeedleAngle<<std::endl;
    os << "Depth (cm): "<<targetingParams.DepthCM<<std::endl;       
  }
  
  // Put less important information in the next section
  os << GetTargetInfoSectionSeparator();
  os << "Target: " << targetDesc->GetName()<<std::endl;
  os << "Needle type: " << needle->mDescription<<std::endl;
  os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(1);
  os << targetDesc->GetRASLocationString().c_str()<<std::endl;
  os << std::ends;
  
  std::string result=os.str();
  os.rdbuf()->freeze();
  return result;
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::RemoveAllCalibrationMarkers()
{
  vtkMRMLFiducialListNode *calibrationPointListNode = this->GetCalibrationPointListNode();
  if (calibrationPointListNode==NULL)
  {
    return;
  }

  calibrationPointListNode->RemoveAllFiducials();
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::SetCalibrationInputs(const char *calibVolRef,
  double thresh[CALIB_MARKER_COUNT], double fidDimsMm[3], double radiusMm, 
  double initialAngle, bool enableAutomaticCenterpointAdjustment)
{

  // Update the node

  bool modified=false;

  if (calibVolRef!=NULL && this->CalibrationVolumeNodeID!=NULL)
  {
    if (strcmp(calibVolRef,this->CalibrationVolumeNodeID))
    {
      modified=true;
      SetAndObserveCalibrationVolumeNodeID(calibVolRef);      
    }
  }
  else
  {
    if (calibVolRef!=this->CalibrationVolumeNodeID)
    {
      modified=true;
      SetAndObserveCalibrationVolumeNodeID(calibVolRef);      
    }
  }

  for (unsigned int markerInd=0; markerInd<CALIB_MARKER_COUNT; markerInd++)
  {       
    if (this->MarkerSegmentationThreshold[markerInd]!=thresh[markerInd])
    {
      modified=true;
      this->MarkerSegmentationThreshold[markerInd]=thresh[markerInd];
    }
  }
  
  for (int j=0; j<3; j++)
  {
    if (this->MarkerDimensionsMm[j]!=fidDimsMm[j])
    {
      modified=true;
      this->MarkerDimensionsMm[j]=fidDimsMm[j];
    }
  }

  if (this->MarkerRadiusMm!=radiusMm)
  {
    modified=true;
    this->MarkerRadiusMm=radiusMm;
  }
  
  if (this->RobotInitialAngle!=initialAngle)
  {
    modified=true;
    this->RobotInitialAngle=initialAngle;
  }

  if (this->EnableAutomaticMarkerCenterpointAdjustment!=enableAutomaticCenterpointAdjustment)
  {
    modified=true;
    this->EnableAutomaticMarkerCenterpointAdjustment=enableAutomaticCenterpointAdjustment;
  }
  
  if (modified)
  {
    Modified();
    UpdateCalibration();    
  }
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateCalibration()
{

  vtkMRMLFiducialListNode *calibrationPointListNode = this->GetCalibrationPointListNode();
  if (calibrationPointListNode==NULL)
  {
    // cannot update the calibration data because the scene has is not been fully loaded yet
    // so just use the calibration data that was saved with the scene - do not reset!
    if (this->CalibrationData.CalibrationValid)
    {
      SetCalibrationStatusDescription("Using saved calibration data");
    }
    else
    {
      ResetCalibrationData();
    }
    return;
  }

  // Normalize markers (ensure proper name and count)
  int fidCount=calibrationPointListNode->GetNumberOfFiducials();
  for (int i=fidCount-1; i>=int(CALIB_MARKER_COUNT); i--)
  {
    // this is an extra marker => delete it
    calibrationPointListNode->RemoveFiducial(i);
  }
  fidCount=calibrationPointListNode->GetNumberOfFiducials();
  for (unsigned int i=0; i<CALIB_MARKER_COUNT && i<static_cast<unsigned int>(fidCount); i++)
  {
    if (strcmp(calibrationPointListNode->GetNthFiducialLabelText(i),MARKER_LABEL_NAMES[i])!=0)
    {
      calibrationPointListNode->SetNthFiducialLabelText(i,MARKER_LABEL_NAMES[i]);      
    }
  }

  TRLaserAblationCalibrationFromImageInput in; // struct that stores all inputs required for the calib algo

  vtkMRMLScalarVolumeNode *calVolNode=GetCalibrationVolumeNode();
  if (calVolNode==0)
  {    
    SetCalibrationStatusDescription("Calibration volume is not available");
    ResetCalibrationData();
    return;
  }

  if (calVolNode->GetImageData()==NULL)
  {
    // cannot update the calibration data because the scene has is not been fully loaded yet
    // so just use the calibration data that was saved with the scene - do not reset!
    if (this->CalibrationData.CalibrationValid)
    {
      SetCalibrationStatusDescription("Using saved calibration data");
    }
    else
    {
      ResetCalibrationData();
    }
    return;
  }

  in.VolumeImageData=calVolNode->GetImageData();

  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New(); 
  calVolNode->GetIJKToRASMatrix(ijkToRAS);
  in.VolumeIJKToRASMatrix=ijkToRAS;  

  // get frame of reference uid
  const itk::MetaDataDictionary &volDictionary = calVolNode->GetMetaDataDictionary();
  std::string tagValue; 
  tagValue.clear();
  itk::ExposeMetaData<std::string>( volDictionary, "0020|0052", tagValue );   

  // Need to remove extra '0x00' characters. By assigning the value using c_str we achieve just this.
  in.FoR=tagValue.c_str();

  if (calibrationPointListNode->GetNumberOfFiducials()<(int)CALIB_MARKER_COUNT)
  {
    SetCalibrationStatusDescription("Not all calibration markers are defined");
    ResetCalibrationData();
    return;
  }

  // Compute transform matrix from calibrationPointListRas to calVolNodeRas
  vtkSmartPointer<vtkMatrix4x4> transform_calPointRas2calVolRas=vtkSmartPointer<vtkMatrix4x4>::New();
  transform_calPointRas2calVolRas->Identity();
  {
    vtkMRMLTransformNode* calPointTransformNode = calibrationPointListNode->GetParentTransformNode();
    vtkMRMLTransformNode* calVolTransformNode = calVolNode->GetParentTransformNode();  
    if (calPointTransformNode!=NULL)
    {
      calPointTransformNode->GetMatrixTransformToNode(calVolTransformNode, transform_calPointRas2calVolRas);
    }
    else if (calVolTransformNode!=NULL)
    {
      calVolTransformNode->GetMatrixTransformToWorld(transform_calPointRas2calVolRas);
      transform_calPointRas2calVolRas->Invert();
    }
    else
    {
      transform_calPointRas2calVolRas->Identity();
    }
  }

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    
    float* rasPoint=calibrationPointListNode->GetNthFiducialXYZ(i);
    if (rasPoint==NULL)
    {
      SetCalibrationStatusDescription("Not all calibration markers are defined");
      ResetCalibrationData();
      return;
    }

    double point_calPointRas[4]={0,0,0,1};
    double point_calVolRas[4]={0,0,0,1};
    point_calPointRas[0]=rasPoint[0];
    point_calPointRas[1]=rasPoint[1];
    point_calPointRas[2]=rasPoint[2];    
    transform_calPointRas2calVolRas->MultiplyPoint(point_calPointRas,point_calVolRas);
    in.MarkerInitialPositions[i][0]=point_calVolRas[0];
    in.MarkerInitialPositions[i][1]=point_calVolRas[1];
    in.MarkerInitialPositions[i][2]=point_calVolRas[2];  

    in.MarkerSegmentationThreshold[i]=this->MarkerSegmentationThreshold[i];
  }

  for (int i=0; i<3; i++)
  {
    in.MarkerDimensionsMm[i]=this->MarkerDimensionsMm[i];
  }
  in.MarkerRadiusMm=this->MarkerRadiusMm;
  in.RobotInitialAngle=this->RobotInitialAngle;
   
  vtkSmartPointer<vtkLaserApplicatorFiducialCalibrationAlgo> calibrationAlgo=vtkSmartPointer<vtkLaserApplicatorFiducialCalibrationAlgo>::New();

  calibrationAlgo->SetEnableMarkerCenterpointAdjustment(this->EnableAutomaticMarkerCenterpointAdjustment);
  calibrationAlgo->SetEnableMarkerCenterpointAdjustment(false);

  if (!calibrationAlgo->CalibrateFromImage(in))
  {
    // calibration failed    

    std::ostrstream os;
    // if a marker not found, then make the error report more precise
    for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {      
      if (!calibrationAlgo->GetMarkerFound(i))
      {
        os << "Marker "<<i+1<<" cannot be detected. ";
      }
    } 
    os << "Calibration failed." << std::ends;
    SetCalibrationStatusDescription(os.str());
    os.rdbuf()->freeze();
    
    ResetCalibrationData();
    return;
  }

  // calibration is successful

  const TRLaserAblationCalibrationData calibData=calibrationAlgo->GetCalibrationData();  
  
  this->CalibrationData=calibData;

  // Take into account potential transform between robot node and calibration image
  vtkSmartPointer<vtkMatrix4x4> transform_calVol2robotRas=vtkSmartPointer<vtkMatrix4x4>::New();
  transform_calVol2robotRas->Identity();
  {
    vtkMRMLTransformNode* robotTransformNode = this->GetParentTransformNode();
    vtkMRMLTransformNode* calVolTransformNode = calVolNode->GetParentTransformNode();  
    if (calVolTransformNode!=NULL)
    {
      calVolTransformNode->GetMatrixTransformToNode(robotTransformNode, transform_calVol2robotRas);
      std::cout << "vtkMRMLLaserApplicatorNode::UpdateCalibration calVolTransformNode!=NULL" 
                << std::endl;
    }
    else if (robotTransformNode!=NULL)
    {
      robotTransformNode->GetMatrixTransformToWorld(transform_calVol2robotRas);
      transform_calVol2robotRas->Invert();
      std::cout << "vtkMRMLLaserApplicatorNode::UpdateCalibration robotTransformNode !=NULL" 
                << std::endl;

    }
    else
    {
      transform_calVol2robotRas->Identity();
    }
  }
  TransformCalibrationData(transform_calVol2robotRas);  

  UpdateRobotModelAxes();
  UpdateRobotModelProbe();
  vtkMatrix4x4* preProcIjkToRas=calibrationAlgo->GetCalibMarkerPreProcOutputIJKToRAS();
  for (unsigned int markerId=0; markerId<CALIB_MARKER_COUNT; markerId++)
  {      
    UpdateRobotModelMarker(markerId, calibrationAlgo->GetCalibMarkerPreProcOutput(markerId), preProcIjkToRas);    
  }
  TransformRobotModelMarkers(transform_calVol2robotRas);
  UpdateRobotModelNeedle(NULL, NULL);
  UpdateRobotModel();

  UpdateWorkspaceModel();

  SetCalibrationStatusDescription("Congratulations! Calibration is successfully completed.");

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::ResetCalibrationData()
{
  bool modified=false;

  if (this->CalibrationData.CalibrationValid)
  {
    modified=true;
    this->CalibrationData.CalibrationValid=false;
  }

  this->CalibrationData.AxesDistance = 0;
  this->CalibrationData.AxesAngleDegrees = 0;
  this->CalibrationData.RobotRegistrationAngleDegrees = 0;
  for (int i=0; i<3; i++)
  {
    this->CalibrationData.I1[i]=0.0;
    this->CalibrationData.I2[i]=0.0;
    this->CalibrationData.v1[i]=0.0;
    this->CalibrationData.v2[i]=0.0;
  }
    
  UpdateRobotModelAxes();
  UpdateRobotModelProbe();  
  for (unsigned int markerId=0; markerId<CALIB_MARKER_COUNT; markerId++)
  {      
    UpdateRobotModelMarker(markerId, NULL, NULL);
  }
  UpdateRobotModelNeedle(NULL, NULL);
  UpdateRobotModel();


  this->ModelAxes->Reset();

  if (modified)
  {
    Modified();
  }
}


//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateWorkspaceModel()
{
  vtkMRMLModelNode* modelNode=GetWorkspaceModelNode();
  if (modelNode==NULL)
  {
    return;
  }
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode==NULL)
  {
    return;
  }      

  if (!this->CalibrationData.CalibrationValid)
  {
    // no calibration robot position is unknown
    displayNode->VisibilityOff();
    displayNode->SliceIntersectionVisibilityOff();  
    return;
  }

  vtkSmartPointer <vtkSTLReader> modelReader=vtkSmartPointer<vtkSTLReader>::New();

  vtksys_stl::string
modelFileName=this->ModuleShareDirectory+"/LaserApplicator/damageTemplate.stl";
  modelReader->SetFileName(modelFileName.c_str());
  modelReader->Update();

  vtkSmartPointer<vtkTransformPolyDataFilter> polyTrans = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  polyTrans->SetInputConnection(modelReader->GetOutputPort());
  vtkSmartPointer<vtkTransform> modelTransform=vtkSmartPointer<vtkTransform>::New();
  polyTrans->SetTransform(modelTransform);

  GetRobotBaseTransform(modelTransform->GetMatrix());

  /*

  vtkMatrix4x4 *transform=modelTransform->GetMatrix();

  double v1norm[3]={this->CalibrationData.v1[0], this->CalibrationData.v1[1], this->CalibrationData.v1[2]};         
  vtkMath::Normalize(v1norm);
  double v2norm[3]={this->CalibrationData.v2[0], this->CalibrationData.v2[1], this->CalibrationData.v2[2]};         
  vtkMath::Normalize(v2norm);

  double x[3]={v1norm[0],v1norm[1],v1norm[2]};
  double y[3]={0,0,0};
  vtkMath::Cross(x, v2norm, y);
  vtkMath::Normalize(y);
  double z[3]={0,0,0};
  vtkMath::Cross(x, y, z);
  vtkMath::Normalize(z);

  int col=0;

  // orientation
  col=2; // z (orange)
  transform->SetElement(0,col, x[0]);
  transform->SetElement(1,col, x[1]);
  transform->SetElement(2,col, x[2]);

  col=0; // x (orange)
  transform->SetElement(0,col, y[0]);
  transform->SetElement(1,col, y[1]);
  transform->SetElement(2,col, y[2]);

  col=1; // y (orange)
  transform->SetElement(0,col, z[0]);
  transform->SetElement(1,col, z[1]);
  transform->SetElement(2,col, z[2]);

  // Hinge point
  double l=14.5/sin(CalibrationData.AxesAngleDegrees * vtkMath::Pi()/180);

  double H_before[3];
  H_before[0] = CalibrationData.I1[0] - l*CalibrationData.v2[0];
  H_before[1] = CalibrationData.I1[1] - l*CalibrationData.v2[1];
  H_before[2] = CalibrationData.I1[2] - l*CalibrationData.v2[2];

  // position
  
  transform->SetElement(0,3, H_before[0]);
  transform->SetElement(1,3, H_before[1]);
  transform->SetElement(2,3, H_before[2]);
  */

  vtkSmartPointer<vtkTriangleFilter> cleaner=vtkSmartPointer<vtkTriangleFilter>::New();
  cleaner->SetInputConnection(polyTrans->GetOutputPort());
  cleaner->Update();
  
  modelNode->SetAndObservePolyData(cleaner->GetOutput());
  modelNode->SetModifiedSinceRead(1);

  displayNode->SetModifiedSinceRead(1); 

  // project models onto DICOM and write out
  // TODO still have bug transforming imaging  and
  //      models to same coordinate system...
  //WriteDicomPlanningSeries(cleaner->GetOutput());

  displayNode->SliceIntersectionVisibilityOn();  
  displayNode->VisibilityOn();

}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateRobotModel()
{
  vtkMRMLModelNode* modelNode=GetRobotModelNode();
  if (modelNode==NULL)
  {
    return;
  }
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode==NULL)
  {
    return;
  }      

  // Merge all into a single polydata
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  bool hasPoints=false;

  // Probe
  if (this->ModelProbe->GetNumberOfPoints()>0)
  {
    appender->AddInput(this->ModelProbe);
    hasPoints=true;
  }
  for (unsigned int markerId=0; markerId<2 && markerId<CALIB_MARKER_COUNT; markerId++)
  {
    if (this->ModelMarkers[markerId]->GetNumberOfPoints()>0)
    {
      appender->AddInput(this->ModelMarkers[markerId]);
      hasPoints=true;
    }
  }

  // Calibration axes (and calibration needle)
  if (this->ModelAxesVisible && this->ModelAxes->GetNumberOfPoints()>0)
  {
    if (this->ModelAxes->GetNumberOfPoints()>0)
    {
      appender->AddInput(this->ModelAxes);
      hasPoints=true;
    }
    for (unsigned int markerId=2; markerId<CALIB_MARKER_COUNT; markerId++)
    {
      if (this->ModelMarkers[markerId]->GetNumberOfPoints()>0)
      {
        appender->AddInput(this->ModelMarkers[markerId]);
        hasPoints=true;
      }
    }
  }

  // Targeting needle
  if (this->ModelNeedle->GetNumberOfPoints()>0)
  {
    appender->AddInput(this->ModelNeedle);
    hasPoints=true;
  }

  // If there were no points in the polydata at all, then we would get a VTK warning.  
  if (!hasPoints)
  {
    // There is an existing calibration, just keep the corresponding model
    if (this->CalibrationData.CalibrationValid)
    {
      SetCalibrationStatusDescription("Using saved calibration data");
      return;
    }

    // No calibration, add a dummy point to avoid warning
    vtkSmartPointer<vtkPointSource> point=vtkSmartPointer<vtkPointSource>::New();
    point->SetNumberOfPoints(1);
    appender->AddInputConnection(point->GetOutputPort());
  }

  vtkSmartPointer<vtkTriangleFilter> cleaner=vtkSmartPointer<vtkTriangleFilter>::New();
  cleaner->SetInputConnection(appender->GetOutputPort());
  cleaner->Update();
  
  modelNode->SetAndObservePolyData(cleaner->GetOutput());
  modelNode->SetModifiedSinceRead(1);
  
  /*
  displayNode->SetPolyData(modelNode->GetPolyData());

  if (displayNode->GetPolyData() != NULL) 
  {
    displayNode->GetPolyData()->Modified();
  }
  */

  displayNode->SetModifiedSinceRead(1); 
  
  displayNode->SliceIntersectionVisibilityOn();  
  displayNode->VisibilityOn();

}

//------------------------------------------------------------------------------
bool vtkMRMLLaserApplicatorNode::ShowRobotAtTarget(vtkLITTPlanTargetDescriptor *targetDesc, NeedleDescriptorStruct *needle)
{
  UpdateRobotModelNeedle(targetDesc, needle);
  UpdateRobotModel();
  return true;
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateRobotModelNeedle(vtkLITTPlanTargetDescriptor *targetDesc, NeedleDescriptorStruct *needle)
{
  if (targetDesc==NULL)
  {
    this->ModelNeedle->Reset();
    return;
  }

  TRLaserAblationTargetingParams targetingParams;  
  bool validTargeting=vtkLaserApplicatorFiducialCalibrationAlgo::FindTargetingParams(targetDesc, this->CalibrationData, needle, &targetingParams);

  if (!validTargeting || !targetingParams.TargetingParametersValid)
  {
    // no valid targeting parameters are available
    return;
  }

  // get RAS points of start and end point of needle
  // render the needle as a thin pipe

  // start point is the target RAS
  double targetRAS[3];
  targetDesc->GetRASLocation(targetRAS); 

  double targetHingeRAS[3]={
    targetingParams.HingePosition[0],
    targetingParams.HingePosition[1],
    targetingParams.HingePosition[2]};

  double needleVector[3];
  needleVector[0] = targetRAS[0] - targetHingeRAS[0];
  needleVector[1] = targetRAS[1] - targetHingeRAS[1];
  needleVector[2] = targetRAS[2] - targetHingeRAS[2];
  vtkMath::Normalize(needleVector);

  double targetCenter = needle->GetTargetCenter();
  double needleLength = needle->mLength;

  double needleEndRAS[3];
  needleEndRAS[0] = targetRAS[0] - targetCenter*needleVector[0];
  needleEndRAS[1] = targetRAS[1] - targetCenter*needleVector[1];
  needleEndRAS[2] = targetRAS[2] - targetCenter*needleVector[2];

  double needleStartRAS[3];
  needleStartRAS[0] = needleEndRAS[0] - needleLength*needleVector[0];
  needleStartRAS[1] = needleEndRAS[1] - needleLength*needleVector[1];
  needleStartRAS[2] = needleEndRAS[2] - needleLength*needleVector[2];

  // merge all polydata by this appender
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  vtkSmartPointer<vtkLineSource> NeedleTrajectoryLine=vtkSmartPointer<vtkLineSource>::New();
  NeedleTrajectoryLine->SetResolution(20); 
  NeedleTrajectoryLine->SetPoint1(needleEndRAS);
  NeedleTrajectoryLine->SetPoint2(needleStartRAS);

  vtkSmartPointer<vtkTubeFilter> NeedleTrajectoryTube=vtkSmartPointer<vtkTubeFilter>::New();
  NeedleTrajectoryTube->SetInputConnection(NeedleTrajectoryLine->GetOutputPort());
  NeedleTrajectoryTube->SetRadius(1.0);
  NeedleTrajectoryTube->SetNumberOfSides(16);
  NeedleTrajectoryTube->CappingOn();

  appender->AddInputConnection(NeedleTrajectoryTube->GetOutputPort());  

  // a thicker tube representing the needle target (the core size, in case of a core biopsy)
  double targetLength=needle->mTargetLength;
  if (targetLength>0)
  {
    double targetEndRAS[3];
    targetEndRAS[0] = targetRAS[0] - targetLength/2*needleVector[0];
    targetEndRAS[1] = targetRAS[1] - targetLength/2*needleVector[1];
    targetEndRAS[2] = targetRAS[2] - targetLength/2*needleVector[2];

    double targetStartRAS[3];
    targetStartRAS[0] = targetRAS[0] + targetLength/2*needleVector[0];
    targetStartRAS[1] = targetRAS[1] + targetLength/2*needleVector[1];
    targetStartRAS[2] = targetRAS[2] + targetLength/2*needleVector[2];

    vtkSmartPointer<vtkLineSource> line=vtkSmartPointer<vtkLineSource>::New();
    line->SetResolution(20); 
    line->SetPoint1(targetEndRAS);
    line->SetPoint2(targetStartRAS);

    vtkSmartPointer<vtkTubeFilter> tube=vtkSmartPointer<vtkTubeFilter>::New();
    tube->SetInputConnection(line->GetOutputPort());
    tube->SetRadius(1.5);
    tube->SetNumberOfSides(8);
    tube->CappingOn();

    appender->AddInputConnection(tube->GetOutputPort());
  }

  // a thinner tube representing the needle extension (when the needle is triggered it extends to -extension distance from the needle tip)
  double extension=needle->GetExtension();
  if (extension>0)
  {
    double extensionEndRAS[3];
    extensionEndRAS[0] = needleEndRAS[0];
    extensionEndRAS[1] = needleEndRAS[1];
    extensionEndRAS[2] = needleEndRAS[2];

    double extensionStartRAS[3];
    extensionStartRAS[0] = needleEndRAS[0] + extension*needleVector[0];
    extensionStartRAS[1] = needleEndRAS[1] + extension*needleVector[1];
    extensionStartRAS[2] = needleEndRAS[2] + extension*needleVector[2];

    vtkSmartPointer<vtkLineSource> line=vtkSmartPointer<vtkLineSource>::New();
    line->SetResolution(20); 
    line->SetPoint1(extensionEndRAS);
    line->SetPoint2(extensionStartRAS);

    vtkSmartPointer<vtkTubeFilter> tube=vtkSmartPointer<vtkTubeFilter>::New();
    tube->SetInputConnection(line->GetOutputPort());
    tube->SetRadius(0.2);
    tube->SetNumberOfSides(8);
    tube->CappingOn();

    appender->AddInputConnection(tube->GetOutputPort());
  }

  // Save result
  appender->Update();
  this->ModelNeedle->DeepCopy(appender->GetOutput());

}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateRobotModelProbe()
{
  this->ModelProbe->Reset();

  if (!this->CalibrationData.CalibrationValid)
  {
    // if there is no calibration, we cannot show where is the probe
    return;
  }

  vtkSmartPointer <vtkSTLReader> modelReader=vtkSmartPointer<vtkSTLReader>::New();

  vtksys_stl::string modelFileName=this->ModuleShareDirectory+"/LaserApplicator/laserApplicator.stl";
  modelReader->SetFileName(modelFileName.c_str());
  modelReader->Update();

  vtkSmartPointer<vtkTransformPolyDataFilter> polyTrans = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  polyTrans->SetInputConnection(modelReader->GetOutputPort());
  vtkSmartPointer<vtkTransform> modelTransform=vtkSmartPointer<vtkTransform>::New();
  polyTrans->SetTransform(modelTransform);

  // update robot base position
  vtkSmartPointer<vtkMatrix4x4> baseTransform=vtkSmartPointer<vtkMatrix4x4>::New();
  if (!GetRobotBaseTransform(modelTransform->GetMatrix()))    
  {
    // no calibration
    return;
  }

  polyTrans->Update();
  this->ModelProbe->DeepCopy(polyTrans->GetOutput());
}


//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateRobotModelAxes()
{
  this->ModelAxes->Reset();

  if (!this->CalibrationData.CalibrationValid)
  {
    return;
  }

  // merge all polydata by this appender
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  // form the axis 1 line
  // set up the line actors
  vtkSmartPointer<vtkLineSource> axis1Line = vtkSmartPointer<vtkLineSource>::New();  
  axis1Line->SetResolution(100); 
  
  double needle1[3];
  double needle2[3];
  for (int i=0; i<3; i++)
  {
    needle1[i]=this->CalibrationData.I2[i]-100*this->CalibrationData.v2[i];
    needle2[i]=this->CalibrationData.I2[i]+200*this->CalibrationData.v2[i];
  }

  axis1Line->SetPoint1(needle1);
  axis1Line->SetPoint2(needle2);

  appender->AddInputConnection(axis1Line->GetOutputPort());

  // 2nd axis line

  // form the axis 2 line
  // set up the line actors
  vtkSmartPointer<vtkLineSource> axis2Line = vtkSmartPointer<vtkLineSource>::New();  
  axis2Line->SetResolution(100); 

  double robotaxis1[3];
  double robotaxis2[3];
  for (int i=0; i<3; i++)
  {
    robotaxis1[i]=this->CalibrationData.I1[i]-100*this->CalibrationData.v1[i];
    robotaxis2[i]=this->CalibrationData.I1[i]+200*this->CalibrationData.v1[i];
  }

  axis2Line->SetPoint1(robotaxis1);
  axis2Line->SetPoint2(robotaxis2);

  appender->AddInputConnection(axis2Line->GetOutputPort());

  appender->Update();
  this->ModelAxes->DeepCopy(appender->GetOutput());
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateRobotModelMarker(int markerInd, vtkImageData *imagedata, vtkMatrix4x4* ijkToRAS)
{
  if (imagedata==NULL || ijkToRAS==NULL)
  {
    this->ModelMarkers[markerInd]->Reset();
    return;
  }
  vtkSmartPointer<vtkContourFilter> objectSurfaceExtractor=vtkSmartPointer<vtkContourFilter>::New();
  objectSurfaceExtractor->SetInput(imagedata);

  double* imgIntensityRange = imagedata->GetPointData()->GetScalars()->GetRange();
  double imgIntensityUnit = (imgIntensityRange[1]-imgIntensityRange[0])* 0.01; // 1 unit is 1 percent of the intensity range    
  objectSurfaceExtractor->SetValue(0, imgIntensityUnit);

  vtkSmartPointer<vtkTransformPolyDataFilter> polyTrans = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  polyTrans->SetInputConnection(objectSurfaceExtractor->GetOutputPort());
  vtkSmartPointer<vtkTransform> ijkToRASTransform=vtkSmartPointer<vtkTransform>::New();

  ijkToRASTransform->SetMatrix(ijkToRAS);
  polyTrans->SetTransform(ijkToRASTransform);
  polyTrans->Update();

  this->ModelMarkers[markerInd]->DeepCopy(polyTrans->GetOutput());  
}


//------------------------------------------------------------------------------
bool vtkMRMLLaserApplicatorNode::GetRobotBaseTransform(vtkMatrix4x4* transform)
{
  if (!this->CalibrationData.CalibrationValid)
  {
    // no claibration robot position is unknown
    return false;
  }  

  /*
    The transformation problem can be posed as:
    given a vector in spherical coordinates 

       \vec{a}  = a_\theta \vec{e}_\theta  + a_\phi \vec{e}_\phi +  a_r \vec{e}_r   

    find the corresponding components in a cartesion system

       \vec{a}  = a_x \vec{i}  + a_y \vec{j} + a_z \vec{k} 

    if we take this->CalibrationData.v1 to the be the radial direction \vec{e}_r ,
    representing this vector in terms of cartesion (x,y,z) has the
    interpretation of transforming the data to this line
     _   _       _                                                        _    _        _
    | a_x |     |  cos \theta cos \phi   -sin \phi   sin \theta cos \phi   |  | a_\theta |   
    | a_y |  =  |  cos \theta sin \phi    cos \phi   sin \theta sin \phi   |  | a_\phi   | 
    |_a_z_|     |_-sin \theta                0       cos \theta           _|  |_a_r     _|  
                                                                                       
     
   */
  double v1_norm[3]={this->CalibrationData.v1[0], this->CalibrationData.v1[1], this->CalibrationData.v1[2]};         
  vtkMath::Normalize(v1_norm);
  //double v2_norm[3]={this->CalibrationData.v2[0], this->CalibrationData.v2[1], this->CalibrationData.v2[2]};         
  //vtkMath::Normalize(v2_norm);

  //double v1xv2_norm[3]={0,0,0};
  //vtkMath::Cross(v1_norm, v2_norm, v1xv2_norm);
  //vtkMath::Normalize(v1xv2_norm);
  //double v1_x_v1xv2_norm[3]={0,0,0};
  //vtkMath::Cross(v1_norm, v1xv2_norm, v1_x_v1xv2_norm);
  //vtkMath::Normalize(v1_x_v1xv2_norm);

  // orientation

  //// x (orange)
  //transform->SetElement(0,0, v1xv2_norm[0]);
  //transform->SetElement(1,0, v1xv2_norm[1]);
  //transform->SetElement(2,0, v1xv2_norm[2]);

  //// y (orange)
  //transform->SetElement(0,1, v1_x_v1xv2_norm[0]);
  //transform->SetElement(1,1, v1_x_v1xv2_norm[1]);
  //transform->SetElement(2,1, v1_x_v1xv2_norm[2]);
  //
  //// z (orange)
  //transform->SetElement(0,2, v1_norm[0]);
  //transform->SetElement(1,2, v1_norm[1]);
  //transform->SetElement(2,2, v1_norm[2]);

  // compute spherical coordinate angles
  double CosTheta    = v1_norm[2];                                                  
  double SinTheta    = std::sqrt( v1_norm[0]*v1_norm[0] + v1_norm[1]*v1_norm[1]  ) ;
  if(SinTheta  > 0.01)
   {
    double CosPhi = v1_norm[0]/SinTheta;
    double SinPhi = v1_norm[1]/SinTheta;
    // x (orange)
    transform->SetElement(0,0,  CosTheta*CosPhi );
    transform->SetElement(1,0,  CosTheta*SinPhi );
    transform->SetElement(2,0, -SinTheta        );

    // y (orange)
    transform->SetElement(0,1, -SinPhi);
    transform->SetElement(1,1,  CosPhi);
    transform->SetElement(2,1,     0  );
    
    // z (orange)
    transform->SetElement(0,2, SinTheta*CosPhi );
    transform->SetElement(1,2, SinTheta*SinPhi );
    transform->SetElement(2,2, CosTheta        );

   }
  else
   {
     std::cout << std::endl
               << "vtkMRMLLaserApplicatorNode::GetRobotBaseTransform  "
               << "no transformation detected..." << std::endl ; 
   }

  // Hinge point
  //double l=14.5/sin(CalibrationData.AxesAngleDegrees * vtkMath::Pi()/180);
  double hingePoint[3];
  //hingePoint[0] = CalibrationData.I1[0] - l*CalibrationData.v2[0];
  //hingePoint[1] = CalibrationData.I1[1] - l*CalibrationData.v2[1];
  //hingePoint[2] = CalibrationData.I1[2] - l*CalibrationData.v2[2];
  hingePoint[0] = CalibrationData.I1[0] ;
  hingePoint[1] = CalibrationData.I1[1] ;
  hingePoint[2] = CalibrationData.I1[2] ;

  // position  
  transform->SetElement(0,3, hingePoint[0]);
  transform->SetElement(1,3, hingePoint[1]);
  transform->SetElement(2,3, hingePoint[2]);

  /*
  transform->SetElement(0,3, this->CalibrationData.I1[0]);
  transform->SetElement(1,3, this->CalibrationData.I1[1]);
  transform->SetElement(2,3, this->CalibrationData.I1[2]);
  */

  // print transformation info
  vtkIndent indent;
  transform->PrintSelf(std::cout,indent);

  return true;
} 

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::SetModelAxesVisible(bool visible)
{ 
  if (this->ModelAxesVisible==visible)
  {
    // no change
    return;
  }
  this->ModelAxesVisible=visible;
  UpdateRobotModel(); // :TODO: use standard vtkSetMacro and update automatically when the update event is triggered
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);

  if (!strcmp(oldID, this->RobotModelNodeRef))
    {
    this->SetRobotModelNodeRef(newID);
    }  
if (!strcmp(oldID, this->WorkspaceModelNodeRef))
    {
    this->SetWorkspaceModelNodeRef(newID);
    }  

  if (this->CalibrationPointListNodeID && !strcmp(oldID, this->CalibrationPointListNodeID))
    {
    this->SetAndObserveCalibrationPointListNodeID(newID);
    }

  if (this->CalibrationVolumeNodeID && !strcmp(oldID, this->CalibrationVolumeNodeID))
    {
    this->SetAndObserveCalibrationVolumeNodeID(newID);
    }

} 

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  
  if (this->RobotModelNodeRef != NULL && this->Scene->GetNodeByID(this->RobotModelNodeRef) == NULL)
    {
    this->SetRobotModelNodeRef(NULL);
    }
  if (this->WorkspaceModelNodeRef != NULL && this->Scene->GetNodeByID(this->WorkspaceModelNodeRef) == NULL)
    {
    this->SetWorkspaceModelNodeRef(NULL);
    }

  if (this->CalibrationPointListNodeID != NULL && this->Scene->GetNodeByID(this->CalibrationPointListNodeID) == NULL)
    {
    this->SetAndObserveCalibrationPointListNodeID(NULL);
    }

  if (this->CalibrationVolumeNodeID != NULL && this->Scene->GetNodeByID(this->CalibrationVolumeNodeID) == NULL)
    {
    this->SetAndObserveCalibrationVolumeNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveCalibrationPointListNodeID(this->CalibrationPointListNodeID);

   this->SetAndObserveCalibrationVolumeNodeID(this->CalibrationVolumeNodeID);

   // All the nodes are fully loaded, so update the calibration now
   UpdateCalibration();
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::SetAndObserveCalibrationPointListNodeID(const char *calibrationPointListNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->CalibrationPointListNode, NULL);
  this->SetCalibrationPointListNodeID(calibrationPointListNodeID);
  vtkMRMLFiducialListNode *tnode = this->GetCalibrationPointListNode();

  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);
  events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->CalibrationPointListNode, tnode, events);
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode* vtkMRMLLaserApplicatorNode::GetCalibrationPointListNode()
{
  vtkMRMLFiducialListNode* node = NULL;
  if (this->GetScene() && this->CalibrationPointListNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->CalibrationPointListNodeID);
    node = vtkMRMLFiducialListNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::SetAndObserveCalibrationVolumeNodeID(const char *calibrationVolumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->CalibrationVolumeNode, NULL);
  this->SetCalibrationVolumeNodeID(calibrationVolumeNodeID);
  vtkMRMLScalarVolumeNode *tnode = this->GetCalibrationVolumeNode();

  vtkSetAndObserveMRMLObjectMacro(this->CalibrationVolumeNode, tnode);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLLaserApplicatorNode::GetCalibrationVolumeNode()
{
  vtkMRMLScalarVolumeNode* node = NULL;
  if (this->GetScene() && this->CalibrationVolumeNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->CalibrationVolumeNodeID);
    node = vtkMRMLScalarVolumeNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
double vtkMRMLLaserApplicatorNode::GetMarkerSegmentationThreshold(int i)
{
  if (i<0 || i>(int)CALIB_MARKER_COUNT)
  {
    return 0.0;
  }
  return this->MarkerSegmentationThreshold[i];
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::ApplyTransform(vtkMatrix4x4* transformMatrix)
{ 
  Superclass::ApplyTransform(transformMatrix); 
  TransformCalibrationData(transformMatrix);
  TransformRobotModelMarkers(transformMatrix);
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::TransformCalibrationData(vtkMatrix4x4* transformMatrix)
{ 
  
// Need to convert coordinates from the volume to the robot coordinate system

  double point_calVolRas[4]={0,0,0,1};
  double point_robotRas[4]={0,0,0,1};  

  point_calVolRas[0]=this->CalibrationData.I1[0];
  point_calVolRas[1]=this->CalibrationData.I1[1];
  point_calVolRas[2]=this->CalibrationData.I1[2];
  transformMatrix->MultiplyPoint(point_calVolRas,point_robotRas);
  this->CalibrationData.I1[0]=point_robotRas[0];
  this->CalibrationData.I1[1]=point_robotRas[1];
  this->CalibrationData.I1[2]=point_robotRas[2];

  point_calVolRas[0]=this->CalibrationData.I2[0];
  point_calVolRas[1]=this->CalibrationData.I2[1];
  point_calVolRas[2]=this->CalibrationData.I2[2];
  transformMatrix->MultiplyPoint(point_calVolRas,point_robotRas);
  this->CalibrationData.I2[0]=point_robotRas[0];
  this->CalibrationData.I2[1]=point_robotRas[1];
  this->CalibrationData.I2[2]=point_robotRas[2];

  double vec_calVolRas[4]={0,0,0,0};
  double vec_robotRas[4]={0,0,0,0};  

  vec_calVolRas[0]=this->CalibrationData.v1[0];
  vec_calVolRas[1]=this->CalibrationData.v1[1];
  vec_calVolRas[2]=this->CalibrationData.v1[2];
  transformMatrix->MultiplyPoint(vec_calVolRas,vec_robotRas);
  this->CalibrationData.v1[0]=vec_robotRas[0];
  this->CalibrationData.v1[1]=vec_robotRas[1];
  this->CalibrationData.v1[2]=vec_robotRas[2];

  vec_calVolRas[0]=this->CalibrationData.v2[0];
  vec_calVolRas[1]=this->CalibrationData.v2[1];
  vec_calVolRas[2]=this->CalibrationData.v2[2];
  transformMatrix->MultiplyPoint(vec_calVolRas,vec_robotRas);
  this->CalibrationData.v2[0]=vec_robotRas[0];
  this->CalibrationData.v2[1]=vec_robotRas[1];
  this->CalibrationData.v2[2]=vec_robotRas[2];
}

//----------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::TransformRobotModelMarkers(vtkMatrix4x4* transformMatrix)
{   
  vtkSmartPointer<vtkTransformPolyDataFilter> polyTrans = vtkSmartPointer<vtkTransformPolyDataFilter>::New(); 
  vtkSmartPointer<vtkTransform> transform=vtkSmartPointer<vtkTransform>::New();
  transform->SetMatrix(transformMatrix);
  polyTrans->SetTransform(transform);
  for (unsigned int markerInd=0; markerInd<CALIB_MARKER_COUNT; markerInd++)
  {          
    polyTrans->SetInput(this->ModelMarkers[markerInd]);    
    polyTrans->Update();
    this->ModelMarkers[markerInd]->DeepCopy(polyTrans->GetOutput());
  }
}

/* copied from Applications/CLI/PolyDataToLabelmap.cxx */

LabelImageType::Pointer BinaryErodeFilter3D ( LabelImageType::Pointer & img , unsigned int ballsize )
{
  typedef itk::BinaryBallStructuringElement<unsigned char, 3> KernalType;
  typedef itk::BinaryErodeImageFilter<LabelImageType, LabelImageType, KernalType> ErodeFilterType;
  ErodeFilterType::Pointer erodeFilter = ErodeFilterType::New();
  erodeFilter->SetInput( img );

  KernalType ball;
  KernalType::SizeType ballSize;
  for (int k = 0; k < 3; k++)
    {
    ballSize[k] = ballsize;
    }
  ball.SetRadius(ballSize);
  ball.CreateStructuringElement();
  erodeFilter->SetKernel( ball );
  erodeFilter->Update();
  return erodeFilter->GetOutput();

}

LabelImageType::Pointer BinaryDilateFilter3D ( LabelImageType::Pointer & img , unsigned int ballsize )
{
  typedef itk::BinaryBallStructuringElement<unsigned char, 3> KernalType;
  typedef itk::BinaryDilateImageFilter<LabelImageType, LabelImageType, KernalType> DilateFilterType;
  DilateFilterType::Pointer dilateFilter = DilateFilterType::New();
  dilateFilter->SetInput( img );
  KernalType ball;
  KernalType::SizeType ballSize;
  for (int k = 0; k < 3; k++)
    {
    ballSize[k] = ballsize;
    }
  ball.SetRadius(ballSize);
  ball.CreateStructuringElement();
  dilateFilter->SetKernel( ball );
  dilateFilter->Update();
  return dilateFilter->GetOutput();
}

LabelImageType::Pointer BinaryOpeningFilter3D ( LabelImageType::Pointer & img , unsigned int ballsize )
{
  LabelImageType::Pointer imgErode = BinaryErodeFilter3D( img, ballsize);
  return BinaryDilateFilter3D( imgErode, ballsize );
}

LabelImageType::Pointer BinaryClosingFilter3D ( LabelImageType::Pointer & img , unsigned int ballsize )
{
  LabelImageType::Pointer imgDilate = BinaryDilateFilter3D( img, ballsize );
  return BinaryErodeFilter3D( imgDilate, ballsize );
}

//
// Description: project polydata onto imaging
template<class T> 
void ProjectLaserApplicatorModelToImage( T, vtkImageData *ImageData ,
                                         vtkPolyData *polyData ,
                                         double *Origin ,
                                         double *Spacing , double sampleDistance )
{
  // convert data to itk
  const   unsigned int    Dimension = 3;
  typedef itk::Image< T , Dimension > ConvertITKImageType;
  typedef itk::VTKImageToImageFilter< ConvertITKImageType >  ConnectorType;
  typename ConnectorType::Pointer connector = ConnectorType::New();
  connector->SetInput( ImageData );
  connector->Update(); 

  // output label map
  LabelImageType::Pointer label = LabelImageType::New();
  LabelImageType::PointType orgn_mm;
  orgn_mm[0] = Origin[0];
  orgn_mm[1] = Origin[1];
  orgn_mm[2] = Origin[2];
  LabelImageType::SpacingType spacing_mm;
  spacing_mm[0] = Spacing[0];
  spacing_mm[1] = Spacing[1];
  spacing_mm[2] = Spacing[2];

  label->CopyInformation( connector->GetOutput() );
  label->SetSpacing( spacing_mm );
  label->SetOrigin( orgn_mm );
  label->SetRegions( label->GetLargestPossibleRegion() );
  label->Allocate();
  label->FillBuffer( 0 );

  // LPS vs RAS

  //vtkPoints * allPoints = polyData->GetPoints();
  //for (int k = 0; k < allPoints->GetNumberOfPoints(); k++)
  //{
  //  double* point = polyData->GetPoint( k );
  //  point[0] = -point[0];
  //  point[1] = -point[1];
  //  allPoints->SetPoint( k, point[0], point[1], point[2] );
  //}  


  // do it
//void PolyDataToLabelMap( vtkPolyData* polyData, LabelImageType::Pointer label)

  vtkPolyDataPointSampler* sampler = vtkPolyDataPointSampler::New();
  
  sampler->SetInput( polyData );
  sampler->SetDistance( sampleDistance );
  sampler->GenerateEdgePointsOn();
  sampler->GenerateInteriorPointsOn();
  sampler->GenerateVertexPointsOn();
  sampler->Update();

  std::cout << polyData->GetNumberOfPoints() << std::endl;
  std::cout << sampler->GetOutput()->GetNumberOfPoints() << std::endl;

  // TODO: coordinate system nightmare 
  // TODO: transform model to imaging coordinate to compute the intersection
  for (int k = 0; k < sampler->GetOutput()->GetNumberOfPoints(); k++)
  {
    double *pt = sampler->GetOutput()->GetPoint( k );
    LabelImageType::PointType pitk;
    pitk[0] = pt[0];
    pitk[1] = pt[1];
    pitk[2] = pt[2];
    LabelImageType::IndexType idx;
    label->TransformPhysicalPointToIndex( pitk, idx );

    if ( label->GetLargestPossibleRegion().IsInside(idx) )
      {
      label->SetPixel( idx, 255 );
      }
  }

  // do morphological closing
  LabelImageType::Pointer closedLabel = BinaryClosingFilter3D( label, 2);
  itk::ImageRegionIteratorWithIndex<LabelImageType> itLabel (closedLabel, closedLabel->GetLargestPossibleRegion() );

  // do flood fill using binary threshold image function
  typedef itk::BinaryThresholdImageFunction<LabelImageType>ImageFunctionType;
  ImageFunctionType::Pointer func = ImageFunctionType::New();
  func->SetInputImage( closedLabel );
  func->ThresholdBelow(1);

  LabelImageType::IndexType idx;
  LabelImageType::PointType COG;

  // set the centre of gravity
  //double *bounds = polyData->GetBounds();
  COG.Fill (0.0);
  for (vtkIdType k = 0; k < polyData->GetNumberOfPoints(); k++)
    {
    double *pt = polyData->GetPoint( k );
    for ( int m = 0; m < 3; m++ )
      {
      COG[m] += pt[m];
      }
    }
  for ( int m = 0; m < 3; m++ )
    {
    COG[m] /= static_cast<float>( polyData->GetNumberOfPoints() );
    }

  label->TransformPhysicalPointToIndex( COG, idx );

  itk::FloodFilledImageFunctionConditionalIterator<LabelImageType,ImageFunctionType> floodFill( closedLabel, func, idx );
  
  for (floodFill.GoToBegin(); !floodFill.IsAtEnd(); ++floodFill)
    {
    LabelImageType::IndexType i = floodFill.GetIndex();
    closedLabel->SetPixel( i, 255 );
    }
  LabelImageType::Pointer finalLabel = BinaryClosingFilter3D( closedLabel, 2);

  for (itLabel.GoToBegin(); !itLabel.IsAtEnd(); ++itLabel)
    {
    LabelImageType::IndexType i = itLabel.GetIndex();
    label->SetPixel( i, finalLabel->GetPixel(i) );
    }
       
  typedef itk::ImageFileWriter< LabelImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( "test.mha" );
  writer->SetInput(  label );
  writer->Update();

  sampler->Delete();

  return ;
}

//------------------------------------------------------------------------------
void vtkMRMLLaserApplicatorNode::WriteDicomPlanningSeries(vtkPolyData *polyData)
{

  ////get pointer to model of damage
  //vtkMRMLModelNode* damageNode=GetWorkspaceModelNode();
  //if (damageNode==NULL)
  //{
  //  return;
  //}
  //vtkMRMLModelDisplayNode* displayDamageNode = vtkMRMLModelDisplayNode::SafeDownCast(damageNode->GetDisplayNode());
  //if (displayDamageNode==NULL)
  //{
  //  return;
  //}      

  //if (!this->CalibrationData.CalibrationValid)
  //{
  //  // no calibration robot position is unknown
  //  //displayDamageNode->VisibilityOff();
  //  //displayDamageNode->SliceIntersectionVisibilityOff();  
  //  return;
  //}

  //vtkSmartPointer <vtkSTLReader> modelReader=vtkSmartPointer<vtkSTLReader>::New();

  //vtksys_stl::string
  //delFileName=this->ModuleShareDirectory+"/LaserApplicator/damageTemplate.stl";
  //modelReader->SetFileName(modelFileName.c_str());
  //modelReader->Update();

  //vtkSmartPointer<vtkTransformPolyDataFilter> polyTrans = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  //polyTrans->SetInputConnection(modelReader->GetOutputPort());
  //vtkSmartPointer<vtkTransform> modelTransform=vtkSmartPointer<vtkTransform>::New();
  //polyTrans->SetTransform(modelTransform);

  //GetRobotBaseTransform(modelTransform->GetMatrix());

  //vtkSmartPointer<vtkTriangleFilter> cleaner=vtkSmartPointer<vtkTriangleFilter>::New();
  //cleaner->SetInputConnection(polyTrans->GetOutputPort());
  //cleaner->Update();
  //
  //modelNode->SetAndObservePolyData(cleaner->GetOutput());
  //modelNode->SetModifiedSinceRead(1);

  //displayDamageNode->SetModifiedSinceRead(1); 

  //displayDamageNode->SliceIntersectionVisibilityOn();  
  //displayDamageNode->VisibilityOn();

  //get pointer to imaging data
  vtkMRMLScalarVolumeNode *calVolNode=GetCalibrationVolumeNode();
  if (calVolNode==0)
  {    
    SetCalibrationStatusDescription("Calibration volume is not available");
    ResetCalibrationData();
    return;
  }

  if (calVolNode->GetImageData()==NULL)
  {
    // cannot update the calibration data because the scene has is not been fully loaded yet
    // so just use the calibration data that was saved with the scene - do not reset!
    if (this->CalibrationData.CalibrationValid)
    {
      SetCalibrationStatusDescription("Using saved calibration data");
    }
    else
    {
      ResetCalibrationData();
    }
    return;
  }

  vtkImageData *VolumeImageData =calVolNode->GetImageData();
  //TODO intelligently choose the sample distance
  double sampleDistance = 1.0;
  try
    {
    switch (VolumeImageData->GetScalarType())
      {
      case VTK_CHAR:           
      case VTK_SIGNED_CHAR:   
      case VTK_UNSIGNED_CHAR: 
        ProjectLaserApplicatorModelToImage(  static_cast<char>(0),
           VolumeImageData, polyData , 
           calVolNode->GetOrigin(), calVolNode->GetSpacing(), sampleDistance );
        break;
      case VTK_SHORT:            
      case VTK_UNSIGNED_SHORT:   
        ProjectLaserApplicatorModelToImage( static_cast<short>(0),
           VolumeImageData, polyData , 
           calVolNode->GetOrigin(), calVolNode->GetSpacing(), sampleDistance );
        break;
      case VTK_INT:              
      case VTK_UNSIGNED_INT:     
        ProjectLaserApplicatorModelToImage( static_cast<int>(0),
           VolumeImageData, polyData , 
           calVolNode->GetOrigin(), calVolNode->GetSpacing(), sampleDistance );
        break;
      case VTK_LONG:             
      case VTK_UNSIGNED_LONG:    
        ProjectLaserApplicatorModelToImage( static_cast<long>(0),
           VolumeImageData, polyData , 
           calVolNode->GetOrigin(), calVolNode->GetSpacing(), sampleDistance );
        break;
      case VTK_FLOAT:            
        ProjectLaserApplicatorModelToImage( static_cast<float>(0),
           VolumeImageData, polyData , 
           calVolNode->GetOrigin(), calVolNode->GetSpacing(), sampleDistance );
        break;
      case VTK_DOUBLE:           
        ProjectLaserApplicatorModelToImage( static_cast<double>(0),
           VolumeImageData, polyData , 
           calVolNode->GetOrigin(), calVolNode->GetSpacing(), sampleDistance );
        break;
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return ;
    }

}
