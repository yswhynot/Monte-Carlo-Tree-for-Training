----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: LYU Yetao Aaron
-- 
-- Create Date: 23/Mar/2016
-- Design Name: BetaTrax_V0
-- Module Name: board_updater
-- Project Name: BetaTrax
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The board updater will update the board according to the tile added
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

entity board_updater is
port (
	reset					: in std_logic; -- active low
	clk					: in std_logic; -- clock
	x_new					: in std_logic_vector(4 downto 0); -- x coordinate of the latest tile
	y_new					: in std_logic_vector(4 downto 0); -- y coordinate of the latest tile
	tile_new				: in std_logic_vector(2 downto 0); -- four edges of the latest tile, recorded counter-clockwisely starting from the top edge
	update_ready		: in std_logic; --new board update ready signal
	board					: out board_type;	-- board representation 1
	board_ready			: out std_logic	-- mark when the board represtation update complete
);
end board_updater;

architecture Behavioral of board_updater is

type states is (idle, update_board_1, update_board_2, shift_board, finish);
signal rep_state		: states := idle;

begin

process(reset, clk)

variable board_temp		: board_type:= (others => (others => (others => '0')));		-- board representation 1
variable board_index_1	: integer range 0 to 31 := 0;
variable board_index_2	: integer range 0 to 31 := 0;
variable board_x_axis	: integer range 0 to 31 := 0;
variable board_y_axis	: integer range 0 to 31 := 0;
variable connected_edges: std_logic_vector(31 downto 0):= (others => '0');
variable connected_edges_status: std_logic_vector(3 downto 0):= (others => '0');

