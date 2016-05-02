----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: LYU Yetao Aaron
-- 
-- Create Date: 02/Feb/2016   
-- Design Name: BetaTrax_V0
-- Module Name: Trax 
-- Project Name: BetaTrax
-- Target Devices: EP4CE115F29C8
-- Tool versions: 
-- Description: This module is used as the top module.
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.my_package.all;

entity Trax is
port(
	reset:	in std_LOGIC;
	clock:	in std_LOGIC;
	Txd:		out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic; --transmitter busy, active high
	Rxd:		in std_logic; -- receiver input signal
	Rx_busy:	out std_logic; -- receiver busy, active high
	Rx_err:	out std_logic; -- error in receiving, active high
	Translation_busy:	out std_logic; -- translate busy, active high
	Tranlation_err:	out std_logic; -- error in command translation, active high
	testing: out std_logic -- for testing, LED 7
);
end Trax;

architecture behavior of Trax is

component pll IS
PORT(
	inclk0: IN STD_LOGIC  := '0';
	c0		: OUT STD_LOGIC 
	);
END component;

component command_translator is
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
end component;

component move_translator is
port (
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
end component;

component board_representor is
port (
	reset			: in std_logic; -- active low
	clk			: in std_logic; -- clock
	enable		: in std_logic; -- enable signal, active high, one clock cycle
	x_new			: in std_logic_vector(4 downto 0); -- x coordinate of the latest tile
	y_new			: in std_logic_vector(4 downto 0); -- y coordinate of the latest tile
	tile_new		: in std_logic_vector(2 downto 0); -- pattern type of the tile
	illegal		: out std_logic;	-- indicator for illegal move
	board			: out board_type;	-- board representation 
	board_ready	: out std_logic	-- mark when the board represtation update complete
);
end component;

component NN_instance
port (
	reset			: in std_logic; -- active low
	clk			: in std_logic; -- clock
	enable		: in std_logic; -- enable signal, active high, one clock cycle
	NN_sample 	: in board_type;		
	NN_result 	: out std_logic_vector (11 downto 0);
	NN_ready 	: out std_logic
);
end component;

signal clk_100			: std_logic;

signal board			: board_type;		-- board representation 

signal x_receive 		: std_logic_vector(4 downto 0)	:= (others => '0'); -- x coordinate
signal y_receive 		: std_logic_vector(4 downto 0)	:= (others => '0'); -- y coordinate
signal tile_receive	: std_logic_vector(2 downto 0)	:= (others => '0'); -- pattern type of the tile
signal x_new 			: std_logic_vector(4 downto 0)	:= (others => '0'); -- x coordinate
signal y_new 			: std_logic_vector(4 downto 0)	:= (others => '0'); -- y coordinate
signal tile_new		: std_logic_vector(2 downto 0)	:= (others => '0'); -- pattern type of the tile
signal x_send 			: std_logic_vector(4 downto 0)	:= (others => '0'); -- x coordinate
signal y_send 			: std_logic_vector(4 downto 0)	:= (others => '0'); -- y coordinate
signal tile_send		: std_logic_vector(2 downto 0)	:= (others => '0'); -- pattern type of the tile
	
signal illegal			: std_logic;	-- indicator for illegal move
signal NN_result 		: std_logic_vector (11 downto 0);	-- in all, 2400(20*20*6) kinds of outputs

signal board_representor_en	: std_logic; 	-- board representor enable signal
signal neural_network_en		: std_logic; 	-- neural network enable signal
signal move_translator_en		: std_logic; 	-- move translator enable signal

signal translate_finish	: std_logic; 	--trigger when the command translation is finished
signal board_ready		: std_logic; -- mark when the board represtation is ready
signal NN_ready 			: std_logic; -- mark when the neural network outpu is ready
signal move_finish		: std_logic; -- mark when the move is sent to the opponent

type states is (idle, board_representation, Neural_network, send_move);
signal top_state		: states := idle;

begin

pll_inst:pll port map(clock,clk_100);

board_representor_inst:board_representor port map(
	reset => reset,
	clk => clk_100,	
	enable => board_representor_en,
	x_new => x_new,
	y_new => y_new,
	tile_new => tile_new,
	illegal => illegal,
	board => board,
	board_ready => board_ready
);

command_translator_inst:command_translator port map(
	reset => reset,
	clk => clk_100,
	board => board,
	Rx_err => Rx_err,
	Tranlation_err => Tranlation_err,
	Rxd => Rxd,
	Rx_busy => Rx_busy,
	Translation_busy => Translation_busy,
	x => x_receive,
	y => y_receive,
	tile => tile_receive,
	translate_finish => translate_finish
);

move_translator_inst:move_translator port map(
	reset => reset,
	clk 	=> clk_100,
	enable=> move_translator_en,
	x => x_send,
	y => y_send,
	tile	=> tile_send,
	Txd 	=> Txd,
	Tx_busy =>Tx_busy,
	move_finish => move_finish
);

neural_network_inst: NN_INSTANCE port map (  
	reset => reset,
	clk 	=> clk_100,
	enable=> neural_network_en,
	NN_sample	=> board,
	NN_result	=> NN_result,
	NN_ready		=> NN_ready
);

process(reset, clk_100)
variable board_index	: integer range 0 to 19 := 0;
begin
	if (reset = '0') then
		top_state <= idle;
	elsif (rising_edge(clk_100)) then
		case top_state is
			when idle =>
				if (translate_finish = '1') then
					x_new <= x_receive;
					y_new <= y_receive;
					tile_new <= tile_receive;
					board_representor_en <= '1';
					top_state <= board_representation;
				end if;
			
			when board_representation =>
				board_representor_en <= '0';
				if (board_ready = '1') then
					neural_network_en <= '1';
					top_state <= Neural_network;
				end if;
				
			when Neural_network =>
				neural_network_en <= '0';
				x_send <= "00101";
				y_send <= "01101";
				tile_send <= board(5)(13);
				--x_send <= x_receive;
				--y_send <= y_receive;
				--tile_send <= tile_receive;
				move_translator_en <= '1';
				top_state <= send_move;
				
			when send_move=>
				move_translator_en <= '0';
				if (move_finish = '1') then
					top_state <= idle;
				end if;
			When others=>
				NULL;
		end case;
	end if;
end process;


testing <= '1';

end architecture;