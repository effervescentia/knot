open Knot.Kore;
open AST;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, ((object_, property), _) as node) => {
    let range = Node.get_range(node);
    let property_name = fst(property);
    let (object_', object_type) =
      object_ |> Node.analyzer(analyze_expression(scope));

    object_type
    |> Validator.validate(property_name)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      (object_', property),
      (
        switch (object_type) {
        | Valid(Object(props)) =>
          props |> List.assoc_opt(property_name) |> Option.map(fst)

        | Valid(Module(entries)) =>
          entries
          |> List.find_map(
               fun
               | (Type.ModuleEntryKind.Value, name, t)
                   when name == property_name =>
                 Some(t)
               | _ => None,
             )

        | _ => None
        }
      )
      |?: Invalid(NotInferrable),
    );
  };
