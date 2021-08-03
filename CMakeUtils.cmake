# Enables ADD_GIT_DEPENDENCY functionality
option(ENABLE_GIT_DEPENDENCIES "Enables automatic cloning of dependencies that do not already exit" OFF)


find_package(Git QUIET)




#
#	Adds a new dependency and automatically clones it if the target does not already exist.
#
#	@param depPath Where to clone the repo into, must be an absolute path!
#	@param depTarget Name of the target, this is used to check if the dependency already exists
#	@param depRepo Path or URL to clone the repo from
#	@param branchName? Name of the branch to clone, defaults to HEAD
#
function(ADD_GIT_DEPENDENCY_FN depPath depTarget depRepo)

	# Ignore if target already exists
	if (NOT TARGET ${depTarget})
	 
		# Only preform branch if git dependencies are allowed
		if (ENABLE_GIT_DEPENDENCIES)
		
			set(gitResult )
			
			# Check result
			message(STATUS ${depPath}/CMakeLists.txt)
			
			# Add the cloned repo as a subdir if it has CMake support
			if (EXISTS "${depPath}/CMakeLists.txt")
				add_subdirectory("${depPath}")
			
				# Check that dependency target is now defined
				if (NOT TARGET ${depTarget})
					message(FATAL "Cloned dependency has a CMakeLists but the dependency target was not defined!")
				endif()
				
			else()

				# Use branch optional parameter if it was provided
				if (ARGC GREATER 3)
					execute_process(COMMAND
						${GIT_EXECUTABLE} clone -b "${ARGV3}" ${depRepo} ${depPath}
						RESULTS_VARIABLE gitResult
						COMMAND_ERROR_IS_FATAL ANY)
				else()
					execute_process(COMMAND
						${GIT_EXECUTABLE} clone ${depRepo} ${depPath}
						RESULTS_VARIABLE gitResult
						COMMAND_ERROR_IS_FATAL ANY)
				endif()

			endif()

		endif()

	endif()
endfunction()

#
#	Adds a new dependency and automatically clones it if the target does not already exist.
#
#	@param depPath Relative path to clone the repo into
#	@param depTarget Name of the target, this is used to check if the dependency already exists
#	@param depRepo Path or URL to clone the repo from
#	@param branchName? Name of the branch to clone, defaults to HEAD
#
macro(ADD_GIT_DEPENDENCY depPath depTarget depRepo)

	# Make file path absolute
	set(__addgitdepedency_realpath )
	file(REAL_PATH ${depPath} __addgitdepedency_realpath)
	
	# Determine invocation syntax
	if (${ARGC} GREATER 3)
		# Invoke with branchName parameter
		ADD_GIT_DEPENDENCY_FN(${__addgitdepedency_realpath} ${depTarget} ${depRepo} ${ARGV3})
	else()
		# Invoke without branchName parameter
		ADD_GIT_DEPENDENCY_FN(${__addgitdepedency_realpath} ${depTarget} ${depRepo})
	endif()

endmacro()


#
#	Adds a list of sources to a target, sourceList should be a list variable
#
macro(ADD_SOURCES_LIST targetName sourceList)
	list(TRANSFORM ${sourceList} PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
	set(lfefiles )
	foreach(lfe IN LISTS ${sourceList})
		set(lfefiles ${lfefiles} ${lfe})
	endforeach()
	target_sources(${targetName} PRIVATE ${lfefiles})
endmacro(ADD_SOURCES_LIST)

#
#	Returns the child paths of a given directory
#
macro(SUBDIRLIST result curdir)
	file(GLOB children RELATIVE ${curdir} ${curdir}/*)
	set(dirlist "")
	foreach(child ${children})
		if(IS_DIRECTORY ${curdir}/${child})
			list(APPEND dirlist ${child})
		endif()
	endforeach()
	set(${result} ${dirlist})
endmacro()

#
#	Adds a list of subdirectories to the project, pathList should be a list variable
#
macro(ADD_SUBDIRS_LIST pathList)
	foreach(lfe IN LISTS ${pathList})
		add_subdirectory(${lfe})
	endforeach()
endmacro(ADD_SUBDIRS_LIST)

#
#	Includes all subdirectories from the current source path
#
macro(ADD_SUBDIRS_HERE)
	set(dirlist )
	SUBDIRLIST(dirlist ${CMAKE_CURRENT_SOURCE_DIR})
	foreach(lfe IN LISTS dirlist)
		set(lfename )		
		get_filename_component(lfename ${lfe} NAME)
		add_subdirectory(${lfename})
	endforeach()
endmacro()

#
#	Includes all paths listed that contain CMake lists
#
#	@param rootDir Root directory path
#
function(ADD_CMAKE_SUBDIRS_FN rootDir)

	# Get subdirectories
	set(subdirList )
	SUBDIRLIST(subdirList ${rootDir})

	# Include each subdir if it has a cmake lists
	foreach(subd IN LISTS subdirList)
		if (EXISTS "${rootDir}/${subd}/CMakeLists.txt")
			add_subdirectory("${rootDir}/${subd}")
		endif()
	endforeach()

endfunction()

#
#	Includes all subdirectories containing CMake lists from the current source dir
#
macro(ADD_CMAKE_SUBDIRS_HERE)
	ADD_CMAKE_SUBDIRS_FN("${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()
