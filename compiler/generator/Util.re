open Globals;

exception SafeVariableNameImpossible;

let safe_variable_regex = Str.regexp("[_a-zA-Z][_a-zA-Z0-9]*$");

let get_safe_char =
  fun
  | '_' as ch
  | 'a'..'z' as ch
  | 'A'..'Z' as ch => ch
  | _ => '_';

let rec generate_safe_variable_name = s =>
  String.length(s) == 0
    ? s
    : (get_safe_char(s.[0]) |> String.make(1))
      ++ generate_safe_variable_name(String.sub(s, 1, String.length(s) - 1));

let get_safe_variable_name =
  fun
  | s when Str.string_match(safe_variable_regex, s, 0) => s
  | s when String.length(s) > 1 =>
    injected_variable ++ "_" ++ generate_safe_variable_name(s)
  | _ => raise(SafeVariableNameImpossible);
