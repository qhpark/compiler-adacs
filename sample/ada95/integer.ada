with ada.text_io, ada.integer_text_io;
use ada.text_io, ada.integer_text_io;

procedure compute is

	procedure double(item : in out integer) is
	begin
		item := item * 2;
	end;
	
x : integer := 1;
begin
	loop
		put(x);
		double(x);
		new_line;
	end loop;
end;
