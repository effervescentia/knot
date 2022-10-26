open Kore;

module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = ParseContext.create(~report=ignore, __namespace);
let __scope = S.create(__context, Range.zero);
let __throw_scope = S.create({...__context, report: throw}, Range.zero);

let suite =
  "Analyze.Semantic | Binary Operation"
  >::: [
    "resolve valid 'and' (&&) and 'or' (||) operations as boolean type"
    >: (
      () =>
        [A.LogicalAnd, A.LogicalOr]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.bool_prim(true), URes.bool_prim(false))
               |> A.of_binary_op
               |> URes.as_bool,
               (op, URaw.bool_prim(true), URaw.bool_prim(false))
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'and' (&&) and 'or' (||) operations as boolean type"
    >: (
      () =>
        [A.LogicalAnd, A.LogicalOr]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"), URes.nil_prim)
               |> A.of_binary_op
               |> URes.as_bool,
               (op, URaw.string_prim("foo"), URaw.nil_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid comparative operations (<, <=, >, >=) as boolean type"
    >: (
      () =>
        [A.LessThan, A.LessOrEqual, A.GreaterThan, A.GreaterOrEqual]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.int_prim(123), URes.int_prim(456))
               |> A.of_binary_op
               |> URes.as_bool,
               (op, URaw.int_prim(123), URaw.int_prim(456))
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve invalid comparative operations (<, <=, >, >=) as boolean type"
    >: (
      () =>
        [A.LessThan, A.LessOrEqual, A.GreaterThan, A.GreaterOrEqual]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"), URes.nil_prim)
               |> A.of_binary_op
               |> URes.as_bool,
               (op, URaw.string_prim("foo"), URaw.nil_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid 'equal' (==) and 'unequal' (!=) operations as boolean type"
    >: (
      () =>
        [A.Equal, A.Unequal]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"), URes.string_prim("bar"))
               |> A.of_binary_op
               |> URes.as_bool,
               (op, URaw.string_prim("foo"), URaw.string_prim("bar"))
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'equal' (==) and 'unequal' (!=) operations as boolean type"
    >: (
      () =>
        [A.Equal, A.Unequal]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"), URes.nil_prim)
               |> A.of_binary_op
               |> URes.as_bool,
               (op, URaw.string_prim("foo"), URaw.nil_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid 'divide' (/) and 'exponent' (^) operations as float type"
    >: (
      () =>
        [A.Divide, A.Exponent]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.int_prim(123), (45.6, 7) |> URes.float_prim)
               |> A.of_binary_op
               |> URes.as_float,
               (op, URaw.int_prim(123), (45.6, 7) |> URaw.float_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'divide' (/) and 'exponent' (^) operations as float type"
    >: (
      () =>
        [A.Divide, A.Exponent]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"), URes.nil_prim)
               |> A.of_binary_op
               |> URes.as_float,
               (op, URaw.string_prim("foo"), URaw.nil_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid 'add' (+), 'subtract' (-) and 'multiply' (*) operations as integer type"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.int_prim(123), URes.int_prim(456))
               |> A.of_binary_op
               |> URes.as_int,
               (op, URaw.int_prim(123), URaw.int_prim(456))
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve valid 'add' (+), 'subtract' (-) and 'multiply' (*) operations as float type"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.int_prim(123), (45.6, 7) |> URes.float_prim)
               |> A.of_binary_op
               |> URes.as_float,
               (op, URaw.int_prim(123), (45.6, 7) |> URaw.float_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve partially valid 'add' (+), 'subtract' (-) and 'multiply' (*) operations as float type"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 op,
                 __id |> A.of_id |> URes.as_invalid(NotInferrable),
                 (45.6, 7) |> URes.float_prim,
               )
               |> A.of_binary_op
               |> URes.as_float,
               (
                 op,
                 __id |> AR.of_id |> URaw.as_node,
                 (45.6, 7) |> URaw.float_prim,
               )
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve 'add' (+), 'subtract' (-) and 'multiply' (*) operations as invalid LHS type"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 op,
                 __id |> A.of_id |> URes.as_invalid(NotInferrable),
                 URes.int_prim(456),
               )
               |> A.of_binary_op
               |> URes.as_invalid(NotInferrable),
               (op, __id |> AR.of_id |> URaw.as_node, URaw.int_prim(456))
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve 'add' (+), 'subtract' (-) and 'multiply' (*) operations as invalid RHS type"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 op,
                 URes.int_prim(456),
                 __id |> A.of_id |> URes.as_invalid(NotInferrable),
               )
               |> A.of_binary_op
               |> URes.as_invalid(NotInferrable),
               (op, URaw.int_prim(456), __id |> AR.of_id |> URaw.as_node)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "resolve NotInferrable for 'add' (+), 'subtract' (-) and 'multiply' (*) operations on invalid type"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (op, URes.string_prim("foo"), URes.nil_prim)
               |> A.of_binary_op
               |> URes.as_invalid(NotInferrable),
               (op, URaw.string_prim("foo"), URaw.nil_prim)
               |> AR.of_binary_op
               |> URaw.as_node
               |> KExpression.Plugin.analyze(__scope),
             )
           )
    ),
    "report InvalidBinaryOperation error on unexpected expression types"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalAnd,
                  Valid(`String),
                  Valid(`Nil),
                ),
              ),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (AR.LogicalAnd, URaw.string_prim("foo"), URaw.nil_prim)
          |> AR.of_binary_op
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__throw_scope)
        )
    ),
  ];
