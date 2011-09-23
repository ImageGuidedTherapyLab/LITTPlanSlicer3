PROJECT(LITTPlan)

cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Echo Compiler Options
SET(CMAKE_VERBOSE_MAKEFILE ON)

# --------------------------------------------------------------------------         
# 
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
#   Load settings for an external project.
#
#     find_package(<package> [version] [EXACT] [QUIET]
#                  [[REQUIRED|COMPONENTS] [components...]]
#                  [NO_POLICY_SCOPE])
#
#   CMake  searches  for  a  file  called "Find<package>.cmake"  in
#   the CMAKE_MODULE_PATH followed by the CMake installation.
#   If no module is found the command proceeds to Config mode.
#   Config  mode  attempts to locate a configuration file
#   provided by the package to be found.  A cache entry called
#   <package>_DIR is created to hold the directory
#   containing the file.  By default the command searches for a pack-
#   age with the name <package>.  If the NAMES option is
#   given the names following it are used instead of <package>.
#   The command searches for a file called
#   "<name>Config.cmake" or "<lower-case-name>-config.cmake"  for  each
#   name specified. 
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

if(NOT CMAKE_CURRENT_BINARY_DIR)
  message("cant find CMAKE_CURRENT_BINARY_DIR")
endif(NOT CMAKE_CURRENT_BINARY_DIR)
if(NOT Slicer3_Libs_INCLUDE_DIRS)
  message("cant find Slicer3_Libs_INCLUDE_DIRS")
endif(NOT Slicer3_Libs_INCLUDE_DIRS)
if(NOT Slicer3_Base_INCLUDE_DIRS)
  message("cant find Slicer3_Base_INCLUDE_DIRS")
endif(NOT Slicer3_Base_INCLUDE_DIRS)
if(NOT OpenIGTLinkIF_SOURCE_DIR)
  set(OpenIGTLinkIF_SOURCE_DIR "${Slicer3_HOME}/../Slicer3/Modules/OpenIGTLinkIF")
  message("OpenIGTLinkIF_SOURCE_DIR set manually: ${OpenIGTLinkIF_SOURCE_DIR}")
endif(NOT OpenIGTLinkIF_SOURCE_DIR)
if(NOT OpenIGTLinkIF_BINARY_DIR)
  set(OpenIGTLinkIF_BINARY_DIR "${Slicer3_HOME}/Modules/OpenIGTLinkIF")
  message("OpenIGTLinkIF_BINARY_DIR set manually: ${OpenIGTLinkIF_BINARY_DIR}")
endif(NOT OpenIGTLinkIF_BINARY_DIR)
if(NOT OpenIGTLink_INCLUDE_DIRS)
  set(OpenIGTLink_DIR "${Slicer3_HOME}/../Slicer3-lib/OpenIGTLink-build")
  find_package(OpenIGTLink REQUIRED)
  message("OpenIGTLink found: OpenIGTLink_INCLUDE_DIRS ${OpenIGTLink_INCLUDE_DIRS}")
endif(NOT OpenIGTLink_INCLUDE_DIRS)
if(NOT vtkSlicerVolumesModuleLogic_SOURCE_DIR)
  set(vtkSlicerVolumesModuleLogic_SOURCE_DIR "${Slicer3_HOME}/../Slicer3/Modules/Volumes/Logic")
  message("vtkSlicerVolumesModuleLogic_SOURCE_DIR set manually: ${vtkSlicerVolumesModuleLogic_SOURCE_DIR} ")
endif(NOT vtkSlicerVolumesModuleLogic_SOURCE_DIR)
if(NOT vtkSlicerVolumesModuleLogic_BINARY_DIR)
  set(vtkSlicerVolumesModuleLogic_BINARY_DIR "${Slicer3_HOME}/Modules/Volumes/Logic")
  message("vtkSlicerVolumesModuleLogic_BINARY_DIR set manually: ${vtkSlicerVolumesModuleLogic_BINARY_DIR}")
endif(NOT vtkSlicerVolumesModuleLogic_BINARY_DIR)

if(NOT Volumes_SOURCE_DIR)
  set(Volumes_SOURCE_DIR "${Slicer3_HOME}/../Slicer3/Modules/Volumes")
  message("Volumes_SOURCE_DIR set manually: ${Volumes_SOURCE_DIR}")
endif(NOT Volumes_SOURCE_DIR)
if(NOT Volumes_BINARY_DIR)
  set(Volumes_BINARY_DIR "${Slicer3_HOME}/Modules/Volumes")
  message("Volumes_BINARY_DIR set manually: ${Volumes_BINARY_DIR}")
