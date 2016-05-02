----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: Thomas Junbin ZHANG
-- 
-- Create Date: 26/Feb/2016
-- Design Name: UART
-- Module Name: TX
-- Project Name: FPGA_Workshop
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The TX module of a simple UART for the following protocol: 19200 baud rate (100 MHz clock), no parity-check bit, 8 bits in one byte and one stopping bit.  
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

entity TX is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	Tx_data:	in std_logic_vector(7 downto 0); -- to-be-transmitted data
	Tx_en:	in std_logic; -- transmitter enable, active high, one clock
	Txd:	out std_logic; -- transmitter output signal
	Tx_busy:	out std_logic --transmitter busy, active high
);
end TX;

architecture Behavioral of TX is
	type states is (idle, start, bit0, bit1, bit2, bit3, bit4, bit5, bit6, bit7, stop);
	signal state : states := idle;
	constant counter_max : integer := 5207; -- For 19200 baud rate
	signal counter : integer range 0 to counter_max - 1;
	signal reg : std_logic_vector(7 downto 0);
begin
	process(clk, reset)
	begin
		if (reset = '0') then
			-- Reset outputs
			Txd <= '1';
			-- Reset signals
			state <= idle;
			counter <= 0;
			reg <= (others => '0');
		elsif rising_edge(clk) then
			if (Tx_en = '1') and (state = idle) then
				-- Load register
				reg <= Tx_data;
				-- Output start bit
				Txd <= '0';
				-- Reset counter
				counter <= 0;
				-- Jump to start state
				state <= start;
			elsif (state /= idle) and (counter < counter_max - 1) then
				counter <= counter + 1;
			elsif (state /= idle) then
				case state is
					when start => Txd <= reg(0); counter <= 0; state <= bit0;
					when bit0 => Txd <= reg(1); counter <= 0; state <= bit1;
					when bit1 => Txd <= reg(2); counter <= 0; state <= bit2;
					when bit2 => Txd <= reg(3); counter <= 0; state <= bit3;
					when bit3 => Txd <= reg(4); counter <= 0; state <= bit4;
					when bit4 => Txd <= reg(5); counter <= 0; state <= bit5;
					when bit5 => Txd <= reg(6); counter <= 0; state <= bit6;
					when bit6 => Txd <= reg(7); counter <= 0; state <= bit7;
					when bit7 => Txd <= '1'; counter <= 0; state <= stop;
					when stop => Txd <= '1'; counter <= 0; state <= idle;
					when others => state <= idle;
				end case;
			end if;
		end if;
	end process;
	
	Tx_busy <= '0' when (state = idle) else '1';
end Behavioral;