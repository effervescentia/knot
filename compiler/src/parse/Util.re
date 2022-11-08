open Kore;
open Onyx;

let unary_op = (f, x) => N.typed(f(x), (), N.get_range(x));

let binary_op = (f, (l, r)) =>
  Node.typed((l, r) |> f, (), Node.join_ranges(l, r));

let define_statement = (kwd, parser, f) =>
  kwd
  >|= Node.get_range
  >>= (
    start =>
      Matchers.attribute(Matchers.identifier(~prefix=Matchers.alpha), parser)
      >|= (
        ((id, (res, range))) =>
          Node.untyped((id, res) |> f, Range.join(start, range |?: start))
      )
  );
