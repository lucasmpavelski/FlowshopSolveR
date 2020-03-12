##########################################################################################
### Doc generation using Doxygen
##########################################################################################

if (DOXYGEN_FOUND)

	set(DOC_DIR ${CMAKE_BINARY_DIR}/doc CACHE PATH "Documentation directory")
	set(DOC_CONFIG_FILE "doxyfile" CACHE PATH "Documentation configuration file")
		# define the doc target
	if (DOXYGEN_EXECUTABLE)
	    	add_custom_target(doc
	        COMMAND ${DOXYGEN_EXECUTABLE} ${MOEO_DOC_CONFIG_FILE}
	        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	    )
	endif (DOXYGEN_EXECUTABLE)
	
	# configure doxyfile file
	configure_file("${CMAKE_SOURCE_DIR}/doc/${DOC_CONFIG_FILE}.cmake"
	               "${CMAKE_BINARY_DIR}/doc/${DOC_CONFIG_FILE}")

else (DOXYGEN_FOUND)
	message(STATUS "Unable to generate the documentation, Doxygen package not found")	
endif (DOXYGEN_FOUND)

##########################################################################################
