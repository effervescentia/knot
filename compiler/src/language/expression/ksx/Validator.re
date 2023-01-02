open Knot.Kore;
open AST;

let validate_ksx_render:
  (bool, (string, Type.t, list((string, Result.untyped_t(Type.t))))) =>
  list((Type.error_t, option(Range.t))) =
  has_children =>
    fun
    | (id, Invalid(_), _) => [(NotFound(id), None)]

    | (id, Valid(`View(attrs, _)), actual_attrs) => {
        let keys =
          (attrs |> List.map(fst))
          @ (actual_attrs |> List.map(fst))
          |> List.uniq_by((==));

        let (invalid, missing) =
          keys
          |> List.fold_left(
               ((invalid, missing) as acc, key) => {
                 let expected = attrs |> List.assoc_opt(key);
                 let actual = actual_attrs |> List.assoc_opt(key);

                 switch (expected, actual) {
                 | _ when key == "children" && has_children => acc
                 | (
                     Some((expected', _)),
                     Some((actual_value, _) as actual'),
                   ) =>
                   Type.(
                     switch (expected', actual_value) {
                     | (Invalid(_), _)
                     | (_, Invalid(_)) => acc
                     | (Valid(_), Valid(_)) when expected' == actual_value => acc
                     | (Valid(_), Valid(_)) => (
                         invalid
                         @ [
                           (
                             InvalidKSXAttribute(key, expected', actual_value),
                             Some(Node.get_range(actual')),
                           ),
                         ],
                         missing,
                       )
                     }
                   )

                 | (Some((expected', true)), None) => (
                     invalid,
                     missing @ [(key, expected')],
                   )

                 | (None, Some(actual')) => (
                     invalid
                     @ [
                       (
                         Type.UnexpectedKSXAttribute(key, fst(actual')),
                         Some(Node.get_range(actual')),
                       ),
                     ],
                     missing,
                   )

                 | (Some((_, false)), None)
                 | (None, None) => acc
                 };
               },
               ([], []),
             );

        if (!List.is_empty(invalid)) {
          invalid;
        } else if (!List.is_empty(missing)) {
          [(Type.MissingKSXAttributes(id, missing), None)];
        } else {
          [];
        };
      }

    | (id, expr_type, attrs) => [
        (
          InvalidKSXTag(
            id,
            expr_type,
            attrs |> List.map(Tuple.map_snd2(fst)),
          ),
          None,
        ),
      ];

let validate_style_binding =
    (styles: list(Result.expression_t))
    : list((Type.error_t, option(Range.t))) => {
  styles
  |> List.filter_map(node =>
       node
       |> Node.get_type
       |> (
         fun
         /* assume this has been reported already and ignore */
         | Type.Invalid(_) => None

         | Type.Valid(`Style) => None

         | type_ =>
           Some((
             Type.TypeMismatch(Valid(`Style), type_),
             Some(Node.get_range(node)),
           ))
       )
     );
};

let validate_ksx_primitive_expression: Type.t => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Nil | `Boolean | `Integer | `Float | `String | `Element) => None

  | type_ => Some(InvalidKSXPrimitiveExpression(type_));
