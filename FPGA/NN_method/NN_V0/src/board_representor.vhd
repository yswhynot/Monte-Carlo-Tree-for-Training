----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: LYU Yetao Aaron
-- 
-- Create Date: 23/Mar/2016
-- Design Name: BetaTrax_V0
-- Module Name: board_representor
-- Project Name: BetaTrax
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The board representor will process the received tile and output tile in the board representation.
--	It will update the board, check tile validation, check win or lose and check the forced play
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

entity board_representor is
port (
	reset					: in std_logic; -- active low
	clk					: in std_logic; -- clock
	enable				: in std_logic; -- enable signal, active high, one clock cycle
	x_new					: in std_logic_vector(4 downto 0); -- x coordinate of the latest tile
	y_new					: in std_logic_vector(4 downto 0); -- y coordinate of the latest tile
	tile_new				: in std_logic_vector(2 downto 0); -- pattern type of the tile
	illegal				: out std_logic;	-- indicator for illegal move
	board					: out board_type;	-- board representation
	board_ready			: out std_logic	-- mark when the board represtation update complete
);
end board_representor;

architecture Behavioral of board_representor is

type states is (idle, shift_board_x, shift_board_y, check_illegal, force_play, finish);
signal rep_state	: states := idle;

signal board_temp: board_type;	-- temporary board representation

signal x 	: std_logic_vector(4 downto 0)	:= (others => '0'); -- x coordinate
signal y		: std_logic_vector(4 downto 0)	:= (others => '0'); -- y coordinate
signal tile	: std_logic_vector(2 downto 0)	:= (others => '0'); -- pattern type of the tile

begin

process(clk, reset)
variable board_x_axis	: integer range 0 to 19 := 0;
variable board_y_axis	: integer range 0 to 19 := 0;
variable board_index_x	: integer range 0 to 19 := 0;
variable board_index_y	: integer range 0 to 19 := 0;
variable A : std_logic := '0';	-- upper edge existance
variable B : std_logic := '0';	-- left edge existance
variable C : std_logic := '0';	-- lower edge existance
variable D : std_logic := '0';	-- right edge existance
variable E : std_logic := '0';	-- upper edge color
variable F : std_logic := '0';	-- left edge color
variable G : std_logic := '0';	-- lower edge color
variable H : std_logic := '0';	-- right edge color
begin
	if (reset = '0') then
		board <= (others => (others => (others => '0')));
		board_temp <= (others => (others => (others => '0')));
		rep_state <= idle;
		illegal <= '0';
	elsif (falling_edge(clk)) then
		case rep_state is
			when idle =>
				board_ready <= '0';
				if (enable = '1') then
					board_x_axis := to_integer(unsigned(x_new));
					board_y_axis := to_integer(unsigned(y_new));
					if ((x_new = "000") and (y_new = "000")) then
						board_temp(1)(1) <= tile_new;
						board_ready <= '1';
						rep_state <= idle;
					else
						board_temp(board_x_axis)(board_y_axis) <= tile_new;
						if (x_new = "000") then
							rep_state <= shift_board_x;	
						else
							if (y_new = "000") then
								rep_state <= shift_board_y;	
							else
								rep_state <= check_illegal;	
							end if;
						end if;	
					end if;
				end if;
				
			when shift_board_x =>	-- shift board one tile right if the new tile is put at Column @
				if (x_new = "000") then
					for board_index in 19 downto 1 loop
						board_temp(board_index)(0 to 19) <= board_temp(board_index - 1)(0 to 19);
					end loop;
					board_temp(0)(0 to 19) <= (others => (others => '0'));
				end if;
				rep_state <= check_illegal;
				
			when shift_board_y =>	-- shift board one tile down if the new tile is put at Row 0
				if (y_new = "000") then
					for board_index in 19 downto 1 loop
						board_temp(0 to 19)(board_index) <= board_temp(0 to 19)(board_index - 1);
					end loop;
					board_temp(0 to 19)(0) <= (others => (others => '0'));
				end if;
				rep_state <= check_illegal;	
			
			when check_illegal =>
				if (board_temp(board_x_axis)(board_y_axis - 1) = "000") then
					A := '0';
				else
					A := '1';
				end if;
				if (board_temp(board_x_axis - 1)(board_y_axis) = "000") then
					B := '0';
				else
					B := '1';
				end if;
				if (board_temp(board_x_axis)(board_y_axis + 1) = "000") then
					C := '0';
				else
					C := '1';
				end if;
				if (board_temp(board_x_axis + 1)(board_y_axis) = "000") then
					D := '0';
				else
					D := '1';
				end if;
				E := board_temp(board_x_axis)(board_y_axis - 1)(0);
				G := board_temp(board_x_axis)(board_y_axis + 1)(2);
				H := board_temp(board_x_axis + 1)(board_y_axis)(1);
				F := ((not G) and E and H) or (G and (not E) and H) or (G and E and (not H));
				if ((F and G and H) or (E and G and H) or (E and F and H) or (E and F and G) or 
				(B and C and D and (not F) and (not G) and (not H)) or (A and C and D and (not E) and (not G) and (not H)) or 
				(A and B and D and (not E) and (not F) and (not H)) or (A and B and C and (not E) and (not F) and (not G))) = '1' then
					illegal <= '1';
					board_ready <= '1';
					rep_state <= idle;
				else
					illegal <= '0';
					rep_state <= force_play;
				end if;
				
			when force_play =>
				for board_index_x in 1 to 18 loop
					for board_index_y in 1 to 18 loop
						if (board_temp(board_index_x)(board_index_y - 1) = "000") then
							A := '0';
						else
							A := '1';
						end if;
						if (board_temp(board_index_x - 1)(board_index_y) = "000") then
							B := '0';
						else
							B := '1';
						end if;
						if (board_temp(board_index_x)(board_index_y + 1) = "000") then
							C := '0';
						else
							C := '1';
						end if;
						if (board_temp(board_index_x + 1)(board_index_y) = "000") then
							D := '0';
						else
							D := '1';
						end if;
						E := board_temp(board_index_x)(board_index_y - 1)(0);
						G := board_temp(board_index_x)(board_index_y + 1)(2);
						H := board_temp(board_index_x + 1)(board_index_y)(1);
						F := ((not G) and E and H) or (G and (not E) and H) or (G and E and (not H));
						if ((E and H) or (E and G) or (E and F) or (C and D and (not G) and (not H)) or (B and D and (not F) and (not H)) or (B and C and (not F) and (not G))) = '1' then
							board_temp(board_index_x)(board_index_y)(2) <= '1';
						else
							board_temp(board_index_x)(board_index_y)(2) <= '0';
						end if;
						if ((F and H) or (F and G) or (E and F) or (C and D and (not G) and (not H)) or (A and D and (not E) and (not H)) or (A and C and (not E) and (not G))) = '1' then
							board_temp(board_index_x)(board_index_y)(1) <= '1';
						else
							board_temp(board_index_x)(board_index_y)(1) <= '0';
						end if;
						if ((F and G) or (E and G) or ((not A) and G and H) or (B and D and (not F) and (not H)) or (A and D and G and (not H)) or (A and B and (not E) and (not F)) or (A and (not C) and D and F and (not H))) = '1' then
							board_temp(board_index_x)(board_index_y)(0) <= '1';
						else
							board_temp(board_index_x)(board_index_y)(0) <= '0';
						end if;
					end loop;
				end loop;
				rep_state <= finish;
				
			when finish =>
				board <= board_temp;
				board_ready <= '1';
				rep_state <= idle;
			when others =>
				NULL;
		end case;		
	end if;
end process;
end Behavioral;