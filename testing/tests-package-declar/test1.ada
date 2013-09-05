-- Just do a declaration for a package
pragma foo;
package thebestpackage is
    x, y: Integer := 9;
    use pkg1, pkg1;
    type supertype is array (1..10) of Integer;
    procedure proc1 (i : Integer);

    type privateType is private;
    function func1 (param1 : in Boolean) return String;
    type privateType2 is private;
end thebestpackage;
