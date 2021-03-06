project(bob_trainer_py${PYVER})

add_subdirectory(overload)

# This defines the dependencies of this package
set(bob_deps "bob_trainer;bob_python")
set(shared "${bob_deps}")
set(incdir ${py_incdir})

set(src
   "linear.cc"
   "kmeans.cc"
   "gmm.cc"
   "rprop.cc"
   "backprop.cc"
   "jfa.cc"
   "wiener.cc"
   "empca.cc"
   "plda.cc"
   "bic.cc"
   "llr.cc"
   "main.cc"
   )

if(LIBSVM_FOUND)
  list(APPEND incdir ${LIBSVM_INCLUDE_DIR})
  list(APPEND src "svm.cc")
endif()

set(ccsrc_overload
   "src/kmeans_wrappers.cc"
   "src/gmm_wrappers.cc"
   "src/main_overload.cc"
   )

# Define the library, compilation and linkage options
bob_sort_headers(incdir)
foreach(inc ${incdir})
  include_directories(SYSTEM ${inc})
endforeach()
bob_add_library(${PROJECT_NAME} "${src}")
target_link_libraries(${PROJECT_NAME} ${shared})

# Pkg-Config generator
bob_pkgconfig(${PROJECT_NAME} "${bob_deps}")
