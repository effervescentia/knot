open Kore;

module A = AST.Result;
module AR = AST.Raw;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = AST.ParseContext.create(~report=ignore, __namespace);
let __scope = AST.Scope.create(__context, Range.zero);
let __throw_scope =
  AST.Scope.create({...__context, report: AST.Error.throw}, Range.zero);

let suite =
  "Analyze.Semantic | Unary Operation"
  >::: [
    "resolve valid 'not' (!) operation as boolean type"
    >: (
      () =>
        Assert.expression(
          (A.Not, URes.bool_prim(true)) |> A.of_unary_op |> URes.as_bool,
          (AR.Not, URaw.bool_prim(true))
          |> AR.of_unary_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve invalid 'not' (!) operation as boolean type"
    >: (
      () =>
        Assert.expression(
          (A.Not, __id |> A.of_id |> URes.as_invalid(NotInferrable))
          |> A.of_unary_op
          |> URes.as_bool,
          (AR.Not, __id |> AR.of_id |> URaw.as_node)
          |> AR.of_unary_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve valid 'positive' (+) and 'negative' (-) operations as integer type"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.int_prim(123)) |> A.of_unary_op |> URes.as_int,
               (op, URaw.int_prim(123))
               |> AR.of_unary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid 'positive' (+) and 'negative' (-) operations as float type"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.expression(
               (op, (123.456, 3) |> URes.float_prim)
               |> A.of_unary_op
               |> URes.as_float,
               (op, (123.456, 3) |> URaw.float_prim)
               |> AR.of_unary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'positive' (+) and 'negative' (-) operations as inner expression type"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.expression(
               (op, __id |> A.of_id |> URes.as_invalid(NotInferrable))
               |> A.of_unary_op
               |> URes.as_invalid(NotInferrable),
               (op, __id |> AR.of_id |> URaw.as_node)
               |> AR.of_unary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve NotInferrable type on non-numeric 'positive' (+) and 'negative' (-) operations"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"))
               |> A.of_unary_op
               |> URes.as_invalid(NotInferrable),
               (op, URaw.string_prim("foo"))
               |> AR.of_unary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "report InvalidUnaryOperation error on unexpected inner expression type"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidUnaryOperation(Not, Valid(`String))),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (AR.Not, URaw.string_prim("foo"))
          |> AR.of_unary_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__throw_scope)
        )
    ),
  ];
