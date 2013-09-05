with Text_IO; use Text_IO;
with Ada.integer_text_io; use Ada.integer_text_io;
procedure Forever is
x : integer;
begin
	loop
	get(x);
	exit when x= 0;
	put(x * x);
	new_line;
	end loop ;
end;
