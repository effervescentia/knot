open Kore;

let is_main: A.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;

let unary_op = (ctx, ~type_: option(TR.t)=?, f, x) =>
  N.typed(f(x), type_ |?: N.get_type(x), N.get_range(x));

let binary_op = (ctx, ~get_type=_ => Type.Raw.(`Unknown), f, (l, r)) =>
  Node.typed((l, r) |> f, (l, r) |> get_type, Node.join_ranges(l, r));

let get_arithmetic_result_type:
  ((AST.Raw.expression_t, AST.Raw.expression_t)) => Type.Raw.t =
  TR.(
    Tuple.map2(N.get_type)
    % (
      fun
      | (`Float, _)
      | (_, `Float) => `Float
      | (`Integer, `Integer) => `Integer

      /* cannot determine the type early in this case */
      | _ => `Unknown
    )
  );

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
