open Knot.Kore;
open Onyx;

let unary_op = (f, x) => Node.wrap(f, x);

let binary_op = (f, (l, r)) =>
  Node.raw((l, r) |> f, Node.join_ranges(l, r));

let define_statement = (kwd, parser, f) =>
  kwd
  >|= Node.get_range
  >>= (
    start =>
      Matchers.attribute(Matchers.identifier(~prefix=Matchers.alpha), parser)
      >|= (
        ((id, (res, range))) =>
          Node.raw((id, res) |> f, Range.join(start, range |?: start))
      )
  );
