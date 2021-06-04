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
# Build LowLevelAABB
#

SET(LOWLEVELAABB_PLATFORM_INCLUDES
	${PHYSX_SOURCE_DIR}/Common/src/windows
	${PHYSX_SOURCE_DIR}/LowLevelAABB/windows/include
	${PHYSX_SOURCE_DIR}/GpuBroadPhase/include
	${PHYSX_SOURCE_DIR}/GpuBroadPhase/src
)

IF(PX_GENERATE_STATIC_LIBRARIES)
	SET(LOWLEVELAABB_LIBTYPE OBJECT)
ELSE()
	SET(LOWLEVELAABB_LIBTYPE STATIC)
ENDIF()

SET(LOWLEVELAABB_COMPILE_DEFS

	# Common to all configurations
	${PHYSX_WINDOWS_COMPILE_DEFS};${PHYSX_LIBTYPE_DEFS};${PHYSXGPU_LIBTYPE_DEFS}

	$<$<CONFIG:Debug>:${PHYSX_WINDOWS_DEBUG_COMPILE_DEFS};>
	$<$<CONFIG:Checked>:${PHYSX_WINDOWS_CHECKED_COMPILE_DEFS};>
	$<$<CONFIG:Profile>:${PHYSX_WINDOWS_PROFILE_COMPILE_DEFS};>
	$<$<CONFIG:Release>:${PHYSX_WINDOWS_RELEASE_COMPILE_DEFS};>
)

IF(NV_USE_GAMEWORKS_OUTPUT_DIRS AND LOWLEVELAABB_LIBTYPE STREQUAL "STATIC")
	SET(LLAABB_COMPILE_PDB_NAME_DEBUG "LowLevelAABB_static${CMAKE_DEBUG_POSTFIX}")
	SET(LLAABB_COMPILE_PDB_NAME_CHECKED "LowLevelAABB_static${CMAKE_CHECKED_POSTFIX}")
	SET(LLAABB_COMPILE_PDB_NAME_PROFILE "LowLevelAABB_static${CMAKE_PROFILE_POSTFIX}")
	SET(LLAABB_COMPILE_PDB_NAME_RELEASE "LowLevelAABB_static${CMAKE_RELEASE_POSTFIX}")
ELSE()
	SET(LLAABB_COMPILE_PDB_NAME_DEBUG "LowLevelAABB${CMAKE_DEBUG_POSTFIX}")
	SET(LLAABB_COMPILE_PDB_NAME_CHECKED "LowLevelAABB${CMAKE_CHECKED_POSTFIX}")
	SET(LLAABB_COMPILE_PDB_NAME_PROFILE "LowLevelAABB${CMAKE_PROFILE_POSTFIX}")
	SET(LLAABB_COMPILE_PDB_NAME_RELEASE "LowLevelAABB${CMAKE_RELEASE_POSTFIX}")
ENDIF()

IF(PX_EXPORT_LOWLEVEL_PDB)
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_DEBUG "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Debug/${LLAABB_COMPILE_PDB_NAME_DEBUG}")
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_CHECKED "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Checked/${LLAABB_COMPILE_PDB_NAME_CHECKED}")
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_PROFILE "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Profile/${LLAABB_COMPILE_PDB_NAME_PROFILE}")
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_RELEASE "${PHYSX_ROOT_DIR}/${PX_ROOT_LIB_DIR}/Release/${LLAABB_COMPILE_PDB_NAME_RELEASE}")

	INSTALL(FILES ${PHYSX_ROOT_DIR}/$<$<CONFIG:Debug>:${PX_ROOT_LIB_DIR}/Debug>$<$<CONFIG:Release>:${PX_ROOT_LIB_DIR}/Release>$<$<CONFIG:Checked>:${PX_ROOT_LIB_DIR}/Checked>$<$<CONFIG:Profile>:${PX_ROOT_LIB_DIR}/Profile>/$<$<CONFIG:Debug>:${LLAABB_COMPILE_PDB_NAME_DEBUG}>$<$<CONFIG:Checked>:${LLAABB_COMPILE_PDB_NAME_CHECKED}>$<$<CONFIG:Profile>:${LLAABB_COMPILE_PDB_NAME_PROFILE}>$<$<CONFIG:Release>:${LLAABB_COMPILE_PDB_NAME_RELEASE}>.pdb
		DESTINATION $<$<CONFIG:Debug>:${PX_ROOT_LIB_DIR}/Debug>$<$<CONFIG:Release>:${PX_ROOT_LIB_DIR}/Release>$<$<CONFIG:Checked>:${PX_ROOT_LIB_DIR}/Checked>$<$<CONFIG:Profile>:${PX_ROOT_LIB_DIR}/Profile> OPTIONAL)	
ELSE()
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_DEBUG "${LLAABB_COMPILE_PDB_NAME_DEBUG}")
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_CHECKED "${LLAABB_COMPILE_PDB_NAME_CHECKED}")
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_PROFILE "${LLAABB_COMPILE_PDB_NAME_PROFILE}")
	SET(LOWLEVELAABB_COMPILE_PDB_NAME_RELEASE "${LLAABB_COMPILE_PDB_NAME_RELEASE}")
ENDIF()

SET(LOWLEVELAABB_PLATFORM_LINK_FLAGS "/MAP")

