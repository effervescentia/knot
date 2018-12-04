open Angstrom;

let foo = () => print_endline("Hello");

let chs_to_string = chs =>
  List.rev(chs) |> List.fold_left((acc, c) => acc ++ String.make(1, c), "");
let chs_to_number = chs => chs_to_string(chs) |> int_of_string;