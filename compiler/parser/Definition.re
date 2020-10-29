open Globals;

let rec stmts = input => (many(stmt) ==> (s => ModuleDefn(s)))(input)
and stmt = input => (M.main >> type_defn ==> (t => MainDefn(t)))(input)
and type_defn = input =>
  (builtin_type_defn <|> object_defn <|> function_defn)(input)
and builtin_type_defn =
  M.exact_identifier("string")
  >> return(StringDefn)
  <|> (M.exact_identifier("number") >> return(NumberDefn))
  <|> (M.exact_identifier("boolean") >> return(BooleanDefn))
  <|> (M.exact_identifier("jsx") >> return(JSXDefn))
  <|> (M.exact_identifier("nil") >> return(NilDefn))
and object_defn = input =>
  (
    M.identifier
    >>= (
      name => M.colon >> type_defn |> M.terminated ==> (typ_ => (name, typ_))
    )
    |> M.closure
    ==> (
      props =>
        List.fold_left(
          (tbl, (key, value)) => {
            Hashtbl.add(tbl, key, value);
            tbl;
          },
          Hashtbl.create(List.length(props)),
          props,
        )
    )
    ==> (tbl => ObjectDefn(tbl))
  )(
    input,
  )
and function_defn = input =>
  (
    M.comma_separated(type_defn)
    |> M.parentheses
    >>= (
      props => M.lambda >> type_defn ==> (type_ => FunctionDefn(props, type_))
    )
  )(
    input,
  );
