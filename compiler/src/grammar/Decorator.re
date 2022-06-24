open Kore;

let parser = parse_expr =>
  Symbol.at_sign
  >>= N.get_range
  % (
    start =>
      M.identifier
      >>= (
        id =>
          parse_expr
          |> M.comma_sep
          |> M.between(Symbol.open_closure, Symbol.close_closure)
          |> option(N.untyped([], start))
      )
  );
