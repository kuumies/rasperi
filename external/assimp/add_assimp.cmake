#---------------------------------------------------------------------
# Add Assimp

set(ASSIMP_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories(${ASSIMP_DIR}/include)

if (MSVC)
    set(ASSIMP_LIBRARY_DIR ${ASSIMP_DIR}/lib/msvc2017)
    set(ASSIMP_BINARY_DIR ${ASSIMP_DIR}/bin)
    if (IS_DEBUG)
        set(ASSIMP_LIBRARIES assimp-vc140-mtd.lib)
        install(
            FILES
            ${ASSIMP_BINARY_DIR}/assimp-vc140-mtd.dll
            DESTINATION bin
        )
    else(IS_DEBUG)
        set(ASSIMP_LIBRARIES assimp-vc140-mt.lib)
        install(
            FILES
            ${ASSIMP_BINARY_DIR}/assimp-vc140-mt.dll
            DESTINATION bin
        )
    endif(IS_DEBUG)
endif (MSVC)

link_directories(${ASSIMP_LIBRARY_DIR})
