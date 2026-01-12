MODULE = pulse_ctrl


SV_FILES = -f files.f

# all cpp files
TB_CPPS = $(wildcard cppFiles/*.cpp)
# 入口文件
MAIN_CPP = sim_main.cpp


all:
	# Verilator 会自动区分：.sv 用 SV 编译器，.cpp 用 C++ 编译器
	verilator -Wall --trace --cc $(SV_FILES) --exe  $(MAIN_CPP) $(TB_CPPS)
	make -C obj_dir -f V$(MODULE).mk V$(MODULE)
	./obj_dir/V$(MODULE)
	
wave:
	gtkwave wave.vcd &

clean:
	rm -rf obj_dir
	rm -f *.vcd *.log
