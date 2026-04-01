cmake_minimum_required(VERSION 3.16)

project(Sahara VERSION 1.0.0 LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 20)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Enable more warnings and multi-processor builds for MSVC
if(MSVC)
	add_compile_options(/W4)
	add_compile_options(/MP)
endif()

add_compile_definitions(_USE_MATH_DEFINES)

if (NOT MSVC)
	add_compile_definitions(_GLIBCXX_USE_TBB_PAR_BACKEND=0)
endif ()

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Gui Widgets OpenGL Concurrent)
find_package(OpenGL REQUIRED)

set(QT_ROOT ${Qt6_DIR}/../../../)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/thirdparty)

option(BUILD_CLANG_FORMAT_TARGET "Build Clang Format Target" ON)


# Activate resource/moc compilation
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

# Gather source files
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set(GEOMETRY_SOURCES
	${SOURCE_DIR}/geometry/AbstractAttributeData.h
	${SOURCE_DIR}/geometry/AbstractLodOctree.h
	${SOURCE_DIR}/geometry/AbstractLodOctree.cpp
	${SOURCE_DIR}/geometry/AttributeData.h
	${SOURCE_DIR}/geometry/AttributeData.inl
	${SOURCE_DIR}/geometry/AttributeMetadata.h
	${SOURCE_DIR}/geometry/AttributeMetadata.inl
	${SOURCE_DIR}/geometry/AttributeMetadata.h
	${SOURCE_DIR}/geometry/AbstractNodePriorityComputation.cpp
	${SOURCE_DIR}/geometry/AbstractNodePriorityComputation.h
	${SOURCE_DIR}/geometry/AttributeSpecification.cpp
	${SOURCE_DIR}/geometry/AttributeSpecification.h
	${SOURCE_DIR}/geometry/AttributeTypes.h
	${SOURCE_DIR}/geometry/BoundingBox.h
	${SOURCE_DIR}/geometry/BoundingBox.cpp
	${SOURCE_DIR}/geometry/Color.h
	${SOURCE_DIR}/geometry/LodOctree.h
	${SOURCE_DIR}/geometry/LodOctree.cpp
	${SOURCE_DIR}/geometry/LodOctreeNode.h
	${SOURCE_DIR}/geometry/LodOctreeNode.cpp
	${SOURCE_DIR}/geometry/MultiTemporalLodOctree.h
	${SOURCE_DIR}/geometry/MultiTemporalLodOctree.cpp
	${SOURCE_DIR}/geometry/NodePriorityComputation.h
	${SOURCE_DIR}/geometry/NodePriorityComputation.cpp
	${SOURCE_DIR}/geometry/NodePriorityComputationHelper.h
	${SOURCE_DIR}/geometry/NodePriorityComputationMultiTemporal.h
	${SOURCE_DIR}/geometry/NodePriorityComputationMultiTemporal.cpp
	${SOURCE_DIR}/geometry/SerializedOctreeNode.h
	${SOURCE_DIR}/geometry/StaticPointCloud.h
	${SOURCE_DIR}/geometry/StaticPointCloud.cpp
	${SOURCE_DIR}/geometry/StaticPointCloudFactory.h
	${SOURCE_DIR}/geometry/StaticPointCloudFactory.cpp
	${SOURCE_DIR}/geometry/PointCloudProcessing.h
	${SOURCE_DIR}/geometry/PointCloudProcessing.cpp
)


set(NAVIGATION_SOURCES
	${SOURCE_DIR}/navigation/AbstractNavigation.h
	${SOURCE_DIR}/navigation/AbstractNavigation.cpp
	${SOURCE_DIR}/navigation/AnimatedPathNavigation.h
	${SOURCE_DIR}/navigation/AnimatedPathNavigation.cpp
	${SOURCE_DIR}/navigation/Camera.h
	${SOURCE_DIR}/navigation/Camera.cpp
	${SOURCE_DIR}/navigation/CameraAnimationPath.h
	${SOURCE_DIR}/navigation/CameraAnimationStep.h
	${SOURCE_DIR}/navigation/CameraAnimationStep.cpp
	${SOURCE_DIR}/navigation/FirstPersonNavigation.h
	${SOURCE_DIR}/navigation/FirstPersonNavigation.cpp
	${SOURCE_DIR}/navigation/NavigationHandler.h
	${SOURCE_DIR}/navigation/NavigationHandler.cpp
	${SOURCE_DIR}/navigation/OrbitalNavigation.h
	${SOURCE_DIR}/navigation/OrbitalNavigation.cpp
)

