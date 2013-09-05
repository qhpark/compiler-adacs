
-- Create a name clash
package body package1 is
  --  type y is private;
    x : constant String;
    procedure proc1 is 
        x: Integer;
    begin
        null;
    end;
    procedure proc1 is 
        x: Integer;
    begin
        null;
    end;

    y, z : Integer;
--private    
--subtype blah is blahblah;
--    subtype foo is foofoo;
begin

end ;
