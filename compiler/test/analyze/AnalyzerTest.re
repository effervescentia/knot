open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let _create_scope = name =>
  Scope.create(Namespace.of_string(name), ignore, Range.zero);
let _create_id = Identifier.of_string % CommonUtil.as_raw_node;

let __empty_scope = _create_scope("foo");
let __id = _create_id("foo");

let suite =
  "Analyze.Analyzer"
  >::: [
    "analyze_primitive()"
    >: (
      () =>
        [
          ((Type.Raw.Strong(`Nil), AST.Analyzed.Nil), AST.Raw.Nil),
          (
            (Type.Raw.Strong(`Boolean), AST.Analyzed.Boolean(true)),
            AST.Raw.Boolean(true),
          ),
          (
            (Type.Raw.Strong(`Integer), AST.Analyzed.Number(Integer(123L))),
            AST.Raw.Number(Integer(123L)),
          ),
          (
            (Type.Raw.Strong(`Float), AST.Analyzed.Number(Float(123.0, 3))),
            AST.Raw.Number(Float(123.0, 3)),
          ),
          (
            (Type.Raw.Strong(`String), AST.Analyzed.String("foo")),
            AST.Raw.String("foo"),
          ),
        ]
        |> List.map(
             Tuple.map_each2(
               Tuple.join2(AnalyzedUtil.as_node),
               RawUtil.as_raw_node % Analyzer.analyze_primitive,
             ),
           )
        |> Assert.(test_many(analyzed_primitive))
    ),
    "analyze_expression() - primitive"
    >: (
      () =>
        [
          (
            AnalyzedUtil.int_prim(123),
            RawUtil.(__empty_scope, int_prim(123)),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_expression)),
           )
        |> Assert.(test_many(analyzed_expression))
    ),
    "analyze_expression() - jsx"
    >: (
      () =>
        [
          (
            AnalyzedUtil.((__id, [], []) |> jsx_tag |> as_element),
            RawUtil.(
              __empty_scope,
              (__id, [], []) |> jsx_tag |> as_raw_node,
            ),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_expression)),
           )
        |> Assert.(test_many(analyzed_expression))
    ),
    "analyze_expression() - group"
    >: (
      () =>
        [
          (
            AnalyzedUtil.(
              AST.Analyzed.((123.0, 3) |> float_prim |> of_group |> as_float)
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.((123.0, 3) |> float_prim |> of_group |> as_raw_node),
            ),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_expression)),
           )
        |> Assert.(test_many(analyzed_expression))
    ),
    "analyze_expression() - closure"
    >: (
      () =>
        [
          (
            AnalyzedUtil.(AST.Analyzed.([] |> of_closure |> as_nil)),
            RawUtil.(
              __empty_scope,
              AST.Raw.([] |> of_closure |> as_raw_node),
            ),
          ),
          (
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
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_expression)),
           )
        |> Assert.(test_many(analyzed_expression))
    ),
    "analyze_expression() - identifier"
    >: (
      () => {
        let id = Identifier.of_string("foo");

        [
          {
            let type_err = Type.Error.NotFound(id);

            (
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
          },
          {
            let type_err = Type.Error.NotFound(id);
            let scope = {
              ..._create_scope("bar"),
              types:
                [(id, Type.Raw.Invalid(type_err))]
                |> List.to_seq
                |> Hashtbl.of_seq,
            };

            (
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
          },
          {
            let scope = {
              ..._create_scope("bar"),
              types:
                [(id, Type.Raw.Strong(`Boolean))]
                |> List.to_seq
                |> Hashtbl.of_seq,
            };

            (
              AnalyzedUtil.(AST.Analyzed.(id |> as_bool |> of_id |> as_bool)),
              RawUtil.(
                scope,
                AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
              ),
            );
          },
          {
            let parent_scope = {
              ..._create_scope("parent"),
              types:
                [(id, Type.Raw.Strong(`String))]
                |> List.to_seq
                |> Hashtbl.of_seq,
            };
            let child_scope = {
              ..._create_scope("child"),
              parent: Some(parent_scope),
            };

            (
              AnalyzedUtil.(
                AST.Analyzed.(id |> as_string |> of_id |> as_string)
              ),
              RawUtil.(
                child_scope,
                AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
              ),
            );
          },
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_expression)),
           )
        |> Assert.(test_many(analyzed_expression));
      }
    ),
    "res_jsx() - tag"
    >: (
      () =>
        [
          (
            AnalyzedUtil.(
              AST.Analyzed.((__id, [], []) |> of_tag |> as_element)
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.((__id, [], []) |> of_tag |> as_raw_node),
            ),
          ),
          (
            AnalyzedUtil.(
              AST.Analyzed.(
                (
                  __id,
                  [],
                  [(_create_id("bar"), [], []) |> jsx_node |> as_element],
                )
                |> of_tag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                (
                  __id,
                  [],
                  [(_create_id("bar"), [], []) |> jsx_node |> as_raw_node],
                )
                |> of_tag
                |> as_raw_node
              ),
            ),
          ),
          (
            AnalyzedUtil.(
              AST.Analyzed.(
                (
                  __id,
                  [],
                  [[] |> of_frag |> as_element |> of_node |> as_element],
                )
                |> of_tag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                (
                  __id,
                  [],
                  [[] |> of_frag |> as_raw_node |> of_node |> as_raw_node],
                )
                |> of_tag
                |> as_raw_node
              ),
            ),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(
               Tuple.join2(Analyzer.(analyze_expression % analyze_jsx)),
             ),
           )
        |> Assert.(test_many(analyzed_jsx))
    ),
    "res_jsx() - fragment"
    >: (
      () =>
        [
          (
            AnalyzedUtil.(AST.Analyzed.([] |> of_frag |> as_element)),
            RawUtil.(__empty_scope, AST.Raw.([] |> of_frag |> as_raw_node)),
          ),
          (
            AnalyzedUtil.(
              AST.Analyzed.(
                [(_create_id("bar"), [], []) |> jsx_node |> as_element]
                |> of_frag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                [(_create_id("bar"), [], []) |> jsx_node |> as_raw_node]
                |> of_frag
                |> as_raw_node
              ),
            ),
          ),
          (
            AnalyzedUtil.(
              AST.Analyzed.(
                [[] |> of_frag |> as_element |> of_node |> as_element]
                |> of_frag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                [[] |> of_frag |> as_raw_node |> of_node |> as_raw_node]
                |> of_frag
                |> as_raw_node
              ),
            ),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(
               Tuple.join2(Analyzer.(analyze_expression % analyze_jsx)),
             ),
           )
        |> Assert.(test_many(analyzed_jsx))
    ),
    "analyze_statement() - variable"
    >: (
      () =>
        [
          (
            AnalyzedUtil.(
              AST.Analyzed.((__id, int_prim(123)) |> of_var |> as_nil)
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.((__id, int_prim(123)) |> of_var |> as_raw_node),
            ),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_statement)),
           )
        |> Assert.(test_many(analyzed_statement))
    ),
    "analyze_statement() - expression"
    >: (
      () =>
        [
          (
            AnalyzedUtil.(AST.Analyzed.(123 |> int_prim |> of_expr |> as_int)),
            RawUtil.(
              __empty_scope,
              AST.Raw.(123 |> int_prim |> of_expr |> as_raw_node),
            ),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Tuple.join2(Analyzer.analyze_statement)),
           )
        |> Assert.(test_many(analyzed_statement))
    ),
  ];
