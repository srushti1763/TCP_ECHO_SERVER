TCP-based frequency generator using the MYIR Z-Turn board (Zynq-7020). The objective is to remotely control frequency signal generation via network communication. The system architecture is built on the Zynq-7000 SoC, where the Processing System (PS) and Programmable Logic (PL) work together seamlessly.
In this design:
•	A PC acts as a TCP client, transmitting frequency control commands over Ethernet.
•	The PS (ARM processor) on the Zynq board runs a TCP server application that receives these commands.
•	Upon receiving the control data, the PS configures and triggers the PL section using AXI GPIO interfaces.
•	The PL side (custom Verilog logic) then generates square wave frequency signals based on the selected parameters.
•	These signals are routed to external pins and can be monitored on an oscilloscope for real-time validation.
