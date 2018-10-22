cmake_minimum_required (VERSION 2.6)
project (updiprog)
set(SOURCES
	app.c
	com.c
	devices.c
	ihex.c
	link.c
	log.c
	main.c
	nvm.c
	phy.c
	progress.c
	sleep.c
)
add_executable (updiprog ${SOURCES})
