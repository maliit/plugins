include(FeatureSummary)
set_package_properties(PRESAGE PROPERTIES
        URL "http://presage.sourceforge.net/"
        DESCRIPTION "Presage is an intelligent predictive text entry platform.")

find_library(PRESAGE_LIBRARIES presage)
find_path(PRESAGE_INCLUDE_DIRS presage.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PRESAGE DEFAULT_MSG PRESAGE_LIBRARIES PRESAGE_INCLUDE_DIRS)
mark_as_advanced(PRESAGE_INCLUDE_DIRS PRESAGE_LIBRARIES)
