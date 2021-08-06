open Kore;

let statement =
  AST.(
    fun
    | Expression(expr) => Node.get_type(expr)
    /* variable declarations result in a `nil` value */
    | Variable(_) => Valid(`Nil)
  );

let declaration =
  AST.(
    fun
    | Constant(const) => Node.get_type(const)
    /* TODO: extract argument types */
    | Function(args, res) =>
      Valid(
        `Function((
          args
          |> List.map((({name}, type_, _)) =>
               (
                 name |> Node.Raw.get_value |> Reference.Identifier.to_string,
                 type_,
               )
             ),
          Node.get_type(res),
        )),
      )
  );
