----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: LYU Yetao Aaron
-- 
-- Create Date: 27/Apr/2016
-- Design Name: BetaTrax_V0
-- Module Name: Neural Network
-- Project Name: BetaTrax
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The policy NN will make decision based on the current board state
--
-- Dependencies: N/A
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;
use IEEE.NUMERIC_STD.ALL;
use work.my_package.all;

entity NN_instance is
port (
	reset			: in std_logic; -- active low
	clk			: in std_logic; -- clock
	enable		: in std_logic; -- enable signal, active high, one clock cycle
	NN_sample 	: in board_type;		
	NN_result 	: out std_logic_vector (11 downto 0);
	NN_ready 	: out std_logic
);
end NN_instance;

architecture Behavioral of NN_instance is
begin
end architecture;