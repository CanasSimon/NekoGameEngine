
if (WIN32)
	file(GLOB_RECURSE FMOD_SRC *.h *.hpp)
	add_library(fmod INTERFACE ${FMOD_SRC})

    file(GLOB_RECURSE FMOD_LIBS *.lib)
    target_include_directories(fmod INTERFACE "win_x64/inc/")

	file(GLOB_RECURSE FMOD_DLLS *.dll)
	set(TEST_EXE_PATH ${CMAKE_BINARY_DIR}/common/aer_lib/)
	set(GAME_EXE_PATH ${CMAKE_BINARY_DIR}/main/aer_game/)
	set(EDITOR_EXE_PATH ${CMAKE_BINARY_DIR}/main/aer_editor/)
	add_custom_target(fmodCopyBinaries
			COMMENT "Copying FMOD dlls to the target's binaries" VERBATIM

			# test debug version
			COMMAND ${CMAKE_COMMAND} -E copy ${FMOD_DLLS} ${TEST_EXE_PATH}/Debug

			# test debug version
			COMMAND ${CMAKE_COMMAND} -E copy ${FMOD_DLLS} ${TEST_EXE_PATH}/Release

			# editor debug version
			COMMAND ${CMAKE_COMMAND} -E copy ${FMOD_DLLS} ${EDITOR_EXE_PATH}/Debug

			# editor debug version
			COMMAND ${CMAKE_COMMAND} -E copy ${FMOD_DLLS} ${EDITOR_EXE_PATH}/Release

			# game debug version
			COMMAND ${CMAKE_COMMAND} -E copy ${FMOD_DLLS} ${GAME_EXE_PATH}/Debug

			# game debug version
			COMMAND ${CMAKE_COMMAND} -E copy ${FMOD_DLLS} ${GAME_EXE_PATH}/Release)

	set_target_properties(fmod PROPERTIES FOLDER Externals)
    target_link_libraries(fmod INTERFACE ${FMOD_LIBS} Neko_Core)

	add_dependencies(fmod fmodCopyBinaries)
	set_target_properties(fmodCopyBinaries PROPERTIES FOLDER Externals)
elseif (UNIX)
    file(GLOB_RECURSE FMOD_SRC *.h *.hpp)
    add_library(fmod STATIC ${FMOD_SRC})

    file(GLOB_RECURSE FMOD_LIBS *.so *.so.12)
    target_link_libraries(fmod PUBLIC ${FMOD_LIBS})
    target_include_directories(fmod PUBLIC "linux_x86_64/inc/")

	set_target_properties(fmod PROPERTIES FOLDER Externals)
	target_link_libraries(fmod PRIVATE Neko_Core)
endif ()