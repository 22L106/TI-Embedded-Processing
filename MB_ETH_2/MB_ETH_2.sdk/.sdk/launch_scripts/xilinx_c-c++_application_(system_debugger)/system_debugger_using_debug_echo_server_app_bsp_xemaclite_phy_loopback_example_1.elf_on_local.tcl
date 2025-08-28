connect -url tcp:127.0.0.1:3121
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292744050A"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292744050A"} -index 0
dow /home/vlsiuser/MB_ETH/NEX_ETH_IP/NEX_ETH_IP.sdk/echo_server_app_bsp_xemaclite_phy_loopback_example_1/Debug/echo_server_app_bsp_xemaclite_phy_loopback_example_1.elf
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292744050A"} -index 0
con
