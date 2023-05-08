open Kore;

let __first_char = 97;

let gen_variable =
  fun
  | x when x < 0 => "a"
  | x =>
    x
    mod 25
    |> (+)(__first_char)
    |> char_of_int
    |> String.make(1)
    |> Fmt.str("%*s", x / 25);
