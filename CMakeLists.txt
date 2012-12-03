
cmake_minimum_required (VERSION 2.6)
project(recd)

include_directories( ./include/ )
include_directories( ../../FEDLibrary/source/template_include/ )
include_directories( ../../FEDLibrary/source/include/ )
include_directories( ../libavcpp/include )

IF(NOT CMAKE_BUILD_TYPE )
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
ENDIF()

IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  set(  RECD_VERSION
        -debug
     )
  set(  RECD_CLI_VERSION
        -2.0.1-debug
     )
  set(  LIBAVCPP_VER
	-3.1.0-debug
    )
  set(  FEDLIB_VER
        -3.0.1-debug
     )
ELSE()
  set(  RECD_VERSION
     )
  set(  RECD_CLI_VERSION
        -2.0.1
     )
  set(  LIBAVCPP_VER
	-3.1.0
    )
  set(  FEDLIB_VER
        -3.0.1
     )
ENDIF()


set(  RECD_INCLUDE 
        ./include/IRecdCommand.h
        ./include/Recd.h
        ./include/RecdChannel.h
        ./include/RecdChannelFactory.h
        ./include/RecdCmdServer.h
        ./include/RecdConfig.h
        ./include/RecdConfigException.h
        ./include/RecdEvents.h
        ./include/RecdResult.h
        ./include/RecdAudioReader.h
        ./include/RecdStreamReader.h
        ./include/RecdReaderCollector.h
        ./include/RecdRenderEncoder.h
        ./include/RecdStreamEncoder.h
        ./include/RecdEncoderCollector.h
        ./include/RecdHighLightsEncoder.h
        ./include/COMMANDS/RecdList.h
        ./include/COMMANDS/RecdHelp.h
        ./include/COMMANDS/RecdGetVersion.h
        ./include/COMMANDS/RecdGetBuffers.h
        ./include/COMMANDS/RecdGetDiskSize.h
        ./include/COMMANDS/RecdStartRecording.h
        ./include/COMMANDS/RecdStopRecording.h
        ./include/COMMANDS/RecdStartHighLights.h
        ./include/COMMANDS/RecdEstimateTime.h
)

set(  RECD_CLI_INCLUDE
        ./include/RecdParser.h
        ./include/RecdClient.h
)

set(  RECD_SRC   
        ./src/main.cpp
        ./src/Recd.cpp
        ./src/RecdChannel.cpp
        ./src/RecdChannelFactory.cpp
        ./src/RecdCmdServer.cpp
        ./src/RecdConfig.cpp
        ./src/RecdConfigException.cpp
        ./src/RecdEvents.cpp
        ./src/RecdRenderEncoder.cpp
        ./src/RecdAudioReader.cpp
        ./src/RecdStreamReader.cpp
        ./src/RecdReaderCollector.cpp
        ./src/RecdStreamEncoder.cpp
        ./src/RecdEncoderCollector.cpp
        ./src/RecdHighLightsEncoder.cpp
)

set(  RECD_CLI_SRC
        ./src/RecdParser.cpp
        ./src/RecdClient.cpp
)

set(  RECD_CLI_LIBRARIES
      pthread
      fedlibrary${FEDLIB_VER}
)

set(  RECD_LIBRARIES
      pthread
      libavcpp${LIBAVCPP_VER}
      fedlibrary${FEDLIB_VER}
)

set(  CAPTURE_LIBRARIES
      pthread
      libavcpp${LIBAVCPP_VER}
      fedlibrary${FEDLIB_VER}
)

IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
add_definitions(
                -ggdb3
)
ENDIF()

add_definitions( 
		  -D__STDC_CONSTANT_MACROS
		  -D_NO_FED_NAMESPACE
		  -D_TRACING_EXCEPTION_RAISE
		  -D_TRACING_EXCEPTION_CATCH
		  -D_ENABLE_ERROR_INFO
		  -D_ENABLE_LOG_INFO
		  -D_ENABLE_RAW_INFO
		  -D_ENABLE_METHOD_TRACING
		  -D_ENABLE_VERBOSE_INFO 
		  -D_ENABLE_ASSERTION_TRACING
		)


add_library   ( recdclient${RECD_CLI_VERSION}  SHARED ${RECD_CLI_SRC}    )
add_executable( recd${RECD_VERSION}                   ${RECD_SRC}        )
add_executable( capture                               utils/capture.cpp  )

target_link_libraries( recdclient${RECD_CLI_VERSION}  ${RECD_CLI_LIBRARIES} )
target_link_libraries( recd${RECD_VERSION}            ${RECD_LIBRARIES}     )
target_link_libraries( capture                        ${CAPTURE_LIBRARIES}  )


install( TARGETS recdclient${RECD_CLI_VERSION}  LIBRARY DESTINATION /usr/local/lib )
install( FILES   ${RECD_CLI_INCLUDE} DESTINATION /usr/include/librecdcli )
