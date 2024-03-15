# vim: syntax=cmake
# ----------------------------------------------------------------------------
# add libvpu implement
# ----------------------------------------------------------------------------




add_library(utils SHARED
    mpp_enc_roi_utils.c
    mpi_enc_utils.c
    mpi_dec_utils.c
    mpp_opt.c
    utils.c
    iniparser.c
    dictionary.c
    camera_source.c
    )

target_link_libraries(utils mpp_base)
