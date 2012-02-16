#	install(FILES
#		${FFTW3F_LIBRARY} ${SNDFILE_LIBRARY} ${PTHREADS_LIBRARY}
#		${PORTAUDIO_LIBRARIES} ${READLINE_LIBRARY}
#		DESTINATION "SuperCollider"
#		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

if(SC_WIN_BUNDLE)
    install(FILES ${CMAKE_SOURCE_DIR}/COPYING DESTINATION SuperCollider)

    file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} NATIVE_INSTALL_PREFIX)
    #configure_file( supercollider.in.nsi supercollider.nsi @ONLY )

    if(SC_ED)
        set(DEFINE_SC_ED "/DSC_ED")
    endif()

    install(CODE "
        include(BundleUtilities)
        fixup_bundle(
            \"${CMAKE_INSTALL_PREFIX}/SuperCollider/sclang.exe\"
            \"\"
            \"${SC_WIN_DEP_DIRS}\"
        )

        #if(FALSE)
        execute_process( COMMAND makensis
            /DSC_SRC_DIR=\"${NATIVE_INSTALL_PREFIX}\"
            # FIXME: Does not support spaces in path:
            /DSC_DST_DIR=${NATIVE_INSTALL_PREFIX}
            ${DEFINE_SC_ED}
            ${CMAKE_CURRENT_SOURCE_DIR}\\supercollider.nsi
        )
        #endif()
    ")
endif()

