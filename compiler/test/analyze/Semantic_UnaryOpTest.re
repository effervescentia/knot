open Kore;

module OU = AST.Operator.Unary;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context: AST.ParseContext.t(Language.Interface.program_t(AST.Type.t)) =
  AST.ParseContext.create(~report=ignore, __namespace);
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
          (true |> URes.bool_prim |> Expression.of_not_op, Valid(Boolean)),
          true
          |> URaw.bool_prim
          |> Expression.of_not_op
          |> URaw.as_node
          |> Expression.analyze(__scope),
        )
    ),
    "resolve invalid 'not' (!) operation as boolean type"
    >: (
      () =>
        Assert.expression(
          (
            __id
            |> Expression.of_identifier
            |> URes.as_invalid(NotInferrable)
            |> Expression.of_not_op,
            Valid(Boolean),
          ),
          __id
          |> Expression.of_identifier
          |> URaw.as_node
          |> Expression.of_not_op
          |> URaw.as_node
          |> Expression.analyze(__scope),
        )
    ),
    "resolve valid 'positive' (+) and 'negative' (-) operations as integer type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               (
                 123 |> URes.int_prim |> Expression.of_unary_op(op),
                 Valid(Integer),
               ),
               123
               |> URaw.int_prim
               |> Expression.of_unary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve valid 'positive' (+) and 'negative' (-) operations as float type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               (
                 (123.456, 3) |> URes.float_prim |> Expression.of_unary_op(op),
                 Valid(Float),
               ),
               (123.456, 3)
               |> URaw.float_prim
               |> Expression.of_unary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'positive' (+) and 'negative' (-) operations as inner expression type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               (
                 __id
                 |> Expression.of_identifier
                 |> URes.as_invalid(NotInferrable)
                 |> Expression.of_unary_op(op),
                 Invalid(NotInferrable),
               ),
               __id
               |> Expression.of_identifier
               |> URaw.as_node
               |> Expression.of_unary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve NotInferrable type on non-numeric 'positive' (+) and 'negative' (-) operations"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.expression(
               (
                 "foo" |> URes.string_prim |> Expression.of_unary_op(op),
                 Invalid(NotInferrable),
               ),
               "foo"
               |> URaw.string_prim
               |> Expression.of_unary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "report InvalidUnaryOperation error on unexpected inner expression type"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidUnaryOperation(Not, Valid(String))),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          "foo"
          |> URaw.string_prim
          |> Expression.of_not_op
          |> URaw.as_node
          |> Expression.analyze(__throw_scope)
        )
    ),
  ];
