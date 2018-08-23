add_executable( gsenode
    main.cpp )


target_link_libraries (gsenode chain utils bundle crypto database config net producer)
