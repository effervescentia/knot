open Knot.Kore;

let rec analyze_type_expression:
  (SymbolTable.t, AST.TypeExpression.raw_t) => Type.t =
  (defs, type_expr) =>
    AST.TypeExpression.(
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
      | Group((x, _)) => analyze_type_expression(defs, x)

      /* use the type of the inner expression to determine type of list items */
      | List((x, _)) => Valid(`List(analyze_type_expression(defs, x)))

      | Struct(xs) =>
        Valid(
          `Struct(
            xs
            |> List.map(
                 Tuple.map_each2(fst, fst % analyze_type_expression(defs)),
               ),
          ),
        )

      | Function(args, (res, _)) =>
        Valid(
          `Function((
            args |> List.map(fst % analyze_type_expression(defs)),
            analyze_type_expression(defs, res),
          )),
        )

      | DotAccess((root, _), (prop, _)) =>
        switch (root |> analyze_type_expression(defs)) {
        | Valid(`Module(entries)) =>
          entries
          |> List.find_map(
               fun
               | (id, Type.Container.Type(type_)) when id == prop =>
                 Some(type_)
               | _ => None,
             )
          |?: Invalid(NotInferrable)
        | _ => Invalid(NotInferrable)
        }
      }
    );
