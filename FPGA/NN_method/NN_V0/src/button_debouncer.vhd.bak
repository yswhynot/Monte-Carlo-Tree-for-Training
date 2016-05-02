----------------------------------------------------------------------------------
-- Company: The Hong Kong Polytechnic University (FPGA interest group led by Dr. Bruce Sham)
-- Engineer/Student: 
-- 
-- Create Date:    
-- Design Name: 
-- Module Name:    
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: This module is used as the interface between the user push-button and a digital system. It converts a bouncing press (active low) into a synchronous request signal (active high) lasting for one system clock cycle.
--
-- Dependencies: 
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

entity button_debouncer is
port (
	reset: 	in std_logic; -- active low
	clk:	in std_logic; -- clock
	push_button:	in std_logic; -- pressed low
	pulse:	out std_logic -- active high, one-cycle
);
end button_debouncer;

architecture Behavioral of button_debouncer is

signal push_button_sync:std_logic_vector(1 downto 0):="11";
signal sampling_counter:std_logic_vector(15 downto 0):=(others=>'0');
signal sampling_enable:std_logic;
signal pulse_counter:std_logic_vector(3 downto 0):="0000";


begin

--synchronizer
	process(clk, reset)
	begin
		if (clk = '1' and clk'event) then
			push_button_sync(0)<=push_button;
			push_button_sync(1)<=push_button_sync(0);
		end if;
	end process;
			
--sampler
	process(clk, reset)
	begin
		if (reset = '0') then
			sampling_counter<=(others=>'0');
		elsif (clk = '1' and clk'event) then
			sampling_counter<=sampling_counter+'1';
		end if;
	end process;
	
--pulse generator	
	process(clk, reset)
	begin
		if (reset = '0') then
			pulse_counter<=(others=>'0');
			pulse<='0';
		elsif (clk = '1' and clk'event) then
			if (sampling_enable='1') then
				if (pulse_counter<"1000") then				
					if (push_button_sync(1)='1') then
						pulse_counter<=(others=>'0');
					else
						pulse_counter<=pulse_counter+'1';
					end if;
				else 
					if (push_button_sync(1)='0') then
						--pulse_counter<=(3=>'1',others=>'0');
					else
						pulse_counter<=pulse_counter+'1';
					end if;
				end if;
			end if;
			
			if (pulse_counter="1111") and (sampling_enable='1') and (push_button_sync(1)='1') then
				pulse<='1';
			else
				pulse<='0';
			end if;
		end if;
	end process;


----- combinational logic
sampling_enable<='1' when sampling_counter=x"ffff" else '0';

end Behavioral;

