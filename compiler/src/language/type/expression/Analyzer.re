open Knot.Kore;
open AST;

let rec analyze: (SymbolTable.t, TypeExpression.raw_t) => Type.t =
  (defs, type_expr) =>
    TypeExpression.(
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
      | Group((x, _)) => analyze(defs, x)

      /* use the type of the inner expression to determine type of list items */
      | List((x, _)) => Valid(`List(analyze(defs, x)))

      | Object(xs) => Valid(`Object(analyze_struct_properties(defs, xs)))

      | Function(args, (res, _)) =>
        Valid(
          `Function((
            args |> List.map(fst % analyze(defs)),
            analyze(defs, res),
          )),
        )

      | DotAccess((root, _), (prop, _)) =>
        switch (root |> analyze(defs)) {
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

      | View((props, _), (res, _)) =>
        switch (props |> analyze(defs), res |> analyze(defs)) {
        | (
            Valid(`Object(props')),
            Valid(`Nil | `Boolean | `Integer | `Float | `String | `Element) as res',
          ) =>
          Valid(`View((props', res')))

        | _ => Invalid(NotInferrable)
        }
      }
    )

and analyze_struct_properties = (defs, properties) =>
  properties
  |> List.fold_left(
       acc =>
         fst
         % TypeExpression.(
             fun
             | Required((key, _), (value, _)) =>
               [(key, (value |> analyze(defs), true))]
               |> List.merge_assoc(acc)

             | Optional((key, _), (value, _)) =>
               [(key, (value |> analyze(defs), false))]
               |> List.merge_assoc(acc)

             | Spread((value, _)) => {
                 let type_ = value |> analyze(defs);

                 switch (type_) {
                 | Invalid(_) => acc

                 | Valid(`Object(xs)) => xs |> List.merge_assoc(acc)

                 // TODO: need to add error handling here to report spreading a non-struct type
                 | Valid(_) => acc
                 };
               }
           ),
       [],
     );
