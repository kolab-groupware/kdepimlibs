
project(kabc)

include_directories(${KDE4_KIO_INCLUDES})

add_definitions(${QDBUS_DEFINITIONS} -DKDE_DEFAULT_DEBUG_AREA=5700)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(
  -DKRESOURCES_DEPRECATED=
  -DKABC_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKRESOURCES_DEPRECATED_EXPORT=KRESOURCES_EXPORT
  -DKABC_DEPRECATED_EXPORT=KABC_EXPORT
)

# these apply also for all subdirs
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/vcardparser
  ${CMAKE_BINARY_DIR}/kresources
)

# kabc/vcardparser/Makefile.am: vcards

set(vcards_STAT_SRCS
  vcardparser/vcard.cpp
  vcardparser/vcardline.cpp
  vcardparser/vcardparser.cpp
)

add_subdirectory(vcardparser)
if(NOT KDEPIM_NO_KRESOURCES)
  add_subdirectory(formats)
  add_subdirectory(plugins)
endif()
add_subdirectory(tests)

########### next target ###############

set(kabc_LIB_SRCS
  address.cpp
  addressee.cpp
  contactgroup.cpp
  contactgrouptool.cpp
  errorhandler.cpp
  field.cpp
  geo.cpp
  key.cpp
  ldapdn.cpp
  ldif.cpp
  phonenumber.cpp
  picture.cpp
  plugin.cpp
  secrecy.cpp
  sound.cpp
  timezone.cpp
  ldifconverter.cpp
  addresseelist.cpp
  vcardconverter.cpp
  vcardtool.cpp
  addresseehelper.cpp
  lock.cpp
  locknull.cpp
  sortmode.cpp
  vcarddrag.cpp
  ${vcards_STAT_SRCS}
  addresseedialog.cpp
  emailselectdialog.cpp
)

if(NOT KDEPIM_NO_KRESOURCES)
  set(kabc_LIB_SRCS
    ${kabc_LIB_SRCS}
    addressbook.cpp
    addresslineedit.cpp
    distributionlist.cpp
    distributionlistdialog.cpp
    formatfactory.cpp
    resource.cpp
    resourceabc.cpp
    resourcecached.cpp
    stdaddressbook.cpp
    vcardformat.cpp
  )
endif()

add_library(kabc ${LIBRARY_TYPE} ${kabc_LIB_SRCS})
generate_export_header(kabc)

target_link_libraries(kabc ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS})
target_link_libraries(kabc LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})
if(NOT KDEPIM_NO_KRESOURCES)
  target_link_libraries(kabc kresources)
  target_link_libraries(kabc LINK_INTERFACE_LIBRARIES kresources)
endif()

set_target_properties(
  kabc PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kabc EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES countrytransl.map DESTINATION ${DATA_INSTALL_DIR}/kabc)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kabc_export.h
  address.h
  addressee.h
  addresseedialog.h
  addresseelist.h
  contactgroup.h
  contactgrouptool.h
  distributionlist.h
  emailselectdialog.h
  errorhandler.h
  field.h
  geo.h
  key.h
  ldifconverter.h
  lock.h
  locknull.h
  phonenumber.h
  picture.h
  plugin.h
  secrecy.h
  sortmode.h
  sound.h
  timezone.h
  vcardconverter.h
  vcarddrag.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kabc COMPONENT Devel
)

if(NOT KDEPIM_NO_KRESOURCES)
  install(FILES kabc_manager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kresources)
  install(FILES
    addressbook.h
    addresslineedit.h
    distributionlistdialog.h
    format.h
    formatfactory.h
    resource.h
    resourceabc.h
    resourcecached.h
    stdaddressbook.h
    vcardformat.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kabc COMPONENT Devel
  )
endif()
