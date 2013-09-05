with ada.text_io, ada.integer_text_io;
use ada.text_io, ada.integer_text_io;

procedure testtype is
type Day is (M, T, W, R, F);
type table is array(1..100) of integer is array(1..20) of integer;
subtype count is integer range 0 .. 100;
cnt : count;
date : Day;
begin
	--cnt :=  101;
	date := T;
	put(cnt);	
end;
