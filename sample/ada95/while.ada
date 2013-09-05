with Text_IO; use Text_IO;
with Ada.integer_text_io; use Ada.integer_text_io;
procedure Forever is
n : integer;
begin
	while n < 20
	loop
	put(n);
	n := n +1;
	end loop ;
end;