set(GENERAL_RENDERING_SOURCES
	${SOURCE_DIR}/rendering/Framebuffer.h
	${SOURCE_DIR}/rendering/Framebuffer.cpp
	${SOURCE_DIR}/rendering/OpenGLContext.h
	${SOURCE_DIR}/rendering/OpenGLContext.cpp
	${SOURCE_DIR}/rendering/Renderer.h
	${SOURCE_DIR}/rendering/Renderer.cpp
	${SOURCE_DIR}/rendering/RendererModuleTypes.h
	${SOURCE_DIR}/rendering/Scene.h
	${SOURCE_DIR}/rendering/Scene.cpp
	${SOURCE_DIR}/rendering/ShaderProgramFactory.h
	${SOURCE_DIR}/rendering/ShaderProgramFactory.cpp
)

set(PARAMETERS_SOURCES
	${SOURCE_DIR}/rendering/parameters/AbstractParameter.h
	${SOURCE_DIR}/rendering/parameters/AbstractParameter.cpp
	${SOURCE_DIR}/rendering/parameters/EnumParameter.h
	${SOURCE_DIR}/rendering/parameters/EnumParameter.cpp
	${SOURCE_DIR}/rendering/parameters/FilePathParameter.h
	${SOURCE_DIR}/rendering/parameters/FilePathParameter.cpp
	${SOURCE_DIR}/rendering/parameters/Parameter.h
	${SOURCE_DIR}/rendering/parameters/Parameter.inl
	${SOURCE_DIR}/rendering/parameters/RangeParameter.h
	${SOURCE_DIR}/rendering/parameters/RangeParameter.inl
	${SOURCE_DIR}/rendering/parameters/TriggerParameter.h
	${SOURCE_DIR}/rendering/parameters/TriggerParameter.cpp
)

set(POINTCLOUD_PROVIDERS_SOURCES
	${SOURCE_DIR}/rendering/pointcloudProviders/AbstractOOCPointCloudProvider.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/AbstractOOCPointCloudProvider.h
	${SOURCE_DIR}/rendering/pointcloudProviders/AbstractPointCloudProvider.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/AbstractPointCloudProvider.h
	${SOURCE_DIR}/rendering/pointcloudProviders/AbstractPointCloudProvider.inl
	${SOURCE_DIR}/rendering/pointcloudProviders/BufferSegmentMetadata.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/BufferSegmentMetadata.h
	${SOURCE_DIR}/rendering/pointcloudProviders/DrawArraysIndirectCommand.h
	${SOURCE_DIR}/rendering/pointcloudProviders/DrawCommandBuffer.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/DrawCommandBuffer.h
	${SOURCE_DIR}/rendering/pointcloudProviders/GPUBufferManager.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/GPUBufferManager.h
	${SOURCE_DIR}/rendering/pointcloudProviders/MappedOpenGLBuffer.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/MappedOpenGLBuffer.h
	${SOURCE_DIR}/rendering/pointcloudProviders/MultiTemporalPointCloudProvider.h
	${SOURCE_DIR}/rendering/pointcloudProviders/MultiTemporalPointCloudProvider.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/OOCSinglePointCloudProvider.h
	${SOURCE_DIR}/rendering/pointcloudProviders/OOCSinglePointCloudProvider.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/OOCMultiTemporalPointCloudProvider.h
	${SOURCE_DIR}/rendering/pointcloudProviders/OOCMultiTemporalPointCloudProvider.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/PointCloudProviderFactory.h
	${SOURCE_DIR}/rendering/pointcloudProviders/PointCloudProviderFactory.cpp
	${SOURCE_DIR}/rendering/pointcloudProviders/StaticPointCloudProvider.h
	${SOURCE_DIR}/rendering/pointcloudProviders/StaticPointCloudProvider.cpp
)

set(RASTERIZATION_SOURCES
	${SOURCE_DIR}/rendering/rasterization/AbstractRasterizer.h
	${SOURCE_DIR}/rendering/rasterization/AbstractRasterizer.cpp
	${SOURCE_DIR}/rendering/rasterization/OOCPointPrimitiveRasterizer.h
	${SOURCE_DIR}/rendering/rasterization/OOCPointPrimitiveRasterizer.cpp
	${SOURCE_DIR}/rendering/rasterization/PointPrimitiveRasterizer.h
	${SOURCE_DIR}/rendering/rasterization/PointPrimitiveRasterizer.cpp
	${SOURCE_DIR}/rendering/rasterization/RasterizerFactory.h
	${SOURCE_DIR}/rendering/rasterization/RasterizerFactory.cpp
)