endif(NOT Volumes_BINARY_DIR)
# --------------------------------------------------------------------------
# Find Slicer3

#find_package(Subversion)
#if (Subversion_FOUND)
 # Subversion_WC_INFO(${CMAKE_HOME_DIRECTORY} LITTPlan)
 # set(LITTPlan_REVISION "${LITTPlan_WC_REVISION}")
#endif (Subversion_FOUND)

# --------------------------------------------------------------------------
# Include dirs


#
# Include dirs
#


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateTemplate
  ${CMAKE_CURRENT_SOURCE_DIR}/LaserApplicator
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${Slicer3_HOME}/../Slicer3-lib/Insight/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src
  ${OpenIGTLinkIF_SOURCE_DIR}
  ${OpenIGTLinkIF_BINARY_DIR}
  ${OpenIGTLink_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )


#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkLITTPlanConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkLITTPlanConfigure.h
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkLITTPlanVersion.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkLITTPlanVersion.h
  )



#
# Installation
#

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateTemplate/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LaserApplicator/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Wizard/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/newmat/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkLITTPlanConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# Install transrectal robot files
set(TransRectalRobotSubdir LaserApplicator)
foreach(TransRectalRobotFile 
  Sheath.stl
  Workspace150mm.stl
  Workspace200mm.stl
  laserApplicator.stl
  damageTemplate.stl
  )
  # Copy to the build tree (for Slicer execution in the Slicer3-build directory)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${TransRectalRobotSubdir}/${TransRectalRobotFile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${TransRectalRobotSubdir}/${TransRectalRobotFile}
    COPYONLY
    )
  # Copy to the installation target (for Slicer execution when it is installed)
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/${TransRectalRobotSubdir}/${TransRectalRobotFile} 
    DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${TransRectalRobotSubdir}
    )
endforeach(TransRectalRobotFile)  

# --------------------------------------------------------------------------
# Sources

set(LITTPlan_SRCS 
  vtkMRMLLITTPlanManagerNode.cxx
  vtkLITTPlanLogic.cxx
  vtkLITTPlanGUI.cxx
  vtkSlicerSecondaryViewerWindow.cxx 
  vtkMRMLRobotNode.cxx
  vtkMRMLRobotDisplayNode.cxx
  vtkLITTPlanTargetDescriptor.cxx  
  vtkRobotToImageRegistration.cxx
  vtkZFrameRobotToImageRegistration.cxx
  vtkZFrameRobotToImageRegistration2.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkIGTLToMRMLCoordinate.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkIGTLToMRMLBrpRobotCommand.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkMRMLBrpRobotCommandNode.cxx  
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkMRMLTransPerinealProstateRobotNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateTemplate/vtkMRMLTransPerinealProstateTemplateNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/LaserApplicator/vtkMRMLLaserApplicatorNode.cxx  
  ${CMAKE_CURRENT_SOURCE_DIR}/LaserApplicator/vtkLaserApplicatorFiducialCalibrationAlgo.cxx  
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanStepSetUp.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanStepSetUpTemplate.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanCalibrationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanFiducialCalibrationStep.cxx   
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanTargetingStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanManualControlStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanStepVerification.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLITTPlanStepTargetingTemplate.cxx
  )

set(LITTPlanIGTL_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlCoordinateMessage.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlHeaderMessage.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlMoveToMessage.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlSetZFrameMessage.cxx
)

set(LITTPlanNewmat_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat9.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmatrm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/nm_misc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/bandmat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/solution.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/cholesky.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/hholder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmatex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat3.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/sort.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/evalue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/jacobi.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat4.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/submat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/example.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat5.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/svd.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/myexcept.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat6.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmatnl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/fft.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat7.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newfft.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat8.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/ZLinAlg.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(LITTPlan LITTPlan_TCL_SRCS "${LITTPlan_SRCS}" "")


#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(LITTPlan_SRCS LITTPlan.txt)


# --------------------------------------------------------------------------
# Build and install the library

set(lib_name LITTPlan)

message("${lib_name} adding ${Slicer3_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR} to link_directories.. needs to be before add_library")
link_directories(${Slicer3_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR})

add_library(${lib_name}
  ${LITTPlan_SRCS} 
  ${LITTPlanIGTL_SRCS}
  ${LITTPlanNewmat_SRCS} 
  ${LITTPlan_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  OpenIGTLinkIF
  vtkSlicerVolumesModuleLogic
  Volumes
  )


slicer3_install_modules(${lib_name})

