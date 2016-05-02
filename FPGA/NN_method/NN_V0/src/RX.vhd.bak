----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: Thomas Junbin ZHANG
-- 
-- Create Date: 26/Feb/2016
-- Design Name: UART
-- Module Name: RX
-- Project Name: FPGA_Workshop
-- Target Devices: EP4CE115F29C8
-- Tool versions: Quartus II 13.0sp1 (64-bit)
-- Description: The RX module of a simple UART for the following protocol: 19200 baud rate (100 MHz clock), no parity-check bit, 8 bits in one byte and one stopping bit.  
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

entity RX is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	Rx_data:	out std_logic_vector(7 downto 0); -- received data
	Rx_err:	out std_logic; -- error in receiving, active high
	Rxd:	in std_logic; -- receiver input signal
	Rx_busy:	out std_logic -- receiver busy, active low
);
end RX;

architecture Behavioral of RX is
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
			Rx_data <= (others => '0');
			Rx_err <= '0';
			-- Reset signals
			state <= idle;
			counter <= 0;
			reg <= (others => '0');
		elsif rising_edge(clk) then
			if (Rxd = '0') and (state = idle) then
				-- Reset counter
				counter <= 0;
				-- Jump to start state
				state <= start;
			elsif (state /= idle) then
				-- Read data
				if (counter = counter_max / 2) then
					case state is
						when bit0 => reg(0) <= Rxd;
						when bit1 => reg(1) <= Rxd;
						when bit2 => reg(2) <= Rxd;
						when bit3 => reg(3) <= Rxd;
						when bit4 => reg(4) <= Rxd;
						when bit5 => reg(5) <= Rxd;
						when bit6 => reg(6) <= Rxd;
						when bit7 => reg(7) <= Rxd;
						when stop =>
							if (Rxd /= '1') then
								Rx_err <= '1';
							end if;
						when others => null;
					end case;
					counter <= counter + 1;
				-- Update state
				elsif (counter = counter_max - 1) then
					case state is
						when start => counter <= 0; state <= bit0;
						when bit0 => counter <= 0; state <= bit1;
						when bit1 => counter <= 0; state <= bit2;
						when bit2 => counter <= 0; state <= bit3;
						when bit3 => counter <= 0; state <= bit4;
						when bit4 => counter <= 0; state <= bit5;
						when bit5 => counter <= 0; state <= bit6;
						when bit6 => counter <= 0; state <= bit7;
						when bit7 => counter <= 0; state <= stop;
						when stop => counter <= 0; state <= idle; Rx_data <= reg;
						when others => state <= idle;
					end case;
				else
					counter <= counter + 1;
				end if;
			end if;
		end if;
	end process;
	
	Rx_busy <= '0' when (state = idle) else '1';
end Behavioral;

