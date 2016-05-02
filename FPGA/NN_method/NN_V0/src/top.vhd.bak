library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity top is
port(
	reset,clock:	in std_LOGIC;
	Tx_data:	in std_logic_vector(7 downto 0); -- to-be-transmitted data
	send:	in std_logic; -- transmitter enable, active high, one clock
	Txd:	out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic; --transmitter busy, active high
	Rx_data:	out std_logic_vector(7 downto 0); -- received data
	Rx_err:	out std_logic; -- error in receiving, active high
	Rxd:	in std_logic; -- receiver input signal
	Rx_busy:	out std_logic -- receiver busy, active low
);
end entity;

architecture behavior of top is

component UART is
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
end component;

component button_debouncer is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	push_button:	in std_logic; -- pressed low
	pulse:	out std_logic -- active high, one-cycle
);
end component;

component pll IS
	PORT
	(
		inclk0		: IN STD_LOGIC  := '0';
		c0		: OUT STD_LOGIC 
	);
END component;

signal clk_100:std_logic;
signal Tx_en:std_logic;

begin

pll_inst:pll port map(clock,clk_100);
button_debouncer_inst:button_debouncer port map(reset,clk_100,send,Tx_en);
UART_inst:UART port map(reset,clk_100,Tx_data,Tx_en,Txd,Tx_busy,Rx_data,Rx_err,Rxd,Rx_busy);

end architecture;