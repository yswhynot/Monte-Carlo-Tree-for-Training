----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: LYU Yetao Aaron
-- 
-- Create Date: 24/Mar/2016
-- Design Name: BetaTrax_V0
-- Module Name: command_translator
-- Project Name: BetaTrax
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The command_translator will translate the received command into corresponding tile type and tile position. 
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

entity command_translator is
port (
	reset					: 	in std_logic; -- active low
	clk					:	in std_logic; -- clock
	Rxd					:	in std_logic; -- receiver input signal
	board					:	in board_type;-- board representation 
	Rx_err				:	out std_logic; -- error in receiving, active high
	Rx_busy				:	out std_logic; -- receiver busy, active high
	Tranlation_err		:	out std_logic; -- error in command translation, active high
	Translation_busy	:	out std_logic; -- translate busy, active high
	x						:	out std_logic_vector(4 downto 0); -- x coordinate
	y						:	out std_logic_vector(4 downto 0); -- y coordinate
	tile					: 	out std_logic_vector(2 downto 0); -- pattern type of the tile
	translate_finish	:	out std_logic 	--trigger when the command translation is finished
);
end command_translator;

architecture Behavioral of command_translator is

type states is (idle, axis_x, axis_y, tile_pattern);
signal Rx_state:		states := idle;
signal Rx_data:		std_logic_vector(7 downto 0); -- received data
signal x_temp:  		std_logic_vector(4 downto 0) := (others=>'0'); -- temporary storage of x coordinate
signal y_temp:			std_logic_vector(4 downto 0) := (others=>'0'); -- temporary storage of y coordinate
signal Rx_finish:	 	std_logic;	--trigger when the character is received

--Component: The RX module of a simple UART for the following protocol: 19200 baud rate (100 MHz clock), no parity-check bit, 8 bits in one byte and one stopping bit.  
component RX is
port (
	reset		: 	in std_logic; 	-- active low
	clk		:	in std_logic; 	-- clock
	Rx_data	:	out std_logic_vector(7 downto 0); -- received data
	Rx_err	:	out std_logic; -- error in receiving, active high
	Rxd		:	in std_logic; 	-- receiver input signal
	Rx_busy	:	out std_logic; -- receiver busy, active low
	Rx_finish:	out std_logic 	--trigger when the character is received
);
end component;

