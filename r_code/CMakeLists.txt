set(r_code_SRC
    atom.cpp
    image.cpp
    #image.tpl.cpp
    image_impl.cpp
    object.cpp
    r_code.cpp
    utils.cpp
    vector.cpp
    )

add_library(r_code SHARED ${r_code_SRC})
