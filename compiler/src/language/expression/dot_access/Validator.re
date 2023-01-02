open Knot.Kore;
open AST;

let validate: (string, Type.t) => option(Type.error_t) =
  prop =>
    fun
    /* assume this has been reported already and ignore */
    | Invalid(_) => None

    | Valid(`Object(props))
        when props |> List.exists(((name, _)) => name == prop) =>
      None

    | Valid(`Module(entries))
        when entries |> List.exists(((name, _)) => name == prop) =>
      None

    | type_ => Some(InvalidDotAccess(type_, prop));
