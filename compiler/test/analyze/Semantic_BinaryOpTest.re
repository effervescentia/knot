open Kore;

module OB = AST.Operator.Binary;
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
  "Analyze.Semantic | Binary Operation"
  >::: [
    "resolve valid 'and' (&&) and 'or' (||) operations as boolean type"
    >: (
      () =>
        [OB.LogicalAnd, OB.LogicalOr]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.bool_prim(true), URes.bool_prim(false))
                 |> Expression.of_binary_op(op),
                 Valid(Boolean),
               ),
               (URaw.bool_prim(true), URaw.bool_prim(false))
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'and' (&&) and 'or' (||) operations as boolean type"
    >: (
      () =>
        [OB.LogicalAnd, OB.LogicalOr]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.string_prim("foo"), URes.nil_prim)
                 |> Expression.of_binary_op(op),
                 Valid(Boolean),
               ),
               (URaw.string_prim("foo"), URaw.nil_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve valid comparative operations (<, <=, >, >=) as boolean type"
    >: (
      () =>
        [OB.LessThan, OB.LessOrEqual, OB.GreaterThan, OB.GreaterOrEqual]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.int_prim(123), URes.int_prim(456))
                 |> Expression.of_binary_op(op),
                 Valid(Boolean),
               ),
               (URaw.int_prim(123), URaw.int_prim(456))
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve invalid comparative operations (<, <=, >, >=) as boolean type"
    >: (
      () =>
        [OB.LessThan, OB.LessOrEqual, OB.GreaterThan, OB.GreaterOrEqual]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.string_prim("foo"), URes.nil_prim)
                 |> Expression.of_binary_op(op),
                 Valid(Boolean),
               ),
               (URaw.string_prim("foo"), URaw.nil_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve valid 'equal' (==) and 'unequal' (!=) operations as boolean type"
    >: (
      () =>
        [OB.Equal, OB.Unequal]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.string_prim("foo"), URes.string_prim("bar"))
                 |> Expression.of_binary_op(op),
                 Valid(Boolean),
               ),
               (URaw.string_prim("foo"), URaw.string_prim("bar"))
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'equal' (==) and 'unequal' (!=) operations as boolean type"
    >: (
      () =>
        [OB.Equal, OB.Unequal]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.string_prim("foo"), URes.nil_prim)
                 |> Expression.of_binary_op(op),
                 Valid(Boolean),
               ),
               (URaw.string_prim("foo"), URaw.nil_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve valid 'divide' (/) and 'exponent' (^) operations as float type"
    >: (
      () =>
        [OB.Divide, OB.Exponent]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.int_prim(123), (45.6, 7) |> URes.float_prim)
                 |> Expression.of_binary_op(op),
                 Valid(Float),
               ),
               (URaw.int_prim(123), (45.6, 7) |> URaw.float_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve invalid 'divide' (/) and 'exponent' (^) operations as float type"
    >: (
      () =>
        [OB.Divide, OB.Exponent]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.string_prim("foo"), URes.nil_prim)
                 |> Expression.of_binary_op(op),
                 Valid(Float),
               ),
               (URaw.string_prim("foo"), URaw.nil_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve valid 'add' (+), 'subtract' (-) and 'multiply' (*) operations as integer type"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.int_prim(123), URes.int_prim(456))
                 |> Expression.of_binary_op(op),
                 Valid(Integer),
               ),
               (URaw.int_prim(123), URaw.int_prim(456))
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve valid 'add' (+), 'subtract' (-) and 'multiply' (*) operations as float type"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.int_prim(123), (45.6, 7) |> URes.float_prim)
                 |> Expression.of_binary_op(op),
                 Valid(Float),
               ),
               (URaw.int_prim(123), (45.6, 7) |> URaw.float_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve partially valid 'add' (+), 'subtract' (-) and 'multiply' (*) operations as float type"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 (
                   __id
                   |> Expression.of_identifier
                   |> URes.as_invalid(NotInferrable),
                   (45.6, 7) |> URes.float_prim,
                 )
                 |> Expression.of_binary_op(op),
                 Valid(Float),
               ),
               (
                 __id |> Expression.of_identifier |> URaw.as_node,
                 (45.6, 7) |> URaw.float_prim,
               )
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve 'add' (+), 'subtract' (-) and 'multiply' (*) operations as invalid LHS type"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 (
                   __id
                   |> Expression.of_identifier
                   |> URes.as_invalid(NotInferrable),
                   URes.int_prim(456),
                 )
                 |> Expression.of_binary_op(op),
                 Invalid(NotInferrable),
               ),
               (
                 __id |> Expression.of_identifier |> URaw.as_node,
                 URaw.int_prim(456),
               )
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve 'add' (+), 'subtract' (-) and 'multiply' (*) operations as invalid RHS type"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 (
                   URes.int_prim(456),
                   __id
                   |> Expression.of_identifier
                   |> URes.as_invalid(NotInferrable),
                 )
                 |> Expression.of_binary_op(op),
                 Invalid(NotInferrable),
               ),
               (
                 URaw.int_prim(456),
                 __id |> Expression.of_identifier |> URaw.as_node,
               )
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
             )
           )
    ),
    "resolve NotInferrable for 'add' (+), 'subtract' (-) and 'multiply' (*) operations on invalid type"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply]
        |> List.iter(op =>
             Assert.expression(
               (
                 (URes.string_prim("foo"), URes.nil_prim)
                 |> Expression.of_binary_op(op),
                 Invalid(NotInferrable),
               ),
               (URaw.string_prim("foo"), URaw.nil_prim)
               |> Expression.of_binary_op(op)
               |> URaw.as_node
               |> Expression.analyze(__scope),
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
                  Valid(String),
                  Valid(Nil),
                ),
              ),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (URaw.string_prim("foo"), URaw.nil_prim)
          |> Expression.of_and_op
          |> URaw.as_node
          |> Expression.analyze(__throw_scope)
        )
    ),
  ];
