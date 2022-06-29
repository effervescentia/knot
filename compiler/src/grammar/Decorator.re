open Kore;

let parser = parse_expr =>
  Symbol.at_sign
  >>= (
    start =>
      M.identifier
      >>= (
        id =>
          parse_expr
          |> M.comma_sep
          |> M.between(Symbol.open_group, Symbol.close_group)
          |> option(N.untyped([], N.get_range(id)))
          >|= (
            args =>
              N.untyped(
                (id |> N.add_type(TR.(`Unknown)), fst(args)),
                N.join_ranges(start, args),
              )
          )
      )
  );