set(COLORIZATION_SOURCES
	${SOURCE_DIR}/rendering/colorization/AbstractColorizer.h
	${SOURCE_DIR}/rendering/colorization/AbstractColorizer.cpp
	${SOURCE_DIR}/rendering/colorization/AttributeBasedColorizer.h
	${SOURCE_DIR}/rendering/colorization/AttributeBasedColorizer.cpp
	${SOURCE_DIR}/rendering/colorization/ColorizerFactory.h
	${SOURCE_DIR}/rendering/colorization/ColorizerFactory.cpp
	${SOURCE_DIR}/rendering/colorization/DefaultColorizer.h
	${SOURCE_DIR}/rendering/colorization/DefaultColorizer.cpp
	${SOURCE_DIR}/rendering/colorization/SingleColorColorizer.h
	${SOURCE_DIR}/rendering/colorization/SingleColorColorizer.cpp
)

set(POSTPROCESSING_SOURCES
	${SOURCE_DIR}/rendering/postprocessing/AbstractPostprocessor.h
	${SOURCE_DIR}/rendering/postprocessing/AbstractPostprocessor.cpp
	${SOURCE_DIR}/rendering/postprocessing/EyeDomeLightingPostprocessor.h
	${SOURCE_DIR}/rendering/postprocessing/EyeDomeLightingPostprocessor.cpp
	${SOURCE_DIR}/rendering/postprocessing/HoleFillingPostprocessor.h
	${SOURCE_DIR}/rendering/postprocessing/HoleFillingPostprocessor.cpp
	${SOURCE_DIR}/rendering/postprocessing/PostprocessorFactory.h
	${SOURCE_DIR}/rendering/postprocessing/PostprocessorFactory.cpp
	${SOURCE_DIR}/rendering/postprocessing/PostprocessorPipeline.h
	${SOURCE_DIR}/rendering/postprocessing/PostprocessorPipeline.cpp
)

set(UTILS_SOURCES
	${SOURCE_DIR}/utils/CachedValue.h
	${SOURCE_DIR}/utils/CachedValue.inl
	${SOURCE_DIR}/utils/EasingCurveIcons.h
	${SOURCE_DIR}/utils/EasingCurveIcons.cpp
	${SOURCE_DIR}/utils/LimitedQueue.h
	${SOURCE_DIR}/utils/LimitedQueue.inl
	${SOURCE_DIR}/utils/MemoryStream.h
	${SOURCE_DIR}/utils/MemoryStream.cpp
	${SOURCE_DIR}/utils/MultiTemporalBenchmarkController.h
	${SOURCE_DIR}/utils/MultiTemporalBenchmarkController.cpp
	${SOURCE_DIR}/utils/NoInitializationAllocator.h
	${SOURCE_DIR}/utils/NoInitializationAllocator.inl
	${SOURCE_DIR}/utils/NumericLimits.h
	${SOURCE_DIR}/utils/PriorityMutex.h
	${SOURCE_DIR}/utils/PriorityMutex.cpp
	${SOURCE_DIR}/utils/Profiler.h
	${SOURCE_DIR}/utils/Profiler.cpp
	${SOURCE_DIR}/utils/QVector3DUtilities.h
	${SOURCE_DIR}/utils/ShaderUtilities.h
	${SOURCE_DIR}/utils/ShaderUtilities.cpp
)

set(GENERAL_UI_SOURCES
	${SOURCE_DIR}/ui/GeometryLoadingDialog.ui
	${SOURCE_DIR}/ui/InspectPointCloudDialog.h
	${SOURCE_DIR}/ui/InspectPointCloudDialog.cpp
	${SOURCE_DIR}/ui/InspectPointCloudDialog.ui
	${SOURCE_DIR}/ui/MainWindow.ui
	${SOURCE_DIR}/ui/MainWindow.h
	${SOURCE_DIR}/ui/MainWindow.cpp
	${SOURCE_DIR}/ui/RenderWindow.h
	${SOURCE_DIR}/ui/RenderWindow.cpp
)

