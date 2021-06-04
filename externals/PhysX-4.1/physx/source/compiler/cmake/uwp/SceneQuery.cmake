##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##  * Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
##  * Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
##  * Neither the name of NVIDIA CORPORATION nor the names of its
##    contributors may be used to endorse or promote products derived
##    from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
## EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
## CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
## EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
## PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
## PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
## OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## Copyright (c) 2018-2019 NVIDIA Corporation. All rights reserved.

#
# Build SceneQuery
#


SET(SCENEQUERY_PLATFORM_INCLUDES
	PRIVATE ${PHYSX_SOURCE_DIR}/Common/src/windows
)

# Use generator expressions to set config specific preprocessor definitions
SET(SCENEQUERY_COMPILE_DEFS

	# Common to all configurations
	${PHYSX_UWP_COMPILE_DEFS};${PHYSX_LIBTYPE_DEFS};

	$<$<CONFIG:Debug>:${PHYSX_UWP_DEBUG_COMPILE_DEFS};>
	$<$<CONFIG:Checked>:${PHYSX_UWP_CHECKED_COMPILE_DEFS};>
	$<$<CONFIG:Profile>:${PHYSX_UWP_PROFILE_COMPILE_DEFS};>
	$<$<CONFIG:Release>:${PHYSX_UWP_RELEASE_COMPILE_DEFS};>
)

IF(PX_GENERATE_STATIC_LIBRARIES)
	SET(SCENEQUERY_LIBTYPE OBJECT)
ELSE()
	SET(SCENEQUERY_LIBTYPE STATIC)
ENDIF()

IF(NV_USE_GAMEWORKS_OUTPUT_DIRS AND SCENEQUERY_LIBTYPE STREQUAL "STATIC")
	SET(SQ_COMPILE_PDB_NAME_DEBUG "SceneQuery_static${CMAKE_DEBUG_POSTFIX}")
	SET(SQ_COMPILE_PDB_NAME_CHECKED "SceneQuery_static${CMAKE_CHECKED_POSTFIX}")
	SET(SQ_COMPILE_PDB_NAME_PROFILE "SceneQuery_static${CMAKE_PROFILE_POSTFIX}")
	SET(SQ_COMPILE_PDB_NAME_RELEASE "SceneQuery_static${CMAKE_RELEASE_POSTFIX}")
ELSE()
	SET(SQ_COMPILE_PDB_NAME_DEBUG "SceneQuery${CMAKE_DEBUG_POSTFIX}")
	SET(SQ_COMPILE_PDB_NAME_CHECKED "SceneQuery${CMAKE_CHECKED_POSTFIX}")
	SET(SQ_COMPILE_PDB_NAME_PROFILE "SceneQuery${CMAKE_PROFILE_POSTFIX}")
	SET(SQ_COMPILE_PDB_NAME_RELEASE "SceneQuery${CMAKE_RELEASE_POSTFIX}")
ENDIF()

IF(PX_EXPORT_LOWLEVEL_PDB)
	SET(SCENEQUERY_COMPILE_PDB_NAME_DEBUG "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Debug/${SQ_COMPILE_PDB_NAME_DEBUG}")
	SET(SCENEQUERY_COMPILE_PDB_NAME_CHECKED "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Checked/${SQ_COMPILE_PDB_NAME_CHECKED}")
	SET(SCENEQUERY_COMPILE_PDB_NAME_PROFILE "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Profile/${SQ_COMPILE_PDB_NAME_PROFILE}")
	SET(SCENEQUERY_COMPILE_PDB_NAME_RELEASE "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Release/${SQ_COMPILE_PDB_NAME_RELEASE}")

	INSTALL(FILES ${PHYSX_ROOT_DIR}/$<$<CONFIG:Debug>:${PX_ROOT_LIB_DIR}/Debug>$<$<CONFIG:Release>:${PX_ROOT_LIB_DIR}/Release>$<$<CONFIG:Checked>:${PX_ROOT_LIB_DIR}/Checked>$<$<CONFIG:Profile>:${PX_ROOT_LIB_DIR}/Profile>/$<$<CONFIG:Debug>:${SQ_COMPILE_PDB_NAME_DEBUG}>$<$<CONFIG:Checked>:${SQ_COMPILE_PDB_NAME_CHECKED}>$<$<CONFIG:Profile>:${SQ_COMPILE_PDB_NAME_PROFILE}>$<$<CONFIG:Release>:${SQ_COMPILE_PDB_NAME_RELEASE}>.pdb
		DESTINATION $<$<CONFIG:Debug>:${PX_ROOT_LIB_DIR}/Debug>$<$<CONFIG:Release>:${PX_ROOT_LIB_DIR}/Release>$<$<CONFIG:Checked>:${PX_ROOT_LIB_DIR}/Checked>$<$<CONFIG:Profile>:${PX_ROOT_LIB_DIR}/Profile> OPTIONAL)	
ELSE()
	SET(SCENEQUERY_COMPILE_PDB_NAME_DEBUG "${SQ_COMPILE_PDB_NAME_DEBUG}")
	SET(SCENEQUERY_COMPILE_PDB_NAME_CHECKED "${SQ_COMPILE_PDB_NAME_CHECKED}")
	SET(SCENEQUERY_COMPILE_PDB_NAME_PROFILE "${SQ_COMPILE_PDB_NAME_PROFILE}")
	SET(SCENEQUERY_COMPILE_PDB_NAME_RELEASE "${SQ_COMPILE_PDB_NAME_RELEASE}")
ENDIF()