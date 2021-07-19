open Kore;
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
    "parse primitive" >: (() => Assert.parse("123", 123 |> int_prim)),
    "parse identifier"
    >: (
      () =>
        [
          (
            "foo",
            "foo"
            |> RawUtil.public
            |> as_lexeme
            |> RawUtil.id
            |> as_invalid(NotFound(Public("foo"))),
          ),
          (
            "bar",
            "bar" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
          ),
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
          "foo"
          |> RawUtil.public
          |> as_lexeme
          |> RawUtil.id
          |> as_bool
          |> RawUtil.group
          |> as_bool,
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
              "foo"
              |> RawUtil.public
              |> as_lexeme
              |> RawUtil.id
              |> as_string
              |> RawUtil.expr,
              ("x" |> RawUtil.public |> as_lexeme, false |> bool_prim)
              |> RawUtil.var,
              (
                "y" |> RawUtil.public |> as_lexeme,
                "foo" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_string,
              )
              |> RawUtil.var,
              (
                "z" |> RawUtil.public |> as_lexeme,
                "y" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_string,
              )
              |> RawUtil.var,
              (1 |> int_prim, 2 |> int_prim)
              |> RawUtil.add_op
              |> as_int
              |> RawUtil.expr,
            ]
            |> RawUtil.closure
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
          ("-123", 123 |> int_prim |> RawUtil.neg_op |> as_int),
          ("!true", true |> bool_prim |> RawUtil.not_op |> as_bool),
        ]
        |> Assert.parse_many
    ),
    "parse boolean logic"
    >: (
      () =>
        [("&&", RawUtil.and_op), ("||", RawUtil.or_op)]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("true%sfalse"),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_bool,
               ),
               (
                 op |> Print.fmt(" true %s false "),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_bool,
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
          ("+", RawUtil.add_op % as_int),
          ("-", RawUtil.sub_op % as_int),
          ("*", RawUtil.mult_op % as_int),
          ("/", RawUtil.div_op % as_float),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("123%s456"),
                 (123 |> int_prim, 456 |> int_prim) |> tag,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (123 |> int_prim, 456 |> int_prim) |> tag,
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
          ("<=", RawUtil.lte_op),
          ("<", RawUtil.lt_op),
          (">=", RawUtil.gte_op),
          (">", RawUtil.gt_op),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("123%s456"),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_bool,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_bool,
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
                  (int_prim(4), int_prim(5)) |> RawUtil.expo_op |> as_float,
                )
                |> RawUtil.mult_op
                |> as_float,
              )
              |> RawUtil.add_op
              |> as_float,
              (int_prim(6) |> RawUtil.neg_op |> as_int, int_prim(7))
              |> RawUtil.div_op
              |> as_float,
            )
            |> RawUtil.sub_op
            |> as_float,
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
            (
              (int_prim(2), int_prim(3))
              |> RawUtil.add_op
              |> as_int
              |> RawUtil.group
              |> as_int,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> RawUtil.div_op
                  |> as_float
                  |> RawUtil.group
                  |> as_float
                  |> RawUtil.neg_op
                  |> as_float,
                )
                |> RawUtil.sub_op
                |> as_float
                |> RawUtil.group
                |> as_float,
              )
              |> RawUtil.expo_op
              |> as_float,
            )
            |> RawUtil.mult_op
            |> as_float,
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
            (
              (
                "a" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_bool,
                (
                  (
                    "b" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                    "c"
                    |> RawUtil.public
                    |> as_lexeme
                    |> RawUtil.id
                    |> as_float,
                  )
                  |> RawUtil.gt_op
                  |> as_bool,
                  (
                    "e" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_float,
                    "f" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                  )
                  |> RawUtil.lte_op
                  |> as_bool,
                )
                |> RawUtil.or_op
                |> as_bool
                |> RawUtil.group
                |> as_bool,
              )
              |> RawUtil.and_op
              |> as_bool,
              (
                "g" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_bool,
                "h" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_bool,
              )
              |> RawUtil.or_op
              |> as_bool
              |> RawUtil.group
              |> as_bool
              |> RawUtil.not_op
              |> as_bool
              |> RawUtil.group
              |> as_bool,
            )
            |> RawUtil.and_op
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
        [
          ("+", RawUtil.add_op),
          ("-", RawUtil.sub_op),
          ("*", RawUtil.mult_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                   "b" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                 )
                 |> tag
                 |> as_int,
                 "c" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
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

        [("/", RawUtil.div_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                   "b" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                 )
                 |> tag
                 |> as_float,
                 "c" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
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

        [("&&", RawUtil.and_op), ("||", RawUtil.or_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_bool,
                   "b" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_bool,
                 )
                 |> tag
                 |> as_bool,
                 "c" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_bool,
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
          ("<=", RawUtil.lte_op),
          ("<", RawUtil.lt_op),
          (">=", RawUtil.gte_op),
          (">", RawUtil.gt_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a"
                   |> RawUtil.public
                   |> as_lexeme
                   |> RawUtil.id
                   |> as_weak(0),
                   "b"
                   |> RawUtil.public
                   |> as_lexeme
                   |> RawUtil.id
                   |> as_weak(1),
                 )
                 |> tag
                 |> as_invalid(NotAssignable(K_Weak(0), K_Numeric)),
                 "c"
                 |> RawUtil.public
                 |> as_lexeme
                 |> RawUtil.id
                 |> as_weak(2),
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
        [("==", RawUtil.eq_op), ("!=", RawUtil.ineq_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a"
                   |> RawUtil.public
                   |> as_lexeme
                   |> RawUtil.id
                   |> as_weak(0),
                   "b"
                   |> RawUtil.public
                   |> as_lexeme
                   |> RawUtil.id
                   |> as_weak(1),
                 )
                 |> tag
                 |> as_invalid(TypeMismatch(K_Weak(0), K_Weak(1))),
                 "c"
                 |> RawUtil.public
                 |> as_lexeme
                 |> RawUtil.id
                 |> as_weak(2),
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
              "a" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
              (
                "b" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                "c" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
              )
              |> RawUtil.expo_op
              |> as_float,
            )
            |> RawUtil.expo_op
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
            |> RawUtil.public
            |> as_lexeme
            |> RawUtil.id
            |> as_int
            |> RawUtil.neg_op
            |> as_int
            |> RawUtil.neg_op
            |> as_int
            |> RawUtil.neg_op
            |> as_int,
          ),
        ]
        |> Assert.parse_many(~scope=to_scope([("a", K_Strong(K_Integer))]));
        [
          (
            "! ! ! a",
            "a"
            |> RawUtil.public
            |> as_lexeme
            |> RawUtil.id
            |> as_bool
            |> RawUtil.not_op
            |> as_bool
            |> RawUtil.not_op
            |> as_bool
            |> RawUtil.not_op
            |> as_bool,
          ),
        ]
        |> Assert.parse_many(~scope=to_scope([("a", K_Strong(K_Boolean))]));
      }
    ),
  ];
