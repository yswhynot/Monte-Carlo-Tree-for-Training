library ieee;
use ieee.std_logic_1164.all;

package my_package is

-- board representation (tile representation : 3 bits)	
	subtype tile_type is std_logic_vector(2 downto 0);
	type boardColumn_type is array (0 to 19) of tile_type;
	type board_type is array (0 to 19) of boardColumn_type;
	--subtype edgesColumn_type is array (0 to 19) of std_logic;
	--type edgesNet_type is array (0 to 19) of edgesColumn_type;
	
-- Neural Network
	constant weight_length : integer:= 18;	
	subtype weight_type is std_logic_vector(weight_length - 1 downto 0);
	type weight_network_type is array (natural range <>) of weight_type;

end my_package;