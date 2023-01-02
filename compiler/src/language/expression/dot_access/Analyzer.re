open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Raw.expression_t, Node.t(string, unit)),
    Range.t
  ) =>
  (Result.expression_t, Type.t) =
  (scope, analyze_expression, (object_, property), range) => {
    let property_name = fst(property);
    let object_' = analyze_expression(scope, object_);
    let object_type = Node.get_type(object_');

    object_type
    |> Validator.validate(property_name)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      object_',
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
