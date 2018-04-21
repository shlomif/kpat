project(kpat)
include(FindPkgConfig)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.30.0")

pkg_check_modules(FC_SOLVE REQUIRED libfreecell-solver)
find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Completion
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    GuiAddons
    I18n
    KIO
    NewStuff
    WidgetsAddons
    XmlGui
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libkcardgame/include)

add_subdirectory(icons)
add_subdirectory(libkcardgame)
add_subdirectory(mimetypes)
add_subdirectory(previews)
add_subdirectory(sounds)
add_subdirectory(themes)
add_subdirectory(doc)

set(kpat_SRCS ${libfcs_SRCS}
   clock.cpp
   dealer.cpp
   dealerinfo.cpp
   fortyeight.cpp
   freecell.cpp
   freecell.cpp
   gameselectionscene.cpp
   golf.cpp
   grandf.cpp
   gypsy.cpp
   idiot.cpp
   klondike.cpp
   main.cpp
   mainwindow.cpp
   messagebox.cpp
   mod3.cpp
   numbereddealdialog.cpp
   patpile.cpp
   patsolve/abstract_fc_solve_solver.cpp
   patsolve/clocksolver.cpp
   patsolve/fortyeightsolver.cpp
   patsolve/freecellsolver.cpp
   patsolve/golfsolver.cpp
   patsolve/grandfsolver.cpp
   patsolve/gypsysolver.cpp
   patsolve/idiotsolver.cpp
   patsolve/klondikesolver.cpp
   patsolve/memory.cpp
   patsolve/mod3solver.cpp
   patsolve/patsolve.cpp
   patsolve/simonsolver.cpp
   patsolve/spidersolver.cpp
   patsolve/yukonsolver.cpp
   pileutils.cpp
   renderer.cpp
   simon.cpp
   spider.cpp
   soundengine.cpp
   statisticsdialog.cpp
   view.cpp
   yukon.cpp
)

ki18n_wrap_ui( kpat_SRCS statisticsdialog.ui )
kconfig_add_kcfg_files( kpat_SRCS settings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kpat.png")
ecm_add_app_icon(kpat_SRCS ICONS ${ICONS_SRCS})
add_executable(kpat ${kpat_SRCS})
target_link_libraries(kpat
    KF5::Crash
    KF5::DBusAddons
    KF5::I18n
    KF5::KIOCore
    KF5KDEGames
    kcardgame
    ${FC_SOLVE_LIBRARIES}
)

install(TARGETS kpat ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(PROGRAMS org.kde.kpat.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES kpatui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kpat)
install(FILES kpat.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
