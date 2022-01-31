open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __id = Identifier.of_string("foo");
let __empty_scope = Util.create_scope("foo");

let _add_weak_type = (id, weak_id, scope: Scope.t) => {
  Hashtbl.add(scope.types, id, Type.Raw.Weak(scope.id, weak_id));
  Hashtbl.add(scope.weak_types, weak_id, Ok(`Generic((scope.id, weak_id))));
};

let _create_weak_scope = () => {
  let scope = Util.create_scope("weak");

  scope |> _add_weak_type(__id, 0);

  scope;
};

let _assert_weak_type =
  Alcotest.(check(testable(Type.Raw.pp_weak, (==)), "weak type matches"));

let suite =
  "Analyze.Analyzer | Add Operation"
  >::: [
    "integer primitives"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              (int_prim(123), int_prim(456)) |> of_add_op |> as_int
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              (int_prim(123), int_prim(456)) |> of_add_op |> as_raw_node
            ),
          ),
        )
    ),
    "float primitives"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              ((123.0, 3) |> float_prim, (456.0, 3) |> float_prim)
              |> of_add_op
              |> as_float
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              ((123.0, 3) |> float_prim, (456.0, 3) |> float_prim)
              |> of_add_op
              |> as_raw_node
            ),
          ),
        )
    ),
    "left integer and right float"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              (int_prim(123), (456.0, 3) |> float_prim)
              |> of_add_op
              |> as_float
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              (int_prim(123), (456.0, 3) |> float_prim)
              |> of_add_op
              |> as_raw_node
            ),
          ),
        )
    ),
    "left float and right integer"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              ((456.0, 3) |> float_prim, int_prim(123))
              |> of_add_op
              |> as_float
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              ((456.0, 3) |> float_prim, int_prim(123))
              |> of_add_op
              |> as_raw_node
            ),
          ),
        )
    ),
    "left float, infer right-hand type as integer"
    >: (
      () => {
        let scope = _create_weak_scope();

        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              (
                (456.0, 3) |> float_prim,
                __id |> as_weak(0, 0) |> of_id |> as_weak(0, 0),
              )
              |> of_add_op
              |> as_float
            )
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              (
                (456.0, 3) |> float_prim,
                __id |> as_raw_node |> of_id |> as_raw_node,
              )
              |> of_add_op
              |> as_raw_node
            ),
          ),
        );

        let (_, inferred_type) = scope |> Scope.find_weak(0, 0);

        Assert.weak_type(inferred_type, Ok(`Integer));
      }
    ),
    "right float, infer left-hand type as integer"
    >: (
      () => {
        let scope = _create_weak_scope();

        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              (
                __id |> as_weak(0, 0) |> of_id |> as_weak(0, 0),
                (456.0, 3) |> float_prim,
              )
              |> of_add_op
              |> as_float
            )
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              (
                __id |> as_raw_node |> of_id |> as_raw_node,
                (456.0, 3) |> float_prim,
              )
              |> of_add_op
              |> as_raw_node
            ),
          ),
        );

        let (_, inferred_type) = scope |> Scope.find_weak(0, 0);

        Assert.weak_type(inferred_type, Ok(`Integer));
      }
    ),
    "infer both types as integer"
    >: (
      () => {
        let left_id = Identifier.of_string("left");
        let right_id = Identifier.of_string("right");
        let scope = Util.create_scope("weak");

        scope |> _add_weak_type(left_id, 0);
        scope |> _add_weak_type(right_id, 1);

        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              (
                left_id |> as_weak(0, 0) |> of_id |> as_weak(0, 0),
                right_id |> as_weak(0, 1) |> of_id |> as_weak(0, 1),
              )
              |> of_add_op
              |> as_int
            )
          ),
          RawUtil.(
            scope,
            AST.Raw.(
              (
                left_id |> as_raw_node |> of_id |> as_raw_node,
                right_id |> as_raw_node |> of_id |> as_raw_node,
              )
              |> of_add_op
              |> as_raw_node
            ),
          ),
        );

        let (_, left_type) = scope |> Scope.find_weak(0, 0);
        let (_, right_type) = scope |> Scope.find_weak(0, 1);

        Assert.weak_type(left_type, Ok(`Integer));
        Assert.weak_type(right_type, Ok(`Integer));
      }
    ),
    "throw TypeMismatch if right-hand type is incompatible"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
              (
                int_prim(123),
                false
                |> of_bool
                |> as_bool
                |> of_prim
                |> as_invalid(
                     TypeMismatch(Strong(`Integer), Strong(`Boolean)),
                   ),
              )
              |> of_add_op
              |> as_int
            )
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.(
              (int_prim(123), bool_prim(false)) |> of_add_op |> as_raw_node
            ),
          ),
        )
    ),
  ];
