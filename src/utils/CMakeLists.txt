set(X_UTILS_SRC)

set(FILE_HPP
	app.h
	event.h
	events.h
	robotid.h
)

set(FILE_CPP
	app.cpp
)

set(TMP_SRC ${FILE_HPP} ${FILE_CPP})
foreach(item ${TMP_SRC})
    list(APPEND X_UTILS_SRC ${X_SRC_PATH}/utils/${item})
endforeach(item)

auto_source_group(${X_SRC_PATH}/utils)