set(CAMERA_PANELS_SOURCES
	${SOURCE_DIR}/ui/cameraPanels/CameraPathPanel.ui
	${SOURCE_DIR}/ui/cameraPanels/CameraPathPanel.h
	${SOURCE_DIR}/ui/cameraPanels/CameraPathPanel.cpp
	${SOURCE_DIR}/ui/cameraPanels/CameraSettingsPanel.ui
	${SOURCE_DIR}/ui/cameraPanels/CameraSettingsPanel.h
	${SOURCE_DIR}/ui/cameraPanels/CameraSettingsPanel.cpp
)

set(PARAMETER_WIDGETS_SOURCES
	${SOURCE_DIR}/ui/parameterWidgets/MultipleParametersWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/MultipleParametersWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/BoolParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/BoolParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/ColorParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/ColorParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/EnumParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/EnumParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/FilePathParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/FilePathParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/FileSelector.h
	${SOURCE_DIR}/ui/parameterWidgets/FileSelector.cpp
	${SOURCE_DIR}/ui/parameterWidgets/FloatParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/FloatParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/HidingSlider.h
	${SOURCE_DIR}/ui/parameterWidgets/HidingSlider.cpp
	${SOURCE_DIR}/ui/parameterWidgets/HidingSliderToolButton.h
	${SOURCE_DIR}/ui/parameterWidgets/HidingSliderToolButton.cpp
	${SOURCE_DIR}/ui/parameterWidgets/IntParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/IntParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/SliderAndSpinBoxFloat.h
	${SOURCE_DIR}/ui/parameterWidgets/SliderAndSpinBoxFloat.cpp
	${SOURCE_DIR}/ui/parameterWidgets/SliderAndSpinBoxInt.h
	${SOURCE_DIR}/ui/parameterWidgets/SliderAndSpinBoxInt.cpp
	${SOURCE_DIR}/ui/parameterWidgets/TriggerParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/TriggerParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/ParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/ParameterWidget.cpp
	${SOURCE_DIR}/ui/parameterWidgets/ParameterWidgetFactory.h
	${SOURCE_DIR}/ui/parameterWidgets/ParameterWidgetFactory.cpp
	${SOURCE_DIR}/ui/parameterWidgets/Vector3DParameterWidget.h
	${SOURCE_DIR}/ui/parameterWidgets/Vector3DParameterWidget.cpp
)


set(RENDERER_SETTINGS_PANEL_SOURCES
	${SOURCE_DIR}/ui/rendererSettingsPanel/ParametersPanel.ui
	${SOURCE_DIR}/ui/rendererSettingsPanel/ParametersPanel.h
	${SOURCE_DIR}/ui/rendererSettingsPanel/ParametersPanel.cpp
	${SOURCE_DIR}/ui/rendererSettingsPanel/PostprocessorPipelineParametersPanel.ui
	${SOURCE_DIR}/ui/rendererSettingsPanel/PostprocessorPipelineParametersPanel.h
	${SOURCE_DIR}/ui/rendererSettingsPanel/PostprocessorPipelineParametersPanel.cpp
	${SOURCE_DIR}/ui/rendererSettingsPanel/PostprocessorPipelineParametersWidget.h
	${SOURCE_DIR}/ui/rendererSettingsPanel/PostprocessorPipelineParametersWidget.cpp
	${SOURCE_DIR}/ui/rendererSettingsPanel/RendererSettingsPanel.ui
	${SOURCE_DIR}/ui/rendererSettingsPanel/RendererSettingsPanel.h
	${SOURCE_DIR}/ui/rendererSettingsPanel/RendererSettingsPanel.cpp
)

set(SCENE_PANEL_SOURCES
	${SOURCE_DIR}/ui/scenePanel/ScenePanel.ui
	${SOURCE_DIR}/ui/scenePanel/ScenePanel.h
	${SOURCE_DIR}/ui/scenePanel/ScenePanel.cpp
	${SOURCE_DIR}/ui/scenePanel/ScenePanelItem.h
	${SOURCE_DIR}/ui/scenePanel/ScenePanelItem.cpp
	${SOURCE_DIR}/ui/scenePanel/ScenePanelItemWidget.h
	${SOURCE_DIR}/ui/scenePanel/ScenePanelItemWidget.cpp
)

set(CORE_SOURCES
	${SOURCE_DIR}/main.cpp
)