begin
	if (reset = '0') then
		board_temp := (others => (others => '0'));
		connected_edges:= (others => '0');
		board_ready <= '0';
	elsif (falling_edge(clk)) then
		case rep_state is
			when idle =>
				board_ready <= '0';
				if (tile_ready = '1') then
					if(x_new = "00000") and (y_new = "00000") then
						-- update board representation 1
						board1_temp(1)(1) := '1';
						case tile_type_receive(1) is
							when '0' =>
								board2_temp(0) := '0' & "01" & "00001" & "00001";
								board2_temp(1) := '0' & "10" & "00001" & "00001";
								board2_temp(2) := '1' & "11" & "00001" & "00001";
								board2_temp(3) := '1' & "00" & "00001" & "00001";
							when others =>
								board2_temp(0) := '0' & "01" & "00001" & "00001";
								board2_temp(1) := '1' & "10" & "00001" & "00001";
								board2_temp(2) := '0' & "11" & "00001" & "00001";
								board2_temp(3) := '1' & "00" & "00001" & "00001";
						end case;	
						rep_state <= idle;
					else
						-- update board representation 1
						board1_temp(to_integer(unsigned(x_new)))(to_integer(unsigned(y_new))) := '1';
						
						-- check surrounding spaces
						check_surrounding: for board2_index_3 in 0 to board2_length - 1 loop
							if (board2_temp(board2_index_3)(7 downto 3) = x_new) and (board2_temp(board2_index_3)(7 downto 3) =  (y_new - '1')) and (board2_temp(board2_index_3)(2 downto 1) = "01") then
								connected_edges_status(0) := '1';
								connected_edges(7 downto 0) := std_logic_vector(to_unsigned(board2_index_3 , 8));

							elsif (board2_temp(board2_index_3)(7 downto 3) = x_new) and (board2_temp(board2_index_3)(7 downto 3) =  (y_new - '1')) and (board2_temp(board2_index_3)(2 downto 1) = "10") then
								connected_edges_status(1) := '1';
								connected_edges(15 downto 8) := std_logic_vector(to_unsigned(board2_index_3 , 8));

							elsif (board2_temp(board2_index_3)(7 downto 3) = x_new) and (board2_temp(board2_index_3)(7 downto 3) =  (y_new - '1')) and (board2_temp(board2_index_3)(2 downto 1) = "11") then
								connected_edges_status(2) := '1';
								connected_edges(23 downto 16) := std_logic_vector(to_unsigned(board2_index_3 , 8));

							elsif	(board2_temp(board2_index_3)(7 downto 3) = x_new) and (board2_temp(board2_index_3)(7 downto 3) =  (y_new - '1')) and (board2_temp(board2_index_3)(2 downto 1) = "00") then
								connected_edges_status(3) := '1';
								connected_edges(31 downto 24) := std_logic_vector(to_unsigned(board2_index_3 , 8));

							else
								NULL;
							end if;
						end loop check_surrounding;
				
						rep_state <= update_board2_1;
					end if;
				end if;

			when update_board2_1 =>
				
				-- update board representation 2
				case connected_edges_status is
					when "0001" =>
						board2_index_1 := to_integer(unsigned(connected_edges(7 downto 0)));
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > board2_index_1 then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							elsif	board2_index = board2_index_1 then
								board2_temp(board2_index + 1)(0) := edges_new(3);
								board2_temp(board2_index + 1)(2 downto 1) := "11";
								board2_temp(board2_index + 1)(7 downto 3) := x_new;
								board2_temp(board2_index + 1)(12 downto 8) := y_new;
							else
								NULL;
							end if;
						end loop;
						
					when "0010" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > to_integer(unsigned(connected_edges(15 downto 8))) then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							end if;
						end loop;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(0) := edges_new(0);
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(2 downto 1) := "00";
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(12 downto 8) := y_new;

					when "0100" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > to_integer(unsigned(connected_edges(23 downto 16))) then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							end if;
						end loop;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(0) := edges_new(1);
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(2 downto 1) := "01";
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(12 downto 8) := y_new;

					when "1000" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > to_integer(unsigned(connected_edges(31 downto 24))) then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							end if;
						end loop;						
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(0) := edges_new(2);
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(2 downto 1) := "10";
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(12 downto 8) := y_new;

					when "0011" =>
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(0) := edges_new(3);
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(2 downto 1) := "11";
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(0) := edges_new(2);
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(2 downto 1) := "10";
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(12 downto 8) := y_new;	

					when "0110" =>
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(0) := edges_new(0);
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(2 downto 1) := "00";
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(0) := edges_new(3);
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(2 downto 1) := "11";
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(12 downto 8) := y_new;	

					when "1100" =>
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(0) := edges_new(1);
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(2 downto 1) := "01";
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(0) := edges_new(0);
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(2 downto 1) := "00";
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(12 downto 8) := y_new;	

					when "1001" =>
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(0) := edges_new(2);
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(2 downto 1) := "10";
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(0) := edges_new(1);
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(2 downto 1) := "01";
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(12 downto 8) := y_new;	

					when "1110" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(31 downto 24))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');

					when "1101"	=>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(7 downto 0))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
						
					when "1011"	=>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(15 downto 8))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
						
					when "0111"	=>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(23 downto 16))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
					
					when "0101" =>
					
					when "1010" =>		
					
					when "1111" =>
					
					when others =>
						NULL;					
				end case;
				rep_state <= update_board2_2;
			
			when update_board2_2 =>
			
				-- update board representation 2
				case connected_edges_status is
					when "0001" =>
						board2_index_1 := to_integer(unsigned(connected_edges(7 downto 0)));
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > board2_index_1 then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							else
								NULL;
							end if;
						end loop;
						
						board2_temp(board2_index_1)(0) := edges_new(1);
						board2_temp(board2_index_1)(2 downto 1) := "01";
						board2_temp(board2_index_1)(7 downto 3) := x_new;
						board2_temp(board2_index_1)(12 downto 8) := y_new;
						board2_temp(board2_index_1 + 1)(0) := edges_new(2);
						board2_temp(board2_index_1 + 1)(2 downto 1) := "10";
						board2_temp(board2_index_1 + 1)(7 downto 3) := x_new;
						board2_temp(board2_index_1 + 1)(12 downto 8) := y_new;	
						

					when "0010" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > to_integer(unsigned(connected_edges(15 downto 8))) then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							end if;
						end loop;
						
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(0) := edges_new(2);
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(2 downto 1) := "10";
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(0) := edges_new(3);
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(2 downto 1) := "11";
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))) + 1)(12 downto 8) := y_new;	
						

					when "0100" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > to_integer(unsigned(connected_edges(23 downto 16))) then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							end if;
						end loop;
						
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(0) := edges_new(3);
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(2 downto 1) := "11";
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(0) := edges_new(0);
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(2 downto 1) := "00";
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))) + 1)(12 downto 8) := y_new;	
						

					when "1000" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index > to_integer(unsigned(connected_edges(31 downto 24))) then
								board2_temp(board2_index + 1) := board2_temp(board2_index);
							end if;
						end loop;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(0) := edges_new(0);
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(2 downto 1) := "00";
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(12 downto 8) := y_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(0) := edges_new(1);
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(2 downto 1) := "01";
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))) + 1)(12 downto 8) := y_new;	
				
					when "1110" =>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(31 downto 24))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(0) := edges_new(0);
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(2 downto 1) := "00";
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(31 downto 24))))(12 downto 8) := y_new;

					when "1101"	=>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(7 downto 0))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(0) := edges_new(1);
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(2 downto 1) := "01";
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(7 downto 0))))(12 downto 8) := y_new;
						
					when "1011"	=>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(15 downto 8))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(0) := edges_new(2);
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(2 downto 1) := "10";
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(15 downto 8))))(12 downto 8) := y_new;
						
					when "0111"	=>
						for board2_index in board2_length - 2 downto 0 loop
							if board2_index >= to_integer(unsigned(connected_edges(23 downto 16))) then
								board2_temp(board2_index) := board2_temp(board2_index + 1);
							end if;
						end loop;
						board2_temp(board2_length - 1) := (others => '0');
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(0) := edges_new(3);
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(2 downto 1) := "11";
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(7 downto 3) := x_new;
						board2_temp(to_integer(unsigned(connected_edges(23 downto 16))))(12 downto 8) := y_new;

					when "0101" =>
					
					when "1010" =>		
					
					when others =>
						NULL;					
				end case;
				rep_state <= shift_board;
			
			when shift_board =>
			
				-- shift board one tile right if the new tile is put at Column @
				if (x_new = "00000") then
					-- shift board representation 1
					Shift_Board1_x: for board1_index_1 in 31 downto 1 loop
						board1_temp(board1_index_1) := board1_temp(board1_index_1 - 1);
					end loop Shift_Board1_x;
					board1_temp(0) := (others => '0');
					-- update board representation 2
					Shift_Board2_x: for board2_index_1 in board2_length - 1 downto 1 loop
						board2_temp(board2_index_1)(7 downto 3) := board2_temp(board2_index_1)(7 downto 3) + '1';
					end loop Shift_Board2_x;
				end if;
				
				-- shift board one tile down if the new tile is put at Row 0
				if (y_new = "00000") then
					-- shift board representation 1
					Shift_Board1_y: for board1_index_2 in 0 to 31 loop
						board1_temp(board1_index_2)(31 downto 1) :=	board1_temp(board1_index_2)(30 downto 0);
						board1_temp(board1_index_2)(0) := '0';
					end loop Shift_Board1_y;
					-- update board representation 2
					Shift_Board2_y: for board2_index_2 in board2_length - 1 downto 1 loop
						board2_temp(board2_index_2)(12 downto 8) := board2_temp(board2_index_2)(12 downto 8) + '1';
					end loop Shift_Board2_y;
				end if;
				
				-- output board representations
				board1 <= board1_temp;
				board2 <= board2_temp;
				board_ready <= '1';
				rep_state <= finish;
			
			when finish =>
				rep_state <= idle;
				
			when others =>
				NULL;
		end case;		
	end if;
end process;

end Behavioral;