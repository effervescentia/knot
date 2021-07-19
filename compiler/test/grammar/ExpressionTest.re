open Kore;
open AST.Raw.Util;
open Util;

module Expression = Grammar.Expression;

module Assert =
  Assert.Make({
    open AST.Raw;

    type t = expression_t;

    let parser = ctx => Parser.parse(Expression.parser(ctx));

    let test =
      Alcotest.(
        check(
          testable(
            (pp, (value, type_, cursor)) =>
              Debug.print_typed_lexeme(
                "Expression",
                value |> Debug.print_expr,
                type_,
                cursor,
              )
              |> Cow.Xml.to_string
              |> Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Expression"
  >::: [
    "no parse" >: (() => ["~gibberish"] |> Assert.no_parse),
    "parse primitive" >: (() => Assert.parse("123", int_prim(123))),
    "parse identifier"
    >: (
      () =>
        [
          (
            "foo",
            "foo" |> to_public_id(as_invalid(NotFound(Public("foo")))),
          ),
          ("bar", "bar" |> to_public_id(as_int)),
        ]
        |> Assert.parse_many(
             ~report=ignore,
             ~scope=to_scope([("bar", K_Strong(K_Integer))]),
           )
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          ~scope=to_scope([("foo", K_Strong(K_Boolean))]),
          "(foo)",
          "foo" |> to_public_id(as_bool) |> to_group |> as_bool,
        )
    ),
    "parse closure"
    >: (
      () =>
        [
          (
            "{
            foo;
            let x = false;
            let y = foo;
            let z = y;
            1 + 2;
          }",
            [
              "foo" |> to_public_id(as_string) |> to_expr,
              ("x" |> to_public |> as_lexeme, bool_prim(false)) |> to_var,
              (
                "y" |> to_public |> as_lexeme,
                "foo" |> to_public_id(as_string),
              )
              |> to_var,
              ("z" |> to_public |> as_lexeme, "y" |> to_public_id(as_string))
              |> to_var,
              (int_prim(1), int_prim(2)) |> to_add_op |> as_int |> to_expr,
            ]
            |> to_closure
            |> as_int,
          ),
        ]
        |> Assert.parse_many(
             ~scope=to_scope([("foo", K_Strong(K_String))]),
           )
    ),
    "parse unary"
    >: (
      () =>
        [
          ("-123", 123 |> int_prim |> to_neg_int),
          ("!true", true |> bool_prim |> to_not_bool),
        ]
        |> Assert.parse_many
    ),
    "parse boolean logic"
    >: (
      () =>
        [("&&", to_and_op), ("||", to_or_op)]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("true%sfalse"),
                 (bool_prim(true), bool_prim(false)) |> tag |> as_bool,
               ),
               (
                 op |> Print.fmt(" true %s false "),
                 (bool_prim(true), bool_prim(false)) |> tag |> as_bool,
               ),
             ]
           )
        |> List.flatten
        |> Assert.parse_many
    ),
    "parse arithmetic"
    >: (
      () =>
        [
          ("+", to_add_op % as_int),
          ("-", to_sub_op % as_int),
          ("*", to_mult_op % as_int),
          ("/", to_div_op % as_float),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("123%s456"),
                 (int_prim(123), int_prim(456)) |> tag,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (int_prim(123), int_prim(456)) |> tag,
               ),
             ]
           )
        |> List.flatten
        |> Assert.parse_many
    ),
    "parse comparison"
    >: (
      () =>
        [
          ("<=", to_lte_op),
          ("<", to_lt_op),
          (">=", to_gte_op),
          (">", to_gt_op),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("123%s456"),
                 (int_prim(123), int_prim(456)) |> tag |> as_bool,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (int_prim(123), int_prim(456)) |> tag |> as_bool,
               ),
             ]
           )
        |> List.flatten
        |> Assert.parse_many
    ),
    "parse complex expression"
    >: (
      () =>
        [
          (
            "2 + 3 * 4 ^ 5 - -6 / 7",
            (
              (
                int_prim(2),
                (
                  int_prim(3),
                  (int_prim(4), int_prim(5)) |> to_expo_op |> as_float,
                )
                |> to_mult_op
                |> as_float,
              )
              |> to_add_op
              |> as_float,
              (int_prim(6) |> to_neg_int, int_prim(7))
              |> to_div_op
              |> as_float,
            )
            |> to_sub_op
            |> as_float,
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
            (
              (int_prim(2), int_prim(3))
              |> to_add_op
              |> as_int
              |> to_group
              |> as_int,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> to_div_op
                  |> as_float
                  |> to_group
                  |> as_float
                  |> to_neg_float,
                )
                |> to_sub_op
                |> as_float
                |> to_group
                |> as_float,
              )
              |> to_expo_op
              |> as_float,
            )
            |> to_mult_op
            |> as_float,
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
            (
              (
                "a" |> to_public_id(as_bool),
                (
                  (
                    "b" |> to_public_id(as_int),
                    "c" |> to_public_id(as_float),
                  )
                  |> to_gt_op
                  |> as_bool,
                  (
                    "e" |> to_public_id(as_float),
                    "f" |> to_public_id(as_int),
                  )
                  |> to_lte_op
                  |> as_bool,
                )
                |> to_or_op
                |> as_bool
                |> to_group
                |> as_bool,
              )
              |> to_and_op
              |> as_bool,
              ("g" |> to_public_id(as_bool), "h" |> to_public_id(as_bool))
              |> to_or_op
              |> as_bool
              |> to_group
              |> as_bool
              |> to_not_bool
              |> to_group
              |> as_bool,
            )
            |> to_and_op
            |> as_bool,
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               to_scope([
                 ("a", K_Strong(K_Boolean)),
                 ("b", K_Strong(K_Integer)),
                 ("c", K_Strong(K_Float)),
                 ("e", K_Strong(K_Float)),
                 ("f", K_Strong(K_Integer)),
                 ("g", K_Strong(K_Boolean)),
                 ("h", K_Strong(K_Boolean)),
               ]),
           )
    ),
    "parse left-associative"
    >: (
      () => {
        [("+", to_add_op), ("-", to_sub_op), ("*", to_mult_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 ("a" |> to_public_id(as_int), "b" |> to_public_id(as_int))
                 |> tag
                 |> as_int,
                 "c" |> to_public_id(as_int),
               )
               |> tag
               |> as_int,
             )
           )
        |> Assert.parse_many(
             ~scope=
               to_scope([
                 ("a", K_Strong(K_Integer)),
                 ("b", K_Strong(K_Integer)),
                 ("c", K_Strong(K_Integer)),
               ]),
           );

        [("/", to_div_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 ("a" |> to_public_id(as_int), "b" |> to_public_id(as_int))
                 |> tag
                 |> as_float,
                 "c" |> to_public_id(as_int),
               )
               |> tag
               |> as_float,
             )
           )
        |> Assert.parse_many(
             ~scope=
               to_scope([
                 ("a", K_Strong(K_Integer)),
                 ("b", K_Strong(K_Integer)),
                 ("c", K_Strong(K_Integer)),
               ]),
           );

        [("&&", to_and_op), ("||", to_or_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 ("a" |> to_public_id(as_bool), "b" |> to_public_id(as_bool))
                 |> tag
                 |> as_bool,
                 "c" |> to_public_id(as_bool),
               )
               |> tag
               |> as_bool,
             )
           )
        |> Assert.parse_many(
             ~scope=
               to_scope([
                 ("a", K_Strong(K_Boolean)),
                 ("b", K_Strong(K_Boolean)),
                 ("c", K_Strong(K_Boolean)),
               ]),
           );
        [
          ("<=", to_lte_op),
          ("<", to_lt_op),
          (">=", to_gte_op),
          (">", to_gt_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> to_public_id(as_weak(0)),
                   "b" |> to_public_id(as_weak(1)),
                 )
                 |> tag
                 |> as_invalid(NotAssignable(K_Weak(0), K_Numeric)),
                 "c" |> to_public_id(as_weak(2)),
               )
               |> tag
               |> as_invalid(
                    NotAssignable(
                      K_Invalid(NotAssignable(K_Weak(0), K_Numeric)),
                      K_Numeric,
                    ),
                  ),
             )
           )
        |> Assert.parse_many(
             ~report=ignore,
             ~scope=
               to_scope([
                 ("a", K_Weak(0)),
                 ("b", K_Weak(1)),
                 ("c", K_Weak(2)),
               ]),
           );
        [("==", to_eq_op), ("!=", to_ineq_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> to_public_id(as_weak(0)),
                   "b" |> to_public_id(as_weak(1)),
                 )
                 |> tag
                 |> as_invalid(TypeMismatch(K_Weak(0), K_Weak(1))),
                 "c" |> to_public_id(as_weak(2)),
               )
               |> tag
               |> as_invalid(
                    TypeMismatch(
                      K_Invalid(TypeMismatch(K_Weak(0), K_Weak(1))),
                      K_Weak(2),
                    ),
                  ),
             )
           )
        |> Assert.parse_many(
             ~report=ignore,
             ~scope=
               to_scope([
                 ("a", K_Weak(0)),
                 ("b", K_Weak(1)),
                 ("c", K_Weak(2)),
               ]),
           );
      }
    ),
    "parse right-associative"
    >: (
      () => {
        [
          (
            "a ^ b ^ c",
            (
              "a" |> to_public_id(as_int),
              ("b" |> to_public_id(as_int), "c" |> to_public_id(as_int))
              |> to_expo_op
              |> as_float,
            )
            |> to_expo_op
            |> as_float,
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               to_scope([
                 ("a", K_Strong(K_Integer)),
                 ("b", K_Strong(K_Integer)),
                 ("c", K_Strong(K_Integer)),
               ]),
           );

        [
          (
            "- - - a",
            "a"
            |> to_public_id(as_int)
            |> to_neg_int
            |> to_neg_int
            |> to_neg_int,
          ),
        ]
        |> Assert.parse_many(~scope=to_scope([("a", K_Strong(K_Integer))]));
        [
          (
            "! ! ! a",
            "a"
            |> to_public_id(as_bool)
            |> to_not_bool
            |> to_not_bool
            |> to_not_bool,
          ),
        ]
        |> Assert.parse_many(~scope=to_scope([("a", K_Strong(K_Boolean))]));
      }
    ),
  ];
