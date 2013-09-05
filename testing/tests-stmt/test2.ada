-- Create a name clash
package package1 is
    type y is private;
    x : constant String;
end ;
