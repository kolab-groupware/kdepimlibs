include_directories(
  ${CMAKE_BINARY_DIR}/kpimidentities
  ${CMAKE_BINARY_DIR}/kpimtextedit
)


set (identitytest_SRCS identitytest.cpp )
kde4_add_unit_test (identitytest TESTNAME kpimidentities-identitytest ${identitytest_SRCS})

target_link_libraries ( identitytest
                        ${QT_QTTEST_LIBRARY}
                        ${QT_QTCORE_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        kpimidentities
                      )

set (signaturetest_SRCS signaturetest.cpp )
kde4_add_unit_test (signaturetest TESTNAME kpimidentities-signaturetest ${signaturetest_SRCS})

target_link_libraries ( signaturetest
                        ${QT_QTTEST_LIBRARY}
                        ${KDE4_KDEUI_LIBS}
                        kpimidentities
                        kpimtextedit
                      )