set(IO_SOURCES
	${SOURCE_DIR}/io/CameraPathIO.h
	${SOURCE_DIR}/io/CameraPathIO.cpp
	${SOURCE_DIR}/io/MemoryMappedFile.h
	${SOURCE_DIR}/io/MemoryMappedFile.inl
	${SOURCE_DIR}/io/MemoryMappedFile.cpp
	${SOURCE_DIR}/io/OOCAttributeLoader.h
	${SOURCE_DIR}/io/OOCAttributeLoader.inl
	${SOURCE_DIR}/io/OOCPointCloudLoader.h
	${SOURCE_DIR}/io/OOCPointCloudLoader.cpp
	${SOURCE_DIR}/io/PLYLoader.h
	${SOURCE_DIR}/io/PLYLoader.inl
	${SOURCE_DIR}/io/PLYLoader.cpp
	${SOURCE_DIR}/io/UnbufferedFile.h
	${SOURCE_DIR}/io/UnbufferedFile.inl
	${SOURCE_DIR}/io/UnbufferedFile.cpp
)

source_group("Main" FILES ${CORE_SOURCES})
source_group("Geometry" FILES ${GEOMETRY_SOURCES})
source_group("Navigation" FILES ${NAVIGATION_SOURCES})
source_group("Rendering" FILES ${GENERAL_RENDERING_SOURCES})
source_group("Rendering/Parameters" FILES ${PARAMETERS_SOURCES})
source_group("Rendering/PointcloudProviders" FILES ${POINTCLOUD_PROVIDERS_SOURCES})
source_group("Rendering/Rasterization" FILES ${RASTERIZATION_SOURCES})
source_group("Rendering/Colorization" FILES ${COLORIZATION_SOURCES})
source_group("Rendering/Postprocessing" FILES ${POSTPROCESSING_SOURCES})
source_group("Utils" FILES ${UTILS_SOURCES})
source_group("IO" FILES ${IO_SOURCES})
source_group("UI" FILES ${GENERAL_UI_SOURCES})
source_group("UI/CameraPanels" FILES ${CAMERA_PANELS_SOURCES})
source_group("UI/ParameterWidgets" FILES ${PARAMETER_WIDGETS_SOURCES})
source_group("UI/RendererSettingsPanel" FILES ${RENDERER_SETTINGS_PANEL_SOURCES})
source_group("UI/ScenePanel" FILES ${SCENE_PANEL_SOURCES})
source_group("Automoc" FILES)

set(ALL_SOURCES 
	${GEOMETRY_SOURCES} 
	${VIEW_SOURCES}
	${CAMERA_PANELS_SOURCES}
	${PARAMETER_WIDGETS_SOURCES}
	${RENDERER_SETTINGS_PANEL_SOURCES}
	${SCENE_PANEL_SOURCES}
	${GENERAL_UI_SOURCES}
	${NAVIGATION_SOURCES}
	${GENERAL_RENDERING_SOURCES} 
	${PARAMETERS_SOURCES}
	${POINTCLOUD_PROVIDERS_SOURCES}
	${RASTERIZATION_SOURCES}
	${COLORIZATION_SOURCES}
	${POSTPROCESSING_SOURCES}
	${UTILS_SOURCES} 
	${IO_SOURCES} 
	${CORE_SOURCES})


set(LIBS OpenGL::GL Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::Concurrent TINYPLY)

qt_standard_project_setup()

# Add sources, includes, libs to executable
add_executable(Sahara ${ALL_SOURCES} ${CMAKE_SOURCE_DIR}/data/resources.qrc)

target_include_directories(Sahara PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(Sahara PRIVATE ${LIBS})

# Set filters for placing the files generated by QT
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

# Set working directory for the application
set_target_properties(Sahara PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)
set_target_properties(Sahara PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(Sahara PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${QT_ROOT}/bin\\\;${QT_ROOT}/plugins/platforms")

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

install(TARGETS Sahara
	 BUNDLE DESTINATION .
	 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
 )

 qt_generate_deploy_app_script(
	 TARGET Sahara
	 OUTPUT_SCRIPT deploy_script
	 NO_UNSUPPORTED_PLATFORM_ERROR
 )

 install(SCRIPT ${deploy_script})

if(BUILD_CLANG_FORMAT_TARGET)
	include(cmake/clang-tools.cmake)
endif()

if(ENABLE_PROFILING)
	add_compile_definitions(PROFILER_ENABLED)
endif()


if(BYPASS_OS_FILE_CACHE)
	add_compile_definitions(BYPASS_OS_FILE_CACHE)
endif()
