-- Create a name clash
package package1 is
    type y is private;
    x : constant String;
    type rgb is (red, blue, green);
    type arr10 is array (1..10,1..2) of string;
    i : (red, blue, green);
    subtype f00 is integer range 100-99 .. 10+100;
    subtype f03 is integer;
    --subtype f01 is f01 range 2 .. 10;
    subtype f01 is f00 range 2 .. 10;
    -- subtype f02 is float (2 .. 10, 1 .. 2 );
    kwan : array (1..5) of integer := (1);
    type arr20 is array (arr10 range < > ) of boolean;
    nawk : array (arr10 range < > ) of arr10;
    alan : arr20;
    qha : arr20;
    type pKwan is access integer range 5 ..1;
    pKwan2 : access integer range 1 .. 5;
    pKwan3 : access pKwan;
private
    subtype blah is blahblah;
    subtype foo is integer range 100 .. 10+100;
end ;
