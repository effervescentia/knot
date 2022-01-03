open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __empty_scope = Util.create_scope("foo");
let __id = Util.create_id("foo");

let _assert_jsx = (expected, actual) =>
  Assert.analyzed_jsx(
    expected,
    actual |> Tuple.join2(Analyzer.(analyze_expression % analyze_jsx)),
  );

let suite =
  "Analyze.Analyzer | JSX"
  >::: [
    "empty tag"
    >: (
      () =>
        _assert_jsx(
          AnalyzedUtil.(
            AST.Analyzed.((__id, [], []) |> of_tag |> as_element)
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.((__id, [], []) |> of_tag |> as_raw_node),
          ),
        )
    ),
    "tag with child tag"
    >: (
      () =>
        _assert_jsx(
          AnalyzedUtil.(
            AST.Analyzed.(
              (
                __id,
                [],
                [(Util.create_id("bar"), [], []) |> jsx_node |> as_element],
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
                [
                  (Util.create_id("bar"), [], []) |> jsx_node |> as_raw_node,
                ],
              )
              |> of_tag
              |> as_raw_node
            ),
          ),
        )
    ),
    "tag with child fragment"
    >: (
      () =>
        _assert_jsx(
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
        )
    ),
    "empty fragment"
    >: (
      () =>
        _assert_jsx(
          AnalyzedUtil.(AST.Analyzed.([] |> of_frag |> as_element)),
          RawUtil.(__empty_scope, AST.Raw.([] |> of_frag |> as_raw_node)),
        )
    ),
    "fragment with child tag"
    >: (
      () =>
        _assert_jsx(
          AnalyzedUtil.(
            AST.Analyzed.(
              [(Util.create_id("bar"), [], []) |> jsx_node |> as_element]
              |> of_frag
              |> as_element
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              [(Util.create_id("bar"), [], []) |> jsx_node |> as_raw_node]
              |> of_frag
              |> as_raw_node
            ),
          ),
        )
    ),
    "fragment with child fragment"
    >: (
      () =>
        _assert_jsx(
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
        )
    ),
  ];
