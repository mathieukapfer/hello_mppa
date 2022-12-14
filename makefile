
help:
	@echo " Example of:"
	@echo
	@echo " Openmp example:"
	@echo "  - openmp-x86:    openmp x86"
	@echo "  - openmp-kvx:    openmp on mppa simulator (4 PE)"
	@echo "  - openmp-kvx-16: openmp on mppa simulator (16 PE)"
	@echo "  - openmp-kalray: same as above with Kalray makefile"
	@echo
	@echo " Opencl example:"
	@echo "  - opencl-1:      opencl with kernel code as nested string in code"
	@echo "  - opencl-2:      opencl with kernel in dedicated file (offline and online compilation)"
	@echo "  - opencl-3:      opencl with kernel using C/C++ native library"
	@echo "  - opencl-4:      opencl and out of order mode"
	@echo "  - opencl-5:      opencl and separate queue for code and data"
	@echo "  - opencl-6:      opencl whitebox"
	@echo "  - opencl-7:      opencl whitebox with profiling"
	@echo "  - opencl-7-stv:  opencl whitebox with profiling & trace viewer"


openmp-x86:
	make -f makefile.openmp hello_mp
	./hello_mp

openmp-kvx:
	-rm hello_mppa_mp
	make -f makefile.openmp-kvx hello_mppa_mp
	kvx-cluster -- hello_mppa_mp

openmp-kvx-16:
	-rm hello_mppa_mp
	make -f makefile.simple hello_mppa_mp
	kvx-cluster -- hello_mppa_mp 16

openmp-kalray:
	-rm hello_mppa_mp
	make -f makefile.my_kalray hello_mppa_mp
	kvx-cluster -- output/bin/hello_mppa_mp 16

opencl-1:
	make -f makefile.opencl opencl-sample1
	./opencl-sample1

opencl-2:
	make -f makefile.my_kalray_open_cl && ./output/bin/opencl_sample2

opencl-3:
	TEST_NAME=sample3 KERNEL_NAME=sample3 make -f makefile.my_kalray_open_cl_dispatch && ./output/bin/opencl_sample3

opencl-4:
	TEST_NAME=sample4 KERNEL_NAME=sample4 make -f makefile.my_kalray_open_cl_dispatch && ./output/bin/opencl_sample4

opencl-5:
	TEST_NAME=sample5 KERNEL_NAME=sample4 make -f makefile.my_kalray_open_cl_dispatch && ./output/bin/opencl_sample5

opencl-6:
	TEST_NAME=sample6 KERNEL_NAME=sample4 make -f makefile.my_kalray_whitebox clean all && ./output/bin/opencl_sample6

opencl-7:
	@echo "\nCompiling..."
	TEST_NAME=sample7 KERNEL_NAME=sample7  make -f makefile.my_kalray_whitebox clean all
	@echo "\nRuning..."
	./output/bin/opencl_sample7

opencl-7-stv:
	@echo "\nCompiling..."
	TEST_NAME=sample7 KERNEL_NAME=sample7  make -f makefile.my_kalray_whitebox clean all
	@echo "\nRuning..."
	kvx-trace-util -a --keep -- POCL_MPPA_TRACE_ENABLE=1 POCL_TRACING=lttng ./output/bin/opencl_sample7
	@echo "\nShow trace ..."
	kvx-stv     -t /lib/firmware/kalray/opencl/ocl_fw_l2_d_1m_trace.elf:Cluster0.DSU.bin
