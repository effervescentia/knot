open Kore;

let is_main: A.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;

let unary_op = (f, x) => N.typed(f(x), (), N.get_range(x));

let binary_op = (f, (l, r)) =>
  Node.typed((l, r) |> f, (), Node.join_ranges(l, r));

let define_statement = (kwd, parser, f) =>
  kwd
  >|= Node.get_range
  >>= (
    start =>
      Matchers.identifier(~prefix=Matchers.alpha)
      >>= (
        id =>
          Symbol.colon
          >> parser
          >|= (
            ((res, range)) =>
              Node.untyped(
                (id, res) |> f,
                Range.join(start, range |?: start),
              )
          )
      )
  );
