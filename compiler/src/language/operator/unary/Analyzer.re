open Knot.Kore;
open AST;

let analyze_arithmetic = type_ =>
  Type.(
    switch (type_) {
    | Valid(Integer | Float)
    /* forward invalid types */
    | Invalid(_) => type_

    | _ => Invalid(NotInferrable)
    }
  );

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, ((operator, expression), _) as node) => {
    let range = Node.get_range(node);
    let (expression', expression_type) =
      expression |> Node.analyzer(analyze_expression(scope));

    expression_type
    |> Validator.validate(operator)
    |> Option.iter(Scope.report_type_err(scope, range));

    let type_ =
      switch (operator) {
      | Negative
      | Positive => analyze_arithmetic(expression_type)
      | Not => Valid(Boolean)
      };

    ((operator, expression'), type_);
  };