begin

	uut0: RX port map(
		reset => reset,
		clk => clk,
		Rx_data => Rx_data,
		Rx_err => Rx_err,
		Rxd => Rxd,
		Rx_busy => Rx_busy,
		Rx_finish => Rx_finish
	);
	
	process(clk, reset)
	variable board_x_axis	: integer range 0 to 31 := 0;
	variable board_y_axis	: integer range 0 to 31 := 0;
	begin
		if (reset = '0') then
			x_temp <= (others => '0');
			y_temp <= (others => '0');
			Tranlation_err <= '0';
			translate_finish <= '0';
			Rx_state <= idle;
		elsif falling_edge(clk) then
			if Rx_finish = '1' then
				case Rx_state is
					when idle =>
						if ((Rx_data < x"5b") and (Rx_data >= x"40")) then	--if the received data is axis_x
							x_temp <= Rx_data(4 downto 0);
							Rx_state <= axis_x;
						end if;
						
					when axis_x =>
						if ((Rx_data < x"3a") and (Rx_data >= x"30")) then	--if the received data is axis_y
							y_temp <= '0' & Rx_data(3 downto 0);
							Rx_state <= axis_y;
						else
							x_temp <= Rx_data(4 downto 0) + "11010";	--if the received data is axis_x
						end if;
						
					when axis_y =>
						case Rx_data is
							when x"2F" =>	--ASCII:/
								board_x_axis := to_integer(unsigned(x_temp));
								board_y_axis := to_integer(unsigned(y_temp));
								x <= x_temp;
								y <= y_temp;
								if (x_temp = "000") and (y_temp = "000") then
									tile <= "001";
								else 
									case board(board_x_axis)(board_y_axis - 1) is
										when "110"|"100"|"010" =>	--if the up edge is white
											tile <= "001";
										when "001"|"011"|"101" =>	--if the up edge is black
											tile <= "110";
										when others =>
											NULL;
									end case;
									case board(board_x_axis - 1)(board_y_axis) is
										when "110"|"011"|"101" =>	--if the left edge is white
											tile <= "001";
										when "001"|"100"|"010" =>	--if the left edge is black
											tile <= "110";
										when others =>
											NULL;
									end case;
									case board(board_x_axis)(board_y_axis + 1) is
										when "010"|"011"|"001" =>	--if the down edge is white
											tile <= "110";
										when "101"|"100"|"110" =>	--if the down edge is black
											tile <= "001";
										when others =>
											NULL;
									end case;
									case board(board_x_axis + 1)(board_y_axis) is
										when "001"|"100"|"101" =>	--if the right edge is white
											tile <= "110";
										when "110"|"011"|"010" =>	--if the right edge is black
											tile <= "001";
										when others =>
											NULL;
									end case;
								end if;
								
								translate_finish <= '1';
								Rx_state <= tile_pattern;
								
							when x"5C" =>	--ASCII:\
								board_x_axis := to_integer(unsigned(x_temp));
								board_y_axis := to_integer(unsigned(y_temp));
								x <= x_temp;
								y <= y_temp;
								case board(board_x_axis)(board_y_axis - 1) is
									when "110"|"100"|"010" =>	--if the up edge is white
										tile <= "011";
									when "001"|"011"|"101" =>	--if the up edge is black
										tile <= "100";
									when others =>
										NULL;
								end case;
								case board(board_x_axis - 1)(board_y_axis) is
									when "110"|"011"|"101" =>	--if the left edge is white
										tile <= "100";
									when "001"|"100"|"010" =>	--if the left edge is black
										tile <= "011";
									when others =>
										NULL;
								end case;
								case board(board_x_axis)(board_y_axis + 1) is
									when "010"|"011"|"001" =>	--if the down edge is white
										tile <= "100";
									when "101"|"100"|"110" =>	--if the down edge is black
										tile <= "011";
									when others =>
										NULL;
								end case;
								case board(board_x_axis + 1)(board_y_axis) is
									when "001"|"100"|"101" =>	--if the right edge is white
										tile <= "011";
									when "110"|"011"|"010" =>	--if the right edge is black
										tile <= "100";
									when others =>
										NULL;
								end case;
								translate_finish <= '1';
								Rx_state <= tile_pattern;
								
							when x"2B" =>	--ASCII:+
								board_x_axis := to_integer(unsigned(x_temp));
								board_y_axis := to_integer(unsigned(y_temp));
								x <= x_temp;
								y <= y_temp;
								if (x_temp = "000") and (y_temp = "000") then
									tile <= "010";
								else 
									case board(board_x_axis)(board_y_axis - 1) is
										when "110"|"100"|"010" =>	--if the up edge is white
											tile <= "010";
										when "001"|"011"|"101" =>	--if the up edge is black
											tile <= "101";
										when others =>
											NULL;
									end case;
									case board(board_x_axis - 1)(board_y_axis) is
										when "110"|"011"|"101" =>	--if the left edge is white
											tile <= "101";
										when "001"|"100"|"010" =>	--if the left edge is black
											tile <= "010";
										when others =>
											NULL;
									end case;
									case board(board_x_axis)(board_y_axis + 1) is
										when "010"|"011"|"001" =>	--if the down edge is white
											tile <= "010";
										when "101"|"100"|"110" =>	--if the down edge is black
											tile <= "101";
										when others =>
											NULL;
									end case;
									case board(board_x_axis + 1)(board_y_axis) is
										when "001"|"100"|"101" =>	--if the right edge is white
											tile <= "101";
										when "110"|"011"|"010" =>	--if the right edge is black
											tile <= "010";
										when others =>
											NULL;
									end case;
								end if;
								translate_finish <= '1';
								Rx_state <= tile_pattern;
								
							when others =>	--if the received data is axis_y
								case y_temp(1 downto 0) is
									when "01" =>
										y_temp <= ('0' & Rx_data(3 downto 0)) + "01010";
									when "10" =>
										y_temp <= ('0' & Rx_data(3 downto 0)) + "10100";
									when "11" =>
										y_temp <= ('0' & Rx_data(3 downto 0)) + "11110";
									when others =>
										Tranlation_err <= '1';
								end case;
						end case;
						
					when tile_pattern =>
						translate_finish <= '0';
						if (Rx_data = x"0a") then
							Rx_state <= idle;
						end if;
					when others =>
						Rx_state <= idle;
				end case;
			else
				NULL;
			end if;
		end if;
	end process;
	
	Translation_busy <= '0' when (Rx_state = idle) else '1';
	
end Behavioral;