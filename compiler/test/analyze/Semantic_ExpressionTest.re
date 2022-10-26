open Kore;

module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = ParseContext.create(~report=ignore, __namespace);
let __scope = S.create(__context, Range.zero);
let __throw_scope = S.create({...__context, report: throw}, Range.zero);

let suite =
  "Analyze.Semantic | Expression"
  >::: [
    "adopt primitive type"
    >: (
      () =>
        Assert.expression(
          URes.bool_prim(true),
          true |> URaw.bool_prim |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "extract group inner expression type"
    >: (
      () =>
        Assert.expression(
          "foo" |> URes.string_prim |> A.of_group |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_group
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "extract last closure statement type"
    >: (
      () =>
        Assert.expression(
          [123 |> URes.int_prim |> A.of_expr |> URes.as_int]
          |> A.of_closure
          |> URes.as_int,
          [123 |> URaw.int_prim |> AR.of_expr |> URaw.as_node]
          |> AR.of_closure
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve nil if closure empty"
    >: (
      () =>
        Assert.expression(
          [] |> A.of_closure |> URes.as_nil,
          []
          |> AR.of_closure
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "always resolve JSX as element type"
    >: (
      () =>
        Assert.expression(
          (URes.as_untyped(__id), [], [])
          |> A.of_tag
          |> A.of_jsx
          |> URes.as_element,
          (URaw.as_untyped(__id), [], [])
          |> AR.of_tag
          |> AR.of_jsx
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve NotInferrable type on unrecognized identifier"
    >: (
      () =>
        Assert.expression(
          __id |> A.of_id |> URes.as_invalid(NotInferrable),
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "report NotFound exception on unrecognized identifier"
    >: (
      () =>
        Assert.throws_compile_errors(
          [ParseError(TypeError(NotFound(__id)), __namespace, Range.zero)],
          () =>
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__throw_scope)
        )
    ),
    "resolve recognized identifier"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__id, T.Valid(`Boolean))] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          __id |> A.of_id |> URes.as_bool,
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve dot access"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__id, T.Valid(`Struct([("foo", Valid(`Boolean))])))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            __id |> A.of_id |> URes.as_struct([("foo", T.Valid(`Boolean))]),
            URes.as_untyped("foo"),
          )
          |> A.of_dot_access
          |> URes.as_bool,
          (__id |> AR.of_id |> URaw.as_node, URaw.as_untyped("foo"))
          |> AR.of_dot_access
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve function call"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [
              (
                __id,
                T.Valid(`Function(([Valid(`Integer)], Valid(`String)))),
              ),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            __id
            |> A.of_id
            |> URes.as_function([T.Valid(`Integer)], T.Valid(`String)),
            [URes.int_prim(123)],
          )
          |> A.of_func_call
          |> URes.as_string,
          (__id |> AR.of_id |> URaw.as_node, [URaw.int_prim(123)])
          |> AR.of_func_call
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve style expression"
    >: (
      () => {
        let fizz_type =
          T.Valid(`Function(([Valid(`Integer)], Valid(`Boolean))));
        let scope = {
          ...__throw_scope,
          context: {
            ...__throw_scope.context,
            modules: {
              ...__throw_scope.context.modules,
              plugins: [
                (
                  StyleExpression,
                  [
                    ("fizz", Value(fizz_type)),
                    ("buzz", Value(Valid(`String))),
                  ],
                ),
                (
                  StyleRule,
                  [
                    (
                      "foo",
                      Value(
                        Valid(
                          `Function(([Valid(`Boolean)], Valid(`Nil))),
                        ),
                      ),
                    ),
                    (
                      "bar",
                      Value(
                        Valid(
                          `Function(([Valid(`Integer)], Valid(`Nil))),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
            },
          },
        };

        Assert.expression(
          [
            (
              "foo" |> URes.as_bool,
              (
                "$fizz" |> A.of_id |> URes.as_typed(fizz_type),
                [URes.int_prim(123)],
              )
              |> A.of_func_call
              |> URes.as_bool,
            )
            |> URes.as_untyped,
            ("bar" |> URes.as_int, "$buzz" |> A.of_id |> URes.as_string)
            |> URes.as_untyped,
          ]
          |> A.of_style
          |> URes.as_style,
          [
            (
              "foo" |> URaw.as_node,
              ("$fizz" |> AR.of_id |> URaw.as_node, [URaw.int_prim(123)])
              |> AR.of_func_call
              |> URaw.as_node,
            )
            |> URes.as_untyped,
            ("bar" |> URaw.as_node, "$buzz" |> AR.of_id |> URaw.as_node)
            |> URes.as_untyped,
          ]
          |> AR.of_style
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
  ];
