open Kore;

module S = Set.Make(Stdlib.String);

let rec eval_type_expression: (SymbolTable.t, A.TypeExpression.raw_t) => T.t =
  (defs, type_expr) =>
    A.TypeExpression.(
      switch (type_expr) {
      | Nil => Valid(`Nil)
      | Boolean => Valid(`Boolean)
      | Integer => Valid(`Integer)
      | Float => Valid(`Float)
      | String => Valid(`String)
      | Element => Valid(`Element)
      | Style => Valid(`Style)

      | Identifier((id, _)) =>
        defs |> SymbolTable.resolve_type(id) |?: Invalid(NotInferrable)

      /* use the type of the inner expression to determine type */
      | Group((x, _)) => eval_type_expression(defs, x)

      /* use the type of the inner expression to determine type of list items */
      | List((x, _)) => Valid(`List(eval_type_expression(defs, x)))

      | Struct(xs) =>
        Valid(
          `Struct(
            xs
            |> List.map(
                 Tuple.map_each2(fst, fst % eval_type_expression(defs)),
               ),
          ),
        )

      | Function(args, (res, _)) =>
        Valid(
          `Function((
            args |> List.map(fst % eval_type_expression(defs)),
            eval_type_expression(defs, res),
          )),
        )

      | DotAccess((root, _), (prop, _)) =>
        switch (root |> eval_type_expression(defs)) {
        | Valid(`Module(entries)) =>
          entries
          |> List.find_map(
               fun
               | (id, T.Container.Type(type_)) when id == prop =>
                 Some(type_)
               | _ => None,
             )
          |?: Invalid(NotInferrable)
        | _ => Invalid(NotInferrable)
        }
      }
    );
