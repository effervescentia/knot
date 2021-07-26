open Kore;

let statement =
  AST.(
    fun
    | Expression(expr) => Node.type_(expr)
    | Variable(_) => Valid(`Nil)
  );

let declaration =
  AST.(
    fun
    | Constant(const) => Node.type_(const)
    /* TODO: extract argument types */
    | Function(args, res) =>
      Valid(
        `Function((
          args
          |> List.map((({name}, type_, _)) =>
               (
                 name |> Node.Raw.value |> Reference.Identifier.to_string,
                 type_,
               )
             ),
          Node.type_(res),
        )),
      )
  );
