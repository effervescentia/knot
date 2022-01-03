open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __empty_scope = Util.create_scope("foo");
let __id = Util.create_id("foo");

let _assert_expression = (expected, actual) =>
  Assert.analyzed_expression(
    expected,
    actual |> Tuple.join2(Analyzer.analyze_expression),
  );

let suite =
  "Analyze.Analyzer | Expression"
  >::: [
    "primitive"
    >: (
      () =>
        _assert_expression(
          AnalyzedUtil.int_prim(123),
          RawUtil.(__empty_scope, int_prim(123)),
        )
    ),
    "jsx"
    >: (
      () =>
        _assert_expression(
          AnalyzedUtil.((__id, [], []) |> jsx_tag |> as_element),
          RawUtil.(__empty_scope, (__id, [], []) |> jsx_tag |> as_raw_node),
        )
    ),
    "group"
    >: (
      () =>
        _assert_expression(
          AnalyzedUtil.(
            AST.Analyzed.((123.0, 3) |> float_prim |> of_group |> as_float)
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.((123.0, 3) |> float_prim |> of_group |> as_raw_node),
          ),
        )
    ),
    "empty closure"
    >: (
      () =>
        _assert_expression(
          AnalyzedUtil.(AST.Analyzed.([] |> of_closure |> as_nil)),
          RawUtil.(__empty_scope, AST.Raw.([] |> of_closure |> as_raw_node)),
        )
    ),
    "closure with statements"
    >: (
      () =>
        _assert_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              [
                123 |> int_prim |> of_expr |> as_int,
                nil_prim |> of_expr |> as_nil,
                true |> bool_prim |> of_expr |> as_bool,
              ]
              |> of_closure
              |> as_bool
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              [
                123 |> int_prim |> of_expr |> as_raw_node,
                nil_prim |> of_expr |> as_raw_node,
                true |> bool_prim |> of_expr |> as_raw_node,
              ]
              |> of_closure
              |> as_raw_node
            ),
          ),
        )
    ),
    "unrecognized identifier"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let type_err = Type.Error.NotFound(id);

        _assert_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              id |> as_invalid(type_err) |> of_id |> as_invalid(type_err)
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
          ),
        );
      }
    ),
    "identifier with invalid type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let type_err = Type.Error.NotFound(id);
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Invalid(type_err))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        _assert_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              id |> as_invalid(type_err) |> of_id |> as_invalid(type_err)
            )
          ),
          RawUtil.(
            scope,
            AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
          ),
        );
      }
    ),
    "identifier with local type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Strong(`Boolean))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        _assert_expression(
          AnalyzedUtil.(AST.Analyzed.(id |> as_bool |> of_id |> as_bool)),
          RawUtil.(
            scope,
            AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
          ),
        );
      }
    ),
    "identifier with inherited type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let parent_scope = {
          ...Util.create_scope("parent"),
          types:
            [(id, Type.Raw.Strong(`String))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };
        let child_scope = {
          ...Util.create_scope("child"),
          parent: Some(parent_scope),
        };

        _assert_expression(
          AnalyzedUtil.(AST.Analyzed.(id |> as_string |> of_id |> as_string)),
          RawUtil.(
            child_scope,
            AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
          ),
        );
      }
    ),
  ];
