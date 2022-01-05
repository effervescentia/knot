open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __empty_scope = Util.create_scope("foo");

let suite =
  "Analyze.Analyzer | Identifier"
  >::: [
    "unrecognized"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let type_err = Type.Error.NotFound(id);

        Assert.analyzed_expression(
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
    "invalid type"
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

        Assert.analyzed_expression(
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
    "local type"
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

        Assert.analyzed_expression(
          AnalyzedUtil.(AST.Analyzed.(id |> as_bool |> of_id |> as_bool)),
          RawUtil.(
            scope,
            AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
          ),
        );
      }
    ),
    "inherited type"
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

        Assert.analyzed_expression(
          AnalyzedUtil.(AST.Analyzed.(id |> as_string |> of_id |> as_string)),
          RawUtil.(
            child_scope,
            AST.Raw.(id |> as_raw_node |> of_id |> as_raw_node),
          ),
        );
      }
    ),
  ];
