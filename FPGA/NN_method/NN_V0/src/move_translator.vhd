----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: LYU Yetao Aaron
-- 
-- Create Date: 20/Mar/2016
-- Design Name: BetaTrax_V0
-- Module Name: move_translator
-- Project Name: BetaTrax
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The move_translator will translate the tile type and tile position into corresponding comamand. 
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity move_translator is
port(
	reset: 	in std_logic; -- active low
	clk:		in std_logic; -- clock
	enable:	in std_logic; -- enable signal, active high, one clock cycle
	x:			in std_logic_vector(4 downto 0); -- x coordinate
	y:			in std_logic_vector(4 downto 0); -- y coordinate
	tile: 	in std_logic_vector(2 downto 0); -- pattern type of the tile
	Txd:		out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic; --transmitter busy, active high
	move_finish: out std_logic -- mark when the move is sent to the opponent
);
end move_translator;

architecture Behavioral of move_translator is

type states is (idle, axis_x, axis_y, tile_pattern, line_feed);
signal Tx_state:		states := idle;
signal x_temp:  		std_logic_vector(4 downto 0) := (others=>'0'); -- temporary storage of x coordinate
signal y_temp:			std_logic_vector(4 downto 0) := (others=>'0'); -- temporary storage of y coordinate
signal pattern:		std_logic_vector(7 downto 0) := (others=>'0'); -- ASII Code of the tile pattern
signal Tx_data:		std_logic_vector(7 downto 0) := (others=>'0'); -- to-be-transmitted data
signal Tx_data_temp1:std_logic_vector(7 downto 0) := (others=>'0'); -- to-be-transmitted data
signal Tx_data_temp2:std_logic_vector(7 downto 0) := (others=>'0'); -- to-be-transmitted data
signal Tx_en:			std_logic  := '0'; -- transmitter enable, active high, one clock
signal Tx_busy_temp:	std_logic; -- transmitter busy, active high

--Component: The TX module of a simple UART for the following protocol: 19200 baud rate (100 MHz clock), no parity-check bit, 8 bits in one byte and one stopping bit.  
component TX is
port (
	reset: 	in std_logic; -- active low
	clk:		in std_logic; -- clock
	Tx_data:	in std_logic_vector(7 downto 0); -- to-be-transmitted data
	Tx_en:	in std_logic; -- transmitter enable, active high, one clock
	Txd:		out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic --transmitter busy, active high
);
end component;

begin

	uut0: TX port map(
		reset => reset,
		clk => clk,
		Tx_data => Tx_data,
		Tx_en => Tx_en,
		Txd => Txd,
		Tx_busy => Tx_busy_temp
	);
	
	process(clk, reset)
	begin
		if (reset = '0') then
			x_temp <= x;
			y_temp <= y;
			Tx_en <= '0';
			Tx_state <= idle;
		elsif (falling_edge(clk)) then
			if (Tx_busy_temp = '0') then
				case Tx_state is
					when idle =>
						move_finish <= '0';
						if (enable = '1') then
							x_temp <= x;
							y_temp <= y;
							Tx_state <= axis_x;
						end if;
					when axis_x =>
						if(x_temp > "11010") then
							x_temp <= x_temp - "11010";
							Tx_data <= x"41";
						else
							Tx_data <= x_temp + x"40";
							Tx_state <= axis_y;
						end if;
						Tx_en <= '1';
					when axis_y =>
						if(y_temp > "11101") then	--if y > 29
							y_temp <= y_temp - "11110";
							Tx_data <= x"33";
						elsif((y_temp <= "11101") and (y_temp > "10011")) then	--if 19 < y <= 29
							y_temp <= y_temp - "10100";
							Tx_data <= x"32";
						elsif((y_temp <= "10011") and (y_temp > "01001")) then	--if 9 < y <= 19
							y_temp <= y_temp - "01010";
							Tx_data <= x"31";
						else	--if y <= 9
							Tx_data <= y_temp + x"30";
							Tx_state <= tile_pattern;
						end if;
						Tx_en <= '1';
					when tile_pattern =>
						Tx_data <= pattern;
						Tx_en <= '1';
						move_finish <= '1';
						Tx_state <= line_feed;
					when line_feed =>
						Tx_data <= x"0a";
						Tx_en <= '1';
						Tx_state <= idle;
					when others => 
						Tx_state <= idle;
				end case;
			else
				Tx_en <= '0';
			end if;
		end if;
	end process;	
	
pattern <= x"2f" when (tile = "001") else	--when pattern is "/"
			x"2f" when (tile = "110") else	--when pattern is "/"
			x"5c" when (tile = "011") else	--when pattern is "\"
			x"5c" when (tile = "100") else	--when pattern is "\"
			x"2b" ;	--when pattern is "+"
			
Tx_busy <= Tx_busy_temp;

end Behavioral;