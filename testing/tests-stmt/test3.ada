-- Create a name clash
package package1 is
    type y is private;
    x : constant String;
    type rgb is (red, blue, green);
    type arr10 is array (1..10,1..2) of string;
    i : (red, blue, green);
    subtype f00 is integer range 100-99 .. 10+100;
private
    subtype blah is blahblah;
    subtype foo is integer range 100 .. 10+100;
end ;
