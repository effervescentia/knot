open Kore;
open AST;

let format: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_statement_t) =
  (pp_expression, ppf) => {
    let (&>) = (args, format) => args |> format(pp_expression, ppf);

    fun
    | Variable(name, expression) => (name, expression) &> KVariable.format
    | Effect(expression) => expression &> KEffect.format;
  };
