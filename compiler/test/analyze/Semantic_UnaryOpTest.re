open Kore;

module A = AST.Result;
module AR = AST.Raw;
module OU = AST.Operator.Unary;
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
          true |> URes.bool_prim |> A.of_not_op |> URes.as_bool,
          true
          |> URaw.bool_prim
          |> AR.of_not_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve invalid 'not' (!) operation as boolean type"
    >: (
      () =>
        Assert.expression(
          __id
          |> A.of_id
          |> URes.as_invalid(NotInferrable)
          |> A.of_not_op
          |> URes.as_bool,
          __id
          |> AR.of_id
          |> URaw.as_node
          |> AR.of_not_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve valid 'positive' (+) and 'negative' (-) operations as integer type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               123 |> URes.int_prim |> A.of_unary_op(op) |> URes.as_int,
               123
               |> URaw.int_prim
               |> AR.of_unary_op(op)
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid 'positive' (+) and 'negative' (-) operations as float type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               (123.456, 3)
               |> URes.float_prim
               |> A.of_unary_op(op)
               |> URes.as_float,
               (123.456, 3)
               |> URaw.float_prim
               |> AR.of_unary_op(op)
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'positive' (+) and 'negative' (-) operations as inner expression type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               __id
               |> A.of_id
               |> URes.as_invalid(NotInferrable)
               |> A.of_unary_op(op)
               |> URes.as_invalid(NotInferrable),
               __id
               |> AR.of_id
               |> URaw.as_node
               |> AR.of_unary_op(op)
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve NotInferrable type on non-numeric 'positive' (+) and 'negative' (-) operations"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               "foo"
               |> URes.string_prim
               |> A.of_unary_op(op)
               |> URes.as_invalid(NotInferrable),
               "foo"
               |> URaw.string_prim
               |> AR.of_unary_op(op)
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
          "foo"
          |> URaw.string_prim
          |> AR.of_not_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__throw_scope)
        )
    ),
  ];
