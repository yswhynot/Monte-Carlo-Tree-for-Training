----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: Thomas Junbin ZHANG
-- 
-- Create Date: 26/Feb/2016
-- Design Name: UART
-- Module Name: UART
-- Project Name: FPGA_Workshop
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: A simple UART for the following protocol: 19200 baud rate (100 MHz clock), no parity-check bit, 8 bits in one byte and one stopping bit.  
--
-- Dependencies: 
-- 1. TX.vhd
-- 2. RX.vhd
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity UART is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	Tx_data:	in std_logic_vector(7 downto 0); -- to-be-transmitted data
	Tx_en:	in std_logic; -- transmitter enable, active high, one clock
	Txd:	out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic; --transmitter busy, active high
	Rx_data:	out std_logic_vector(7 downto 0); -- received data
	Rx_err:	out std_logic; -- error in receiving, active high
	Rxd:	in std_logic; -- receiver input signal
	Rx_busy:	out std_logic -- receiver busy, active low
);
end UART;

architecture Behavioral of UART is

component TX is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	Tx_data:	in std_logic_vector(7 downto 0); -- to-be-transmitted data
	Tx_en:	in std_logic; -- transmitter enable, active high, one clock
	Txd:	out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic --transmitter busy, active high
);
end component;
component RX is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	Rx_data:	out std_logic_vector(7 downto 0); -- received data
	Rx_err:	out std_logic; -- error in receiving, active high
	Rxd:	in std_logic; -- receiver input signal
	Rx_busy:	out std_logic -- receiver busy, active low
);
end component;

begin
-- Port map of components first
	uut0: TX port map(
		reset => reset,
		clk => clk,
		Tx_data => Tx_data,
		Tx_en => Tx_en,
		Txd => Txd,
		Tx_busy => Tx_busy
	);
	
	uut1: RX port map(
		reset => reset,
		clk => clk,
		Rx_data => Rx_data,
		Rx_err => Rx_err,
		Rxd => Rxd,
		Rx_busy => Rx_busy
	);
end Behavioral;

