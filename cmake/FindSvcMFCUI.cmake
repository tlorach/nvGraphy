# Try to find SvcMFCUI project dll and include file
#
unset(SVCMFCUI_DLL CACHE)
unset(SVCMFCUI_INCLUDE_DIR CACHE)
unset(SVCMFCUI_FOUND CACHE)

find_path( SVCMFCUI_INCLUDE_DIR ISvcUI.h
  ${SVCMFCUI_LOCATION}/include
  $ENV{SVCMFCUI_LOCATION}/include
  ${PROJECT_SOURCE_DIR}/../SvcMFCUI/include
  ${SVCMFCUI_LOCATION}/include
  $ENV{SVCMFCUI_LOCATION}/include
  ${PROJECT_SOURCE_DIR}/../SvcMFCUI/include
)

if(ARCH STREQUAL "x86")
    file(GLOB SVCMFCUI_DLLS "${SVCMFCUI_INCLUDE_DIR}/../dll/SvcMFCUI_*.dll")
    #${SVCMFCUI_LOCATION}/dll
    #$ENV{SVCMFCUI_LOCATION}/dll
    #${PROJECT_SOURCE_DIR}/../SvcMFCUI/dll
else()
    file(GLOB SVCMFCUI_DLLS "${SVCMFCUI_INCLUDE_DIR}/../dll/SvcMFCUI64_*.dll")
    #${SVCMFCUI_LOCATION}/dll
    #$ENV{SVCMFCUI_LOCATION}/dll
    #${PROJECT_SOURCE_DIR}/../SvcMFCUI/dll
endif()

set( SVCMFCUI_FOUND "NO" )

list(LENGTH SVCMFCUI_DLLS NUMDLLS)
if(NUMDLLS EQUAL 0)
  message(WARNING "dll for the User Interface not found. Please compile SvcMFCUI" )
  set (SVCMFCUI_DLL "NOTFOUND")
else()
  list(GET SVCMFCUI_DLLS 0 SVCMFCUI_DLL)
endif()

if(SVCMFCUI_INCLUDE_DIR)
  if(SVCMFCUI_DLL)
    set( SVCMFCUI_FOUND "YES" )
  endif(SVCMFCUI_DLL)
endif(SVCMFCUI_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SVCMFCUI DEFAULT_MSG
    SVCMFCUI_INCLUDE_DIR
    SVCMFCUI_DLL
)
# I duno why I have to rewrite the variable here...
SET(SVCMFCUI_DLL ${SVCMFCUI_DLL} CACHE PATH "path")

mark_as_advanced( SVCMFCUI_FOUND )
