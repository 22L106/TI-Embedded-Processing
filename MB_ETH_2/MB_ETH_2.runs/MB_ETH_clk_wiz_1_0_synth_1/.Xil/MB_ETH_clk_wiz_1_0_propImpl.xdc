set_property SRC_FILE_INFO {cfile:/home/vlsiuser/MB_ETH/NEX_ETH_IP/NEX_ETH_IP.srcs/sources_1/bd/MB_ETH/ip/MB_ETH_clk_wiz_1_0/MB_ETH_clk_wiz_1_0.xdc rfile:../../../NEX_ETH_IP.srcs/sources_1/bd/MB_ETH/ip/MB_ETH_clk_wiz_1_0/MB_ETH_clk_wiz_1_0.xdc id:1 order:EARLY scoped_inst:inst} [current_design]
current_instance inst
set_property src_info {type:SCOPED_XDC file:1 line:57 export:INPUT save:INPUT read:READ} [current_design]
set_input_jitter [get_clocks -of_objects [get_ports clk_in1]] 0.1
