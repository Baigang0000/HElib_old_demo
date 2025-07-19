# Install script for directory: /home/baigang/HElib_old_demo/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/baigang/HElib_old_demo/benchmarks/lib/libhelib.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/helib" TYPE FILE FILES
    "/home/baigang/HElib_old_demo/include/helib/helib.h"
    "/home/baigang/HElib_old_demo/include/helib/apiAttributes.h"
    "/home/baigang/HElib_old_demo/include/helib/ArgMap.h"
    "/home/baigang/HElib_old_demo/include/helib/binaryArith.h"
    "/home/baigang/HElib_old_demo/include/helib/binaryCompare.h"
    "/home/baigang/HElib_old_demo/include/helib/bluestein.h"
    "/home/baigang/HElib_old_demo/include/helib/ClonedPtr.h"
    "/home/baigang/HElib_old_demo/include/helib/CModulus.h"
    "/home/baigang/HElib_old_demo/include/helib/CtPtrs.h"
    "/home/baigang/HElib_old_demo/include/helib/Ctxt.h"
    "/home/baigang/HElib_old_demo/include/helib/debugging.h"
    "/home/baigang/HElib_old_demo/include/helib/DoubleCRT.h"
    "/home/baigang/HElib_old_demo/include/helib/EncryptedArray.h"
    "/home/baigang/HElib_old_demo/include/helib/EvalMap.h"
    "/home/baigang/HElib_old_demo/include/helib/Context.h"
    "/home/baigang/HElib_old_demo/include/helib/FHE.h"
    "/home/baigang/HElib_old_demo/include/helib/keys.h"
    "/home/baigang/HElib_old_demo/include/helib/keySwitching.h"
    "/home/baigang/HElib_old_demo/include/helib/log.h"
    "/home/baigang/HElib_old_demo/include/helib/hypercube.h"
    "/home/baigang/HElib_old_demo/include/helib/IndexMap.h"
    "/home/baigang/HElib_old_demo/include/helib/IndexSet.h"
    "/home/baigang/HElib_old_demo/include/helib/intraSlot.h"
    "/home/baigang/HElib_old_demo/include/helib/JsonWrapper.h"
    "/home/baigang/HElib_old_demo/include/helib/matching.h"
    "/home/baigang/HElib_old_demo/include/helib/matmul.h"
    "/home/baigang/HElib_old_demo/include/helib/Matrix.h"
    "/home/baigang/HElib_old_demo/include/helib/multicore.h"
    "/home/baigang/HElib_old_demo/include/helib/norms.h"
    "/home/baigang/HElib_old_demo/include/helib/NumbTh.h"
    "/home/baigang/HElib_old_demo/include/helib/PAlgebra.h"
    "/home/baigang/HElib_old_demo/include/helib/partialMatch.h"
    "/home/baigang/HElib_old_demo/include/helib/permutations.h"
    "/home/baigang/HElib_old_demo/include/helib/polyEval.h"
    "/home/baigang/HElib_old_demo/include/helib/PolyMod.h"
    "/home/baigang/HElib_old_demo/include/helib/PolyModRing.h"
    "/home/baigang/HElib_old_demo/include/helib/powerful.h"
    "/home/baigang/HElib_old_demo/include/helib/primeChain.h"
    "/home/baigang/HElib_old_demo/include/helib/PtrMatrix.h"
    "/home/baigang/HElib_old_demo/include/helib/PtrVector.h"
    "/home/baigang/HElib_old_demo/include/helib/Ptxt.h"
    "/home/baigang/HElib_old_demo/include/helib/query.h"
    "/home/baigang/HElib_old_demo/include/helib/randomMatrices.h"
    "/home/baigang/HElib_old_demo/include/helib/range.h"
    "/home/baigang/HElib_old_demo/include/helib/recryption.h"
    "/home/baigang/HElib_old_demo/include/helib/replicate.h"
    "/home/baigang/HElib_old_demo/include/helib/sample.h"
    "/home/baigang/HElib_old_demo/include/helib/scheme.h"
    "/home/baigang/HElib_old_demo/include/helib/set.h"
    "/home/baigang/HElib_old_demo/include/helib/SumRegister.h"
    "/home/baigang/HElib_old_demo/include/helib/tableLookup.h"
    "/home/baigang/HElib_old_demo/include/helib/timing.h"
    "/home/baigang/HElib_old_demo/include/helib/zzX.h"
    "/home/baigang/HElib_old_demo/include/helib/assertions.h"
    "/home/baigang/HElib_old_demo/include/helib/exceptions.h"
    "/home/baigang/HElib_old_demo/include/helib/PGFFT.h"
    "/home/baigang/HElib_old_demo/include/helib/fhe_stats.h"
    "/home/baigang/HElib_old_demo/include/helib/zeroValue.h"
    "/home/baigang/HElib_old_demo/include/helib/EncodedPtxt.h"
    "/home/baigang/HElib_old_demo/benchmarks/src/helib/version.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/cmake/helib/helibTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/cmake/helib/helibTargets.cmake"
         "/home/baigang/HElib_old_demo/benchmarks/src/CMakeFiles/Export/ab2aafcc98cb43a5e26f9343848858d7/helibTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/cmake/helib/helibTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/cmake/helib/helibTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/cmake/helib" TYPE FILE FILES "/home/baigang/HElib_old_demo/benchmarks/src/CMakeFiles/Export/ab2aafcc98cb43a5e26f9343848858d7/helibTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/cmake/helib" TYPE FILE FILES "/home/baigang/HElib_old_demo/benchmarks/src/CMakeFiles/Export/ab2aafcc98cb43a5e26f9343848858d7/helibTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/cmake/helib" TYPE FILE FILES
    "/home/baigang/HElib_old_demo/benchmarks/src/helibConfig.cmake"
    "/home/baigang/HElib_old_demo/benchmarks/src/helibConfigVersion.cmake"
    )
endif()

