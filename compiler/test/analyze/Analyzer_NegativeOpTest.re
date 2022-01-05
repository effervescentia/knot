open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __empty_scope = Util.create_scope("foo");

let _assert_weak_type =
  Alcotest.(check(testable(Type.Raw.pp_weak, (==)), "weak type matches"));

let suite =
  "Analyze.Analyzer | NegativeOp"
  >::: [
    "integer primitive"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(AST.Analyzed.(123 |> int_prim |> of_neg_op |> as_int)),
          RawUtil.(
            __empty_scope,
            AST.Raw.(123 |> int_prim |> of_neg_op |> as_raw_node),
          ),
        )
    ),
    "float primitive"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.((123.0, 3) |> float_prim |> of_neg_op |> as_float)
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.((123.0, 3) |> float_prim |> of_neg_op |> as_raw_node),
          ),
        )
    ),
    "check weak integer type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Weak(0, 0))] |> List.to_seq |> Hashtbl.of_seq,
          weak_types: [(0, Ok(`Integer))] |> List.to_seq |> Hashtbl.of_seq,
        };
        let as_weak = x => AnalyzedUtil.as_weak(0, 0, x);

        Assert.analyzed_expression(
          AST.Analyzed.(
            id |> as_weak |> of_id |> as_weak |> of_neg_op |> as_weak
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              id
              |> as_raw_node
              |> of_id
              |> as_raw_node
              |> of_neg_op
              |> as_raw_node
            ),
          ),
        );
      }
    ),
    "check weak float type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Weak(0, 0))] |> List.to_seq |> Hashtbl.of_seq,
          weak_types: [(0, Ok(`Float))] |> List.to_seq |> Hashtbl.of_seq,
        };
        let as_weak = x => AnalyzedUtil.as_weak(0, 0, x);

        Assert.analyzed_expression(
          AST.Analyzed.(
            id |> as_weak |> of_id |> as_weak |> of_neg_op |> as_weak
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              id
              |> as_raw_node
              |> of_id
              |> as_raw_node
              |> of_neg_op
              |> as_raw_node
            ),
          ),
        );
      }
    ),
    "infer integer type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Weak(0, 0))] |> List.to_seq |> Hashtbl.of_seq,
          weak_types:
            [(0, Ok(`Generic((0, 0))))] |> List.to_seq |> Hashtbl.of_seq,
        };
        let as_weak = x => AnalyzedUtil.as_weak(0, 0, x);

        Assert.analyzed_expression(
          AST.Analyzed.(
            id |> as_weak |> of_id |> as_weak |> of_neg_op |> as_weak
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              id
              |> as_raw_node
              |> of_id
              |> as_raw_node
              |> of_neg_op
              |> as_raw_node
            ),
          ),
        );

        _assert_weak_type(Ok(`Integer), Hashtbl.find(scope.weak_types, 0));
      }
    ),
    "invalidate strong type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Strong(`String))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.analyzed_expression(
          AST.Analyzed.(
            AnalyzedUtil.(
              id
              |> as_string
              |> of_id
              |> as_string
              |> of_neg_op
              |> as_invalid(
                   TypeMismatch(Strong(`Integer), Strong(`String)),
                 )
            )
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              id
              |> as_raw_node
              |> of_id
              |> as_raw_node
              |> of_neg_op
              |> as_raw_node
            ),
          ),
        );
      }
    ),
    "non-narrowable weak type"
    >: (
      () => {
        let id = Identifier.of_string("foo");
        let scope = {
          ...Util.create_scope("bar"),
          types:
            [(id, Type.Raw.Weak(0, 0))] |> List.to_seq |> Hashtbl.of_seq,
          weak_types: [(0, Ok(`Boolean))] |> List.to_seq |> Hashtbl.of_seq,
        };
        let as_weak = x => AnalyzedUtil.as_weak(0, 0, x);

        Assert.analyzed_expression(
          AST.Analyzed.(
            id
            |> as_weak
            |> of_id
            |> as_weak
            |> of_neg_op
            |> AnalyzedUtil.as_invalid(
                 NotNarrowable(Strong(`Integer), Strong(`Boolean)),
               )
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              id
              |> as_raw_node
              |> of_id
              |> as_raw_node
              |> of_neg_op
              |> as_raw_node
            ),
          ),
        );
      }
    ),
  ];
