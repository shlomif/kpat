project( kpat )

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames 4.9.0 REQUIRED)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${KDEGAMES_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QDBUS_INCLUDE_DIRS})
endif(NOT COMPILING_KDEGAMES)

include(FindPkgConfig)

pkg_check_modules(FC_SOLVE REQUIRED libfreecell-solver)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libkcardgame/include ${FC_SOLVE_INCLUDE_DIRS})

add_subdirectory( icons )
add_subdirectory( libkcardgame )
add_subdirectory( mimetypes )
add_subdirectory( previews )
add_subdirectory( sounds )
add_subdirectory( themes )
add_subdirectory( doc )

set(RELEASE "1")

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

kde4_add_ui_files( kpat_SRCS statisticsdialog.ui )
kde4_add_kcfg_files( kpat_SRCS settings.kcfgc )
kde4_add_app_icon( kpat_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-app-kpat.png" )
kde4_add_executable( kpat ${kpat_SRCS} )
target_link_libraries( kpat kcardgame kdegames ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} ${FC_SOLVE_LIBRARIES} )
install( TARGETS kpat ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS kpat.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kpatui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpat )
install( FILES kpat.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
