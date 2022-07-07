open Kore;
open Reference;

module Style = Grammar.Style;
module U = Util.ResultUtil;
module TE = A.TypeExpression;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((A.export_t, A.declaration_t), unit);

    let parser = ctx =>
      Style.parser(ctx, A.of_named_export)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) => {
              let (export, decl) = fst(stmt);

              A.Dump.(
                untyped_node_to_entity(
                  "Declaration",
                  ~children=[
                    export |> export_to_entity,
                    decl |> decl_to_entity,
                  ],
                  stmt,
                )
                |> Entity.pp(ppf)
              );
            },
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Style"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.parse_none([
          "gibberish",
          "style",
          "style Foo",
          "style Foo ()",
          "style Foo () ->",
          "style Foo () -> {",
          "style Foo ->",
          "style Foo -> {",
        ])
    ),
    "parse - with no arguments"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            ([], []) |> A.of_style |> U.as_style([], [], []),
          )
          |> U.as_untyped,
          "style Foo -> { }",
        )
    ),
    "parse - with empty arguments"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            ([], []) |> A.of_style |> U.as_style([], [], []),
          )
          |> U.as_untyped,
          "style Foo () -> { }",
        )
    ),
    "parse - with typed argument"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            (
              [
                A.{
                  name: U.as_untyped("fizz"),
                  type_: Some(U.as_untyped(TE.Integer)),
                  default: None,
                }
                |> U.as_int,
              ],
              [],
            )
            |> A.of_style
            |> U.as_style([Valid(`Integer)], [], []),
          )
          |> U.as_untyped,
          "style Foo (fizz: integer) -> { }",
        )
    ),
    "parse - with argument with default"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            (
              [
                A.{
                  name: U.as_untyped("fizz"),
                  type_: None,
                  default: Some(U.string_prim("bar")),
                }
                |> U.as_string,
              ],
              [],
            )
            |> A.of_style
            |> U.as_style([Valid(`String)], [], []),
          )
          |> U.as_untyped,
          "style Foo (fizz = \"bar\") -> { }",
        )
    ),
    "parse - with typed argument with default"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            (
              [
                A.{
                  name: U.as_untyped("fizz"),
                  type_: Some(U.as_untyped(TE.Boolean)),
                  default: Some(U.bool_prim(true)),
                }
                |> U.as_bool,
              ],
              [],
            )
            |> A.of_style
            |> U.as_style([Valid(`Boolean)], [], []),
          )
          |> U.as_untyped,
          "style Foo (fizz: boolean = true) -> { }",
        )
    ),
    "parse - with empty class rule set"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            (
              [],
              [(A.MatchClass(U.as_untyped("fizz")), []) |> U.as_untyped],
            )
            |> A.of_style
            |> U.as_style([], [], ["fizz"]),
          )
          |> U.as_untyped,
          "style Foo -> {
            .fizz {}
          }",
        )
    ),
    "parse - with empty identifier rule set"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            ([], [(A.MatchID(U.as_untyped("fizz")), []) |> U.as_untyped])
            |> A.of_style
            |> U.as_style([], ["fizz"], []),
          )
          |> U.as_untyped,
          "style Foo -> {
            #fizz {}
          }",
        )
    ),
    "parse - with multiple rules and rule sets"
    >: (
      () =>
        Assert.parse(
          ~context=ParseContext.create(Namespace.Internal("foo")),
          (
            "Foo" |> U.as_untyped |> A.of_named_export,
            (
              [],
              [
                (A.MatchID(U.as_untyped("bar")), []) |> U.as_untyped,
                (A.MatchID(U.as_untyped("fizz")), []) |> U.as_untyped,
                (A.MatchClass(U.as_untyped("buzz")), []) |> U.as_untyped,
              ],
            )
            |> A.of_style
            |> U.as_style([], ["bar", "fizz"], ["buzz"]),
          )
          |> U.as_untyped,
          "style Foo -> {
            #bar {
            }

            #fizz {
            }

            .buzz {
            }
          }",
          /* TODO: allow these to be referenced */
          /* "style Foo -> {
               #bar {
                 height: $px(20.0)
               }

               #fizz {
                 width: $px(10.0)
               }

               .buzz {
                 color: $red
               }
             }", */
        )
    ),
  ];